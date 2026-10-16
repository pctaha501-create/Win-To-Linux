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
    if (!controller.connectionInstructions().isEmpty()) {
        qCritical() << "Connection instructions must not be available before a session exists.";
        return 1;
    }
    qInfo() << "Screen-share controller starts safely and does not create sessions implicitly.";
    return 0;
}
