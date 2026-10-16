#pragma once
#include <QString>
#include <QStringList>

struct SessionEnvironment {
    bool linuxHost = false;
    bool explicitOptIn = false;
    bool x11DisplaySet = false;
    bool waylandDisplaySet = false;
    bool realVirtualTerminal = false;
    bool runtimeDirectoryPrivate = false;
    bool westonAvailable = false;
    bool qtWaylandPluginAvailable = false;
    bool applicationExecutableVerified = false;
    bool drmDeviceAccessible = false;
    bool waylandSocketVerified = false;
    bool cageAvailable = false;
    QString waylandSocketPath;
    QString ttyPath;
    QString runtimeDirectory;
};

struct SessionBackendResult {
    QString backendId;
    QString displayName;
    QString summary;
    QStringList blockers;
    bool ready() const { return blockers.isEmpty(); }
    QString report() const;
};

class SessionBackend {
public:
    virtual ~SessionBackend() = default;
    virtual QString id() const = 0;
    virtual QString displayName() const = 0;
    virtual SessionBackendResult probe(const SessionEnvironment &environment) const = 0;
};

class WestonNestedSessionBackend final : public SessionBackend {
public:
    QString id() const override;
    QString displayName() const override;
    SessionBackendResult probe(const SessionEnvironment &environment) const override;
};

class WestonDrmSessionBackend final : public SessionBackend {
public:
    QString id() const override;
    QString displayName() const override;
    SessionBackendResult probe(const SessionEnvironment &environment) const override;
};
