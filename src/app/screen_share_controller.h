#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QString>

class ScreenShareController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString status READ status NOTIFY stateChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY stateChanged)
    Q_PROPERTY(bool ready READ ready NOTIFY stateChanged)
    Q_PROPERTY(QString sessionId READ sessionId NOTIFY stateChanged)
    Q_PROPERTY(QString hostUrl READ hostUrl NOTIFY stateChanged)
    Q_PROPERTY(QString viewerUrl READ viewerUrl NOTIFY stateChanged)
    Q_PROPERTY(QString expiresAt READ expiresAt NOTIFY stateChanged)
    Q_PROPERTY(QString mcpEndpoint READ mcpEndpoint CONSTANT)

public:
    explicit ScreenShareController(QObject *parent = nullptr);

    QString status() const;
    bool busy() const;
    bool ready() const;
    QString sessionId() const;
    QString hostUrl() const;
    QString viewerUrl() const;
    QString expiresAt() const;
    QString mcpEndpoint() const;

    Q_INVOKABLE void createMcp();
    Q_INVOKABLE void endSession();
    Q_INVOKABLE void openHostLink();
    Q_INVOKABLE void openViewerLink();
    Q_INVOKABLE void openMcpEndpoint();
    Q_INVOKABLE void copyText(const QString &text);
    Q_INVOKABLE QString connectionInstructions() const;

signals:
    void stateChanged();

private:
    void setStatus(const QString &status);
    bool openSafeUrl(const QString &url, const QString &expectedHost);

    QNetworkAccessManager m_network;
    QString m_status = QStringLiteral("Ready to create a secure screen-share session.");
    QString m_sessionId;
    QString m_hostUrl;
    QString m_viewerUrl;
    QString m_viewerToken;
    QString m_expiresAt;
    QTimer m_expiryTimer;
    bool m_busy = false;
};
