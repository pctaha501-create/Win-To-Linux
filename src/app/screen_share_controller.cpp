#include "screen_share_controller.h"

#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>

namespace {
const QUrl sessionApi(QStringLiteral("https://screen-share.the-end.shop/api/sessions"));
const QString mcpUrl = QStringLiteral("https://mcp.the-end.shop/mcp");

bool safeServiceUrl(const QString &value, const QString &host)
{
    const QUrl url(value);
    return url.isValid() && url.scheme() == QStringLiteral("https")
        && url.host() == host && url.userInfo().isEmpty()
        && url.fragment().isEmpty();
}
}

ScreenShareController::ScreenShareController(QObject *parent)
    : QObject(parent)
{
    m_expiryTimer.setSingleShot(true);
    connect(&m_expiryTimer, &QTimer::timeout, this, [this]() {
        m_sessionId.clear(); m_hostUrl.clear(); m_viewerUrl.clear(); m_viewerToken.clear(); m_expiresAt.clear();
        setStatus(QStringLiteral("The screen-share session expired. Create a new session to reconnect."));
    });
}

QString ScreenShareController::status() const { return m_status; }
bool ScreenShareController::busy() const { return m_busy; }
bool ScreenShareController::ready() const { return !m_sessionId.isEmpty() && !m_viewerToken.isEmpty(); }
QString ScreenShareController::sessionId() const { return m_sessionId; }
QString ScreenShareController::hostUrl() const { return m_hostUrl; }
QString ScreenShareController::viewerUrl() const { return m_viewerUrl; }
QString ScreenShareController::expiresAt() const { return m_expiresAt; }
QString ScreenShareController::mcpEndpoint() const { return mcpUrl; }

void ScreenShareController::setStatus(const QString &status)
{
    m_status = status;
    emit stateChanged();
}

void ScreenShareController::createMcp()
{
    if (m_busy) return;
    m_busy = true;
    m_status = QStringLiteral("Creating a private screen-share session…");
    emit stateChanged();

    QNetworkRequest request(sessionApi);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("User-Agent", "Win-To-Linux/0.2.0");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = m_network.post(request, QByteArrayLiteral("{}"));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();
        const auto networkError = reply->error();
        const QString networkErrorText = reply->errorString();
        reply->deleteLater();
        m_busy = false;

        if (networkError != QNetworkReply::NoError || httpStatus < 200 || httpStatus >= 300) {
            setStatus(QStringLiteral("Could not create the session (HTTP %1). Check your connection and retry.")
                          .arg(httpStatus ? QString::number(httpStatus) : networkErrorText));
            return;
        }

        QJsonParseError parseError{};
        const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            setStatus(QStringLiteral("The screen-share service returned an invalid response."));
            return;
        }
        const QJsonObject json = document.object();
        const QString sessionId = json.value(QStringLiteral("sessionId")).toString();
        const QString hostUrl = json.value(QStringLiteral("hostUrl")).toString();
        const QString viewerUrl = json.value(QStringLiteral("viewerUrl")).toString();
        const QString viewerToken = json.value(QStringLiteral("viewerToken")).toString();
        const QString expiresAt = json.value(QStringLiteral("expiresAt")).toString();
        const QDateTime expiry = QDateTime::fromString(expiresAt, Qt::ISODateWithMs);
        const QUrl host(hostUrl);
        const QUrl viewer(viewerUrl);
        const QUrlQuery hostQuery(host);
        const QUrlQuery viewerQuery(viewer);
        const QUuid parsedSession = QUuid::fromString(sessionId);
        const bool urlsMatchSession = !parsedSession.isNull()
            && host.path() == QStringLiteral("/host/") + sessionId
            && viewer.path() == QStringLiteral("/view/") + sessionId
            && !hostQuery.queryItemValue(QStringLiteral("token")).isEmpty()
            && viewerQuery.queryItemValue(QStringLiteral("token")) == viewerToken;
        if (!json.value(QStringLiteral("ok")).toBool() || sessionId.isEmpty()
            || sessionId.size() > 100 || viewerToken.size() < 32 || !expiry.isValid()
            || expiry <= QDateTime::currentDateTimeUtc() || !urlsMatchSession
            || !safeServiceUrl(hostUrl, QStringLiteral("screen-share.the-end.shop"))
            || !safeServiceUrl(viewerUrl, QStringLiteral("screen-share.the-end.shop"))) {
            setStatus(QStringLiteral("The service response failed security validation; no link was accepted."));
            return;
        }

        m_sessionId = sessionId;
        m_hostUrl = hostUrl;
        m_viewerUrl = viewerUrl;
        m_viewerToken = viewerToken;
        m_expiresAt = expiry.toUTC().toString(Qt::ISODateWithMs);
        m_expiryTimer.start(static_cast<int>(qMin<qint64>(expiry.toMSecsSinceEpoch() - QDateTime::currentDateTimeUtc().toMSecsSinceEpoch(), 3600000)));
        setStatus(QStringLiteral("Session created. It expires in 60 minutes; open the host link and explicitly approve screen sharing."));
    });
}

