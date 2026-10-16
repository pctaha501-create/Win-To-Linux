#include "session_backend.h"
#include <QCoreApplication>
#include <QDebug>

namespace {
bool expect(bool condition, const char *message)
{
    if (!condition) qCritical().noquote() << "FAIL:" << message;
    return condition;
}
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    WestonDrmSessionBackend backend;
    SessionEnvironment ready;
    ready.linuxHost = true;
    ready.explicitOptIn = true;
    ready.realVirtualTerminal = true;
    ready.runtimeDirectoryPrivate = true;
    ready.westonAvailable = true;
    ready.qtWaylandPluginAvailable = true;
    ready.applicationExecutableVerified = true;
    ready.drmDeviceAccessible = true;

    bool ok = true;
    const SessionBackendResult good = backend.probe(ready);
    ok &= expect(backend.id() == QStringLiteral("weston-drm"), "backend id is stable");
    ok &= expect(backend.displayName().contains(QStringLiteral("experimental")), "backend is explicitly marked experimental");
    ok &= expect(good.ready(), "all valid prerequisites permit readiness");
    ok &= expect(good.blockers.isEmpty(), "ready backend has no blockers");
    ok &= expect(good.summary.contains(QStringLiteral("disposable VM")), "ready message keeps the VM warning");
    ok &= expect(good.report().contains(QStringLiteral("weston-drm")), "backend diagnostic report identifies backend");

    struct Case { bool SessionEnvironment::*field; const char *message; };
    const Case cases[] = {
        {&SessionEnvironment::linuxHost, "Linux-only guard"},
        {&SessionEnvironment::explicitOptIn, "explicit opt-in guard"},
        {&SessionEnvironment::realVirtualTerminal, "real VT guard"},
        {&SessionEnvironment::runtimeDirectoryPrivate, "private runtime directory guard"},
        {&SessionEnvironment::westonAvailable, "Weston dependency guard"},
        {&SessionEnvironment::qtWaylandPluginAvailable, "Qt Wayland plugin guard"},
        {&SessionEnvironment::applicationExecutableVerified, "application executable guard"},
        {&SessionEnvironment::drmDeviceAccessible, "DRM device access guard"}
    };
    for (const Case &test : cases) {
        SessionEnvironment invalid = ready;
        invalid.*(test.field) = false;
        const SessionBackendResult result = backend.probe(invalid);
        ok &= expect(!result.ready(), test.message);
        ok &= expect(!result.blockers.isEmpty(), "blocked backend explains its refusal");
        ok &= expect(result.summary.startsWith(QStringLiteral("Not ready for standalone session:")), "blocked result uses the stable refusal prefix");
    }
    WestonNestedSessionBackend nestedBackend;
    SessionEnvironment nestedReady;
    nestedReady.runtimeDirectoryPrivate = true;
    nestedReady.waylandSocketVerified = true;
    nestedReady.waylandSocketPath = QStringLiteral("/run/user/1000/wayland-0");
    nestedReady.qtWaylandPluginAvailable = true;
    nestedReady.westonAvailable = true;
    nestedReady.applicationExecutableVerified = true;
    const SessionBackendResult nestedGood = nestedBackend.probe(nestedReady);
    ok &= expect(nestedBackend.id() == QStringLiteral("weston-nested"), "nested backend id is stable");
    ok &= expect(nestedGood.ready(), "valid active Wayland desktop permits nested preview");
    ok &= expect(nestedGood.summary.contains(QStringLiteral("multi-window desktop")), "Weston is preferred when available");
    SessionEnvironment cageOnly = nestedReady;
    cageOnly.westonAvailable = false;
    cageOnly.cageAvailable = true;
    ok &= expect(nestedBackend.probe(cageOnly).summary.contains(QStringLiteral("using Cage")), "Cage is an explicit fallback only when available");
    SessionEnvironment unsafeNested = nestedReady;
    unsafeNested.waylandSocketVerified = false;
    unsafeNested.runtimeDirectoryPrivate = false;
    unsafeNested.westonAvailable = false;
    unsafeNested.cageAvailable = false;
    const SessionBackendResult nestedBlocked = nestedBackend.probe(unsafeNested);
    ok &= expect(!nestedBlocked.ready(), "nested preview rejects unsafe or unavailable environment");
    ok &= expect(nestedBlocked.blockers.size() == 3, "nested preview reports every independent blocker");
    ok &= expect(nestedBlocked.summary.startsWith(QStringLiteral("Not ready for nested preview:")), "nested refusal prefix remains stable");

    SessionEnvironment existingDesktop = ready;
    existingDesktop.x11DisplaySet = true;
    existingDesktop.waylandDisplaySet = true;
    const SessionBackendResult conflict = backend.probe(existingDesktop);
    ok &= expect(conflict.blockers.size() == 2, "existing X11 and Wayland displays are both reported");
    ok &= expect(conflict.summary.contains(QStringLiteral("DISPLAY is set")) && conflict.summary.contains(QStringLiteral("WAYLAND_DISPLAY is set")), "display conflicts are explicit");
    if (!ok) return 1;
    qInfo() << "Session backend interface and safety-preflight tests passed.";
    return 0;
}
