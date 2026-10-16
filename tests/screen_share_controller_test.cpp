#include "screen_share_controller.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ScreenShareController controller;
    if (controller.busy() || controller.ready() || !controller.sessionId().isEmpty()
        || !controller.hostUrl().isEmpty() || !controller.viewerUrl().isEmpty()) {
        qCritical() << "Screen-share controller must start without an active session.";
        return 1;
    }
    if (controller.mcpEndpoint() != QStringLiteral("https://mcp.the-end.shop/mcp")) {
        qCritical() << "Unexpected remote MCP endpoint.";
        return 1;
    }
    if (!controller.connectionInstructions().isEmpty() || !controller.expiresAt().isEmpty()) {
        qCritical() << "Connection instructions and expiry must not be available before a session exists.";
        return 1;
    }
    controller.endSession();
    if (controller.busy() || controller.ready() || !controller.status().contains(QStringLiteral("no active session"))) {
        qCritical() << "Ending a missing session should fail safely without a network request.";
        return 1;
    }
    qInfo() << "Screen-share controller starts safely and does not create sessions implicitly.";
    return 0;
}
