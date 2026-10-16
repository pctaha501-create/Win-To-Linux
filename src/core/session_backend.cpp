#include "session_backend.h"

SessionBackendResult WestonDrmSessionBackend::probe(const SessionEnvironment &environment) const
{
    SessionBackendResult result;
    result.backendId = id();
    result.displayName = displayName();

    if (!environment.linuxHost)
        result.blockers.append(QStringLiteral("The standalone compositor session is supported only on Linux"));
    if (!environment.explicitOptIn)
        result.blockers.append(QStringLiteral("Explicit opt-in is missing: set WTL_ENABLE_DRM_SESSION=1 only when intentionally starting the experimental session"));
    if (environment.x11DisplaySet)
        result.blockers.append(QStringLiteral("DISPLAY is set; an X11 desktop may be active"));
    if (environment.waylandDisplaySet)
        result.blockers.append(QStringLiteral("WAYLAND_DISPLAY is set; an existing Wayland desktop may be active"));
    if (!environment.realVirtualTerminal)
        result.blockers.append(QStringLiteral("Not running from a real Linux virtual terminal such as /dev/tty3"));
    if (!environment.runtimeDirectoryPrivate)
        result.blockers.append(QStringLiteral("XDG_RUNTIME_DIR is missing or is not a private directory owned by the current user"));
    if (!environment.westonAvailable)
        result.blockers.append(QStringLiteral("Weston is not installed"));
    if (!environment.qtWaylandPluginAvailable)
        result.blockers.append(QStringLiteral("Qt Wayland platform plugin is missing; install the distribution's Qt Wayland runtime package (Ubuntu: qt6-wayland)"));
    if (!environment.applicationExecutableVerified)
        result.blockers.append(QStringLiteral("The Win-To-Linux executable path could not be verified"));
    if (!environment.drmDeviceAccessible)
        result.blockers.append(QStringLiteral("No accessible /dev/dri/card* device was found; this environment may not support a DRM session"));

    result.summary = result.ready()
        ? QStringLiteral("Ready for experimental standalone session preflight. This is not a guarantee of working input, graphics, suspend/resume or application compatibility; test in a disposable VM first.")
        : QStringLiteral("Not ready for standalone session:\n• %1\n\nThis check is read-only. It will not start Weston, access DRM, install packages, or change the login manager.").arg(result.blockers.join(QStringLiteral("\n• ")));
    return result;
}

SessionBackendResult WestonNestedSessionBackend::probe(const SessionEnvironment &environment) const
{
    SessionBackendResult result;
    result.backendId = id();
    result.displayName = displayName();
    if (!environment.runtimeDirectoryPrivate)
        result.blockers.append(QStringLiteral("XDG_RUNTIME_DIR is missing or fails ownership/permission checks"));
    if (!environment.waylandSocketVerified)
        result.blockers.append(environment.waylandSocketPath.isEmpty()
            ? QStringLiteral("No active Wayland display socket is configured; this launcher does not start a physical-display compositor")
            : QStringLiteral("The configured Wayland socket is missing, is not a Unix socket, or is not owned by the current user: %1").arg(environment.waylandSocketPath));
    if (!environment.qtWaylandPluginAvailable)
        result.blockers.append(QStringLiteral("Qt Wayland platform plugin is missing; install the distribution's Qt Wayland runtime package (Ubuntu: qt6-wayland)"));
    if (!environment.westonAvailable && !environment.cageAvailable)
        result.blockers.append(QStringLiteral("Neither Weston nor Cage is installed"));
    if (!environment.applicationExecutableVerified)
        result.blockers.append(QStringLiteral("The Win-To-Linux executable path could not be verified"));
    if (result.ready())
        result.summary = environment.westonAvailable
            ? QStringLiteral("Ready for nested preview using Weston (multi-window desktop). This is not an independent login session.")
            : QStringLiteral("Ready for nested preview using Cage. This is not an independent desktop session.");
    else
        result.summary = QStringLiteral("Not ready for nested preview:\n• %1\n\nThis checker is read-only; it will not install packages or alter your login manager.").arg(result.blockers.join(QStringLiteral("\n• ")));
    return result;
}

QString WestonNestedSessionBackend::id() const
{
    return QStringLiteral("weston-nested");
}

QString WestonNestedSessionBackend::displayName() const
{
    return QStringLiteral("Weston nested preview");
}

QString WestonDrmSessionBackend::id() const
{
    return QStringLiteral("weston-drm");
}

QString WestonDrmSessionBackend::displayName() const
{
    return QStringLiteral("Weston DRM (experimental)");
}

QString SessionBackendResult::report() const
{
    return QStringLiteral("Backend: %1 (%2)\nStatus: %3\n%4")
        .arg(displayName, backendId, ready() ? QStringLiteral("ready") : QStringLiteral("blocked"), summary);
}

std::unique_ptr<SessionBackend> createSessionBackend(const QString &backendId)
{
    if (backendId == QStringLiteral("weston-nested"))
        return std::make_unique<WestonNestedSessionBackend>();
    if (backendId == QStringLiteral("weston-drm"))
        return std::make_unique<WestonDrmSessionBackend>();
    return {};
}

QStringList supportedSessionBackendIds()
{
    return {QStringLiteral("weston-nested"), QStringLiteral("weston-drm")};
}
