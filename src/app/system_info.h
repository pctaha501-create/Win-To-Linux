#pragma once
#include <QObject>
#include <QVariantList>
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
 Q_PROPERTY(QString status READ status NOTIFY statusChanged)
public:
 explicit SystemInfo(QObject *parent=nullptr);
 QString distribution() const; QString distroId() const; QString family() const; QString version() const;
 QString packageManager() const; bool managerAvailable() const; QString kernel() const;
 QString architecture() const; QString hostName() const; QVariantList applications() const; QString status() const;
 Q_INVOKABLE void refreshApplications();
 Q_INVOKABLE bool launchDesktopFile(const QString &path);
signals: void applicationsChanged(); void statusChanged();
private:
 QString m_distribution,m_id,m_family,m_version,m_manager,m_kernel,m_arch,m_host,m_status;
 bool m_managerAvailable=false; QVariantList m_apps;
 void setStatus(const QString &s);
};
