#include "process_monitor.h"
#include <QCoreApplication>
#include <QHash>
#include <QProcess>
#include <QThread>
#include <QFile>
#include <unistd.h>
#include <QDebug>
namespace { bool expect(bool ok,const char *message){if(!ok)qCritical().noquote()<<"FAIL:"<<message;return ok;} }
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);
 ProcessMonitor monitor;
 if(!expect(!monitor.processes().isEmpty(),"procfs returns at least one process"))return 1;
 bool foundSelf=false;
 QHash<qint64,int> positions;
 QHash<qint64,int> depths;
 int row=0;
 for(const QVariant &entry:monitor.processes()){
  const QVariantMap process=entry.toMap();
  positions.insert(process.value(QStringLiteral("pid")).toLongLong(),row++);
  depths.insert(process.value(QStringLiteral("pid")).toLongLong(),process.value(QStringLiteral("depth")).toInt());
  if(!expect(process.value(QStringLiteral("pid")).toLongLong()>0,"positive PID"))return 1;
  if(process.value(QStringLiteral("pid")).toLongLong()==QCoreApplication::applicationPid()) foundSelf=process.value(QStringLiteral("parentPid")).toLongLong()>=0;
  if(!expect(process.value(QStringLiteral("parentPid")).toLongLong()>=-1,"parent PID metadata is valid or unavailable during process exit"))return 1;
  if(!expect(process.value(QStringLiteral("depth")).toInt()>=0&&process.value(QStringLiteral("depth")).toInt()<=32,"bounded process tree depth"))return 1;
  if(!expect(!process.value(QStringLiteral("name")).toString().isEmpty(),"nonempty process name"))return 1;
  if(!expect(process.contains(QStringLiteral("startTicks")),"process identity includes start time"))return 1;
  if(!expect(process.contains(QStringLiteral("cpuPercent")),"process exposes CPU utilization"))return 1;
  const double cpu=process.value(QStringLiteral("cpuPercent")).toDouble();
  if(!expect(cpu>=0.0&&cpu<=100.0,"normalized CPU utilization stays between 0 and 100 percent"))return 1;
  if(!expect(!process.value(QStringLiteral("cpuText")).toString().isEmpty(),"CPU utilization has display text"))return 1;
 }
 for(int i=0;i<monitor.processes().size();++i){
  const QVariantMap process=monitor.processes().at(i).toMap();
  const qint64 ppid=process.value(QStringLiteral("parentPid")).toLongLong();
  const qint64 pid=process.value(QStringLiteral("pid")).toLongLong();
  const int depth=process.value(QStringLiteral("depth")).toInt();
  if(depth>0&&positions.contains(ppid)){
   if(!expect(positions.value(ppid)<i,"parent process appears before its child"))return 1;
   if(!expect(depth==depths.value(ppid)+1,"child depth follows parent depth"))return 1;
  }
  Q_UNUSED(pid);
 }
 if(!expect(foundSelf,"the test process has a valid parent PID"))return 1;
 if(!expect(!monitor.status().isEmpty(),"monitor status is reported"))return 1;
 if(!expect(monitor.totalCpuPercent()>=0.0&&monitor.totalCpuPercent()<=100.0,"overall CPU utilization is normalized"))return 1;
 monitor.refreshProcesses();
 if(!expect(monitor.totalCpuPercent()>=0.0&&monitor.totalCpuPercent()<=100.0,"aggregate CPU remains normalized after refresh"))return 1;
 if(!expect(monitor.memoryPercent()>=0.0&&monitor.memoryPercent()<=100.0,"RAM utilization is normalized"))return 1;
 if(!expect(!monitor.memoryText().isEmpty(),"RAM usage has display text"))return 1;
 QProcess child;
 child.start(QStringLiteral("/bin/sleep"),QStringList{QStringLiteral("30")});
 if(!expect(child.waitForStarted(1500),"start a controlled child process for signal-safety tests"))return 1;
 const qint64 childPid=child.processId();monitor.refreshProcesses();qint64 childTicks=-1;
 for(const QVariant &entry:monitor.processes()){const QVariantMap process=entry.toMap();if(process.value(QStringLiteral("pid")).toLongLong()==childPid){childTicks=process.value(QStringLiteral("startTicks")).toLongLong();break;}}
 if(!expect(childTicks>=0,"discover the child process start-time identity")){child.kill();child.waitForFinished(1000);return 1;}
 if(!expect(!monitor.terminateProcess(childPid,childTicks+1),"reject stale process identity before sending a signal")){child.kill();child.waitForFinished(1000);return 1;}
 if(!expect(monitor.status().contains(QStringLiteral("Process changed or exited")),"retain the stale-process explanation after refreshing the process list")){child.kill();child.waitForFinished(1000);return 1;}
 if(child.state()==QProcess::NotRunning){qCritical()<<"FAIL: stale identity check unexpectedly stopped the child";return 1;}
 const bool stopped=monitor.terminateProcess(childPid,childTicks);
 if(stopped){if(!child.waitForFinished(2000)){child.kill();child.waitForFinished(1000);qCritical()<<"FAIL: PID-bound SIGTERM did not stop the child";return 1;}}
 else {
  const QString refusalStatus=monitor.status();
  QFile procStatus(QStringLiteral("/proc/%1/status").arg(childPid));QString uidLine;
  if(procStatus.open(QIODevice::ReadOnly|QIODevice::Text)){while(!procStatus.atEnd()){const QByteArray line=procStatus.readLine();if(line.startsWith("Uid:")){uidLine=QString::fromLocal8Bit(line).trimmed();break;}}}
  const bool explained=refusalStatus.contains(QStringLiteral("safe"),Qt::CaseInsensitive)||
   refusalStatus.contains(QStringLiteral("exited"),Qt::CaseInsensitive)||
   refusalStatus.contains(QStringLiteral("changed"),Qt::CaseInsensitive)||
   refusalStatus.contains(QStringLiteral("no signal"),Qt::CaseInsensitive);
  if(!explained)qCritical().noquote()<<"Unexpected process-stop refusal status:"<<refusalStatus<<"current UID:"<<getuid()<<"child proc status:"<<uidLine;
  if(child.state()!=QProcess::NotRunning){child.kill();child.waitForFinished(1000);}
  if(!expect(explained,"failed process termination retains a safe refusal explanation"))return 1;
 }
 qInfo()<<"Process monitor tests passed";return 0;
}
