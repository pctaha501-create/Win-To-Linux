#pragma once
#include "distro_detector.h"
#include <QObject>
#include <QProcess>
#include <QTimer>
namespace wtl { class PackageCommandPlanner; }
class PackageOperationController final : public QObject {
 Q_OBJECT
 Q_PROPERTY(bool running READ running NOTIFY runningChanged)
 Q_PROPERTY(QString status READ status NOTIFY statusChanged)
 Q_PROPERTY(QString output READ output NOTIFY outputChanged)
public:
 explicit PackageOperationController(QObject *parent=nullptr);
 PackageOperationController(const wtl::DistroInfo &distro,QObject *parent=nullptr);
 bool running() const;
 QString status() const;
 QString output() const;
 Q_INVOKABLE bool checkUpdates();
 Q_INVOKABLE bool installPackage(const QString &name);
 Q_INVOKABLE bool removePackage(const QString &name);
 Q_INVOKABLE void cancel();
signals:
 void runningChanged();
 void statusChanged();
 void outputChanged();
private:
 wtl::DistroInfo m_distro;
 QProcess m_process;
 QTimer m_timeout;
 QTimer m_killTimer;
 bool m_running=false;
 bool m_timedOut=false;
 bool m_cancelled=false;
 bool m_finalized=false;
 QString m_status,m_output,m_operation;
 void setStatus(const QString &value);
 void appendOutput(const QString &value);
 bool startPlan(const QString &operation,const QString &program,const QStringList &arguments,int timeoutMs,bool supported,const QString &error);
 void finishProcess(int exitCode,QProcess::ExitStatus exitStatus);
 void processError(QProcess::ProcessError error);
};
