#pragma once
#include <QObject>
#include <QVariantList>
#include <QProcess>
class SystemInfo final : public QObject {
 Q_OBJECT
 Q_PROPERTY(QString distribution READ distribution CONSTANT)
 Q_PROPERTY(QString distroId READ distroId CONSTANT)
 Q_PROPERTY(QString family READ family CONSTANT)
 Q_PROPERTY(QString version READ version CONSTANT)
 Q_PROPERTY(QString packageManager READ packageManager CONSTANT)
 Q_PROPERTY(bool managerAvailable READ managerAvailable CONSTANT)
 Q_PROPERTY(QString kernel READ kernel CONSTANT)
 Q_PROPERTY(QString architecture READ architecture CONSTANT)
 Q_PROPERTY(QString hostName READ hostName CONSTANT)
 Q_PROPERTY(QVariantList applications READ applications NOTIFY applicationsChanged)
 Q_PROPERTY(QVariantList nativeTools READ nativeTools CONSTANT)
 Q_PROPERTY(QString sessionType READ sessionType CONSTANT)
 Q_PROPERTY(QString desktopEnvironment READ desktopEnvironment CONSTANT)
 Q_PROPERTY(QString waylandDisplay READ waylandDisplay CONSTANT)
 Q_PROPERTY(QString xDisplay READ xDisplay CONSTANT)
 Q_PROPERTY(QString runtimeDirectory READ runtimeDirectory CONSTANT)
 Q_PROPERTY(QString runtimeDirectoryStatus READ runtimeDirectoryStatus CONSTANT)
 Q_PROPERTY(QVariantList availableCompositors READ availableCompositors CONSTANT)
 Q_PROPERTY(bool qtWaylandPluginAvailable READ qtWaylandPluginAvailable CONSTANT)
 Q_PROPERTY(QString status READ status NOTIFY statusChanged)
 Q_PROPERTY(bool previewRunning READ previewRunning NOTIFY previewStateChanged)
 Q_PROPERTY(bool lightAppearance READ lightAppearance WRITE setLightAppearance NOTIFY appearanceChanged)
 Q_PROPERTY(bool wineAvailable READ wineAvailable CONSTANT)
 Q_PROPERTY(QString winePath READ winePath CONSTANT)
 Q_PROPERTY(bool bottlesAvailable READ bottlesAvailable CONSTANT)
 Q_PROPERTY(QString bottlesPath READ bottlesPath CONSTANT)
public:
 explicit SystemInfo(QObject *parent=nullptr);
 QString distribution() const; QString distroId() const; QString family() const; QString version() const;
 QString packageManager() const; bool managerAvailable() const; QString kernel() const;
 QString architecture() const; QString hostName() const; QVariantList applications() const; QVariantList nativeTools() const; QString status() const; bool previewRunning() const;
 QString sessionType() const; QString desktopEnvironment() const; QString waylandDisplay() const; QString xDisplay() const;
 QString runtimeDirectory() const; QString runtimeDirectoryStatus() const; QVariantList availableCompositors() const; bool qtWaylandPluginAvailable() const;
 bool lightAppearance() const; void setLightAppearance(bool enabled);
 bool wineAvailable() const; QString winePath() const; bool bottlesAvailable() const; QString bottlesPath() const;
 Q_INVOKABLE QString previewReadiness() const;
 Q_INVOKABLE bool launchNestedPreview();
 Q_INVOKABLE QString standaloneReadiness() const;
 Q_INVOKABLE QString diagnosticReport() const;
 Q_INVOKABLE void copyDiagnosticReport() const;
 Q_INVOKABLE QVariantList recentNotifications() const;
 Q_INVOKABLE void saveRecentNotifications(const QVariantList &notifications);
 Q_INVOKABLE void refreshApplications();
 Q_INVOKABLE bool launchDesktopFile(const QString &path);
 Q_INVOKABLE bool launchWindowsFile(const QString &path);
 Q_INVOKABLE bool launchTerminal();
signals: void applicationsChanged(); void statusChanged(); void appearanceChanged(); void previewStateChanged();
private:
 QString m_distribution,m_id,m_family,m_version,m_manager,m_kernel,m_arch,m_host,m_status,m_winePath,m_bottlesPath;
 QString m_sessionType,m_desktopEnvironment,m_waylandDisplay,m_xDisplay,m_runtimeDirectory,m_runtimeDirectoryStatus;
 bool m_managerAvailable=false; bool m_lightAppearance=false; bool m_qtWaylandPluginAvailable=false; QVariantList m_apps,m_nativeTools,m_availableCompositors;
 QProcess m_previewProcess; QString m_previewOutput;
 void setStatus(const QString &s);
 void discoverNativeTools();
 void discoverSession();
};
