#include "screen_share_controller.h"

#include <QClipboard>
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
}

QString ScreenShareController::status() const { return m_status; }
bool ScreenShareController::busy() const { return m_busy; }
bool ScreenShareController::ready() const { return !m_sessionId.isEmpty() && !m_viewerToken.isEmpty(); }
QString ScreenShareController::sessionId() const { return m_sessionId; }
QString ScreenShareController::hostUrl() const { return m_hostUrl; }
QString ScreenShareController::viewerUrl() const { return m_viewerUrl; }
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
            || sessionId.size() > 100 || viewerToken.size() < 32 || !urlsMatchSession
            || !safeServiceUrl(hostUrl, QStringLiteral("screen-share.the-end.shop"))
            || !safeServiceUrl(viewerUrl, QStringLiteral("screen-share.the-end.shop"))) {
            setStatus(QStringLiteral("The service response failed security validation; no link was accepted."));
            return;
        }

        m_sessionId = sessionId;
        m_hostUrl = hostUrl;
        m_viewerUrl = viewerUrl;
        m_viewerToken = viewerToken;
        setStatus(QStringLiteral("Session created. Open the host link and explicitly approve screen sharing."));
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
        "Tool: get_screen_snapshot\n"
        "Token: %3\n\n"
        "First open the host link on the sharing computer and explicitly approve screen capture. "
        "Then connect the AI client to the MCP endpoint, finish OAuth sign-in, and call get_screen_snapshot "
        "with the Session ID and Token above. This token grants access to snapshots for this session; do not share it publicly.")
        .arg(mcpUrl, m_sessionId, m_viewerToken);
}
