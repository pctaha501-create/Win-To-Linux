#include "process_monitor.h"
#include <QCoreApplication>
#include <QDebug>
namespace { bool expect(bool ok,const char *message){if(!ok)qCritical().noquote()<<"FAIL:"<<message;return ok;} }
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);
 ProcessMonitor monitor;
 if(!expect(!monitor.processes().isEmpty(),"procfs returns at least one process"))return 1;
 bool foundSelf=false;
 for(const QVariant &entry:monitor.processes()){
  const QVariantMap process=entry.toMap();
  if(!expect(process.value(QStringLiteral("pid")).toLongLong()>0,"positive PID"))return 1;
  if(process.value(QStringLiteral("pid")).toLongLong()==QCoreApplication::applicationPid()) foundSelf=process.value(QStringLiteral("parentPid")).toLongLong()>=0;
  if(!expect(process.value(QStringLiteral("parentPid")).toLongLong()>=-1,"parent PID metadata is valid or unavailable during process exit"))return 1;
  if(!expect(!process.value(QStringLiteral("name")).toString().isEmpty(),"nonempty process name"))return 1;
  if(!expect(process.contains(QStringLiteral("startTicks")),"process identity includes start time"))return 1;
 }
 if(!expect(foundSelf,"the test process has a valid parent PID"))return 1;
 if(!expect(!monitor.status().isEmpty(),"monitor status is reported"))return 1;
 qInfo()<<"Process monitor tests passed";return 0;
}
