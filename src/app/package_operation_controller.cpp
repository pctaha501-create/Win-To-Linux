#include "package_operation_controller.h"
#include "distro_detector.h"
#include "package_command_planner.h"
#include <QTextStream>
PackageOperationController::PackageOperationController(QObject *parent):QObject(parent),m_distro(wtl::DistroDetector::detect()){
 m_timeout.setSingleShot(true);m_killTimer.setSingleShot(true);
 connect(&m_timeout,&QTimer::timeout,this,[this](){if(!m_running)return;m_timedOut=true;setStatus(QStringLiteral("Operation timed out; asking the package manager to stop."));m_process.terminate();m_killTimer.start(1500);});
 connect(&m_killTimer,&QTimer::timeout,this,[this](){if(m_running&&m_process.state()!=QProcess::NotRunning)m_process.kill();});
 connect(&m_process,&QProcess::readyReadStandardOutput,this,[this](){appendOutput(QString::fromLocal8Bit(m_process.readAllStandardOutput()));});
 connect(&m_process,&QProcess::readyReadStandardError,this,[this](){appendOutput(QString::fromLocal8Bit(m_process.readAllStandardError()));});
 connect(&m_process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus status){finishProcess(code,status);});
 connect(&m_process,&QProcess::errorOccurred,this,[this](QProcess::ProcessError error){processError(error);});
}
bool PackageOperationController::running()const{return m_running;}
QString PackageOperationController::status()const{return m_status;}
QString PackageOperationController::output()const{return m_output;}
void PackageOperationController::setStatus(const QString&value){if(m_status==value)return;m_status=value;emit statusChanged();}
void PackageOperationController::appendOutput(const QString&value){
 if(value.isEmpty())return;
 constexpr qsizetype limit=60000;
 m_output+=value;
 if(m_output.size()>limit)m_output=QStringLiteral("[Earlier output truncated]\n")+m_output.right(limit);
 emit outputChanged();
}
bool PackageOperationController::startPlan(const QString&operation,const QString&program,const QStringList&arguments,int timeoutMs,bool supported,const QString&error){
 if(m_running){setStatus(QStringLiteral("Another package operation is already running."));return false;}
 if(!supported||program.isEmpty()){setStatus(error.isEmpty()?QStringLiteral("This operation is not supported on the detected system."):error);return false;}
 m_operation=operation;m_output.clear();emit outputChanged();m_timedOut=false;m_cancelled=false;m_finalized=false;
 m_process.setProgram(program);m_process.setArguments(arguments);m_process.setWorkingDirectory(QDir::homePath());m_process.setProcessChannelMode(QProcess::SeparateChannels);
 m_running=true;emit runningChanged();setStatus(QStringLiteral("%1 started.").arg(operation));m_timeout.start(qBound(1000,timeoutMs,180000));
 m_process.start();
 return true;
}
bool PackageOperationController::checkUpdates(){
 const auto plan=wtl::PackageCommandPlanner::checkUpdates(m_distro);
 return startPlan(QStringLiteral("Checking available updates"),plan.program,plan.arguments,plan.timeoutMs,plan.supported,plan.error);
}
bool PackageOperationController::installPackage(const QString&name){
 const auto plan=wtl::PackageCommandPlanner::install(m_distro,name);
 return startPlan(QStringLiteral("Installing %1").arg(name),plan.program,plan.arguments,plan.timeoutMs,plan.supported,plan.error);
}
bool PackageOperationController::removePackage(const QString&name){
 const auto plan=wtl::PackageCommandPlanner::remove(m_distro,name);
 return startPlan(QStringLiteral("Removing %1").arg(name),plan.program,plan.arguments,plan.timeoutMs,plan.supported,plan.error);
}
void PackageOperationController::cancel(){
 if(!m_running)return;
 m_cancelled=true;m_timeout.stop();setStatus(QStringLiteral("Cancellation requested; waiting for the package manager to stop."));m_process.terminate();m_killTimer.start(1500);
}
void PackageOperationController::processError(QProcess::ProcessError error){
 if(error==QProcess::FailedToStart&&!m_finalized){
  m_finalized=true;m_timeout.stop();m_killTimer.stop();m_running=false;emit runningChanged();
  setStatus(QStringLiteral("Could not start package operation. Check the package manager and PolicyKit authentication agent."));
  appendOutput(m_process.errorString()+QLatin1Char('\n'));
 }
}
void PackageOperationController::finishProcess(int exitCode,QProcess::ExitStatus exitStatus){
 if(m_finalized)return;
 m_finalized=true;m_timeout.stop();m_killTimer.stop();
 appendOutput(QString::fromLocal8Bit(m_process.readAllStandardOutput()));
 appendOutput(QString::fromLocal8Bit(m_process.readAllStandardError()));
 m_running=false;emit runningChanged();
 if(m_cancelled)setStatus(QStringLiteral("Operation cancelled."));
 else if(m_timedOut)setStatus(QStringLiteral("Operation timed out and has stopped."));
 else if(exitStatus==QProcess::CrashExit)setStatus(QStringLiteral("Package manager exited unexpectedly."));
 else if(exitCode==0)setStatus(QStringLiteral("%1 completed successfully.").arg(m_operation));
 else if((m_distro.packageManager==QStringLiteral("dnf")||m_distro.packageManager==QStringLiteral("yum"))&&m_operation.startsWith(QStringLiteral("Checking"))&&exitCode==100)setStatus(QStringLiteral("Updates are available; see output."));
 else setStatus(QStringLiteral("%1 failed with exit code %2. See output.").arg(m_operation).arg(exitCode));
}