void ScreenShareController::endSession()
{
    if (m_busy) return;
    if (m_sessionId.isEmpty() || m_hostUrl.isEmpty()) {
        setStatus(QStringLiteral("There is no active session to end."));
        return;
    }
    const QUrl host(m_hostUrl);
    const QUrlQuery hostQuery(host);
    const QString hostToken = hostQuery.queryItemValue(QStringLiteral("token"));
    if (hostToken.isEmpty() || QUuid::fromString(m_sessionId).isNull()) {
        setStatus(QStringLiteral("Could not validate the active session credentials."));
        return;
    }
    QUrl revokeUrl(QStringLiteral("https://screen-share.the-end.shop/api/sessions/") + m_sessionId + QStringLiteral("/revoke"));
    QUrlQuery query; query.addQueryItem(QStringLiteral("token"), hostToken); revokeUrl.setQuery(query);
    QNetworkRequest request(revokeUrl);
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("User-Agent", "Win-To-Linux/0.2.0");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    m_busy = true; m_status = QStringLiteral("Ending session and revoking its tokens…"); emit stateChanged();
    QNetworkReply *reply = m_network.post(request, QByteArray{});
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const auto networkError = reply->error();
        const QString errorText = reply->errorString();
        reply->deleteLater(); m_busy = false;
        if ((networkError == QNetworkReply::NoError && httpStatus >= 200 && httpStatus < 300) || httpStatus == 410) {
            m_expiryTimer.stop(); m_sessionId.clear(); m_hostUrl.clear(); m_viewerUrl.clear(); m_viewerToken.clear(); m_expiresAt.clear();
            setStatus(QStringLiteral("Session ended. Its tokens and stored snapshot have been revoked."));
            return;
        }
        setStatus(QStringLiteral("Could not confirm session revocation (HTTP %1). Retry or stop sharing from the host browser.")
                      .arg(httpStatus ? QString::number(httpStatus) : errorText));
    });
}

bool ScreenShareController::openSafeUrl(const QString &value, const QString &expectedHost)
{
    if (!safeServiceUrl(value, expectedHost)) {
        setStatus(QStringLiteral("Refused to open an untrusted or invalid link."));
        return false;
    }
    if (!QDesktopServices::openUrl(QUrl(value))) {
        setStatus(QStringLiteral("Could not open the link in the default browser."));
        return false;
    }
    return true;
}

void ScreenShareController::openHostLink()
{
    if (m_hostUrl.isEmpty()) { setStatus(QStringLiteral("Create a session first.")); return; }
    if (openSafeUrl(m_hostUrl, QStringLiteral("screen-share.the-end.shop")))
        setStatus(QStringLiteral("Host page opened. Choose a screen in the browser prompt to start sharing."));
}

void ScreenShareController::openViewerLink()
{
    if (m_viewerUrl.isEmpty()) { setStatus(QStringLiteral("Create a session first.")); return; }
    if (openSafeUrl(m_viewerUrl, QStringLiteral("screen-share.the-end.shop")))
        setStatus(QStringLiteral("Viewer page opened. Treat this private link like a password."));
}

void ScreenShareController::openMcpEndpoint()
{
    if (openSafeUrl(mcpUrl, QStringLiteral("mcp.the-end.shop")))
        setStatus(QStringLiteral("MCP endpoint opened. Add it to your AI client and complete its sign-in flow."));
}

void ScreenShareController::copyText(const QString &text)
{
    if (text.isEmpty()) { setStatus(QStringLiteral("There is nothing to copy yet.")); return; }
    if (QGuiApplication::clipboard()) {
        QGuiApplication::clipboard()->setText(text);
        setStatus(QStringLiteral("Copied to clipboard. Keep session links and tokens private."));
    } else {
        setStatus(QStringLiteral("Clipboard is unavailable in this session."));
    }
}

QString ScreenShareController::connectionInstructions() const
{
    if (!ready()) return {};
    return QStringLiteral(
        "MCP endpoint: %1\n"
        "Session ID: %2\n"
        "Session expiry (UTC): %4\n"
        "Tool: get_screen_snapshot\n"
        "Token: %3\n\n"
        "First open the host link on the sharing computer and explicitly approve screen capture. "
        "Then connect the AI client to the MCP endpoint, finish OAuth sign-in, and call get_screen_snapshot "
        "with the Session ID and Token above. This token grants access to snapshots for this session; do not share it publicly.")
        .arg(mcpUrl, m_sessionId, m_viewerToken, m_expiresAt);
}
