#include "system_info.h"
#include "process_monitor.h"
#include "package_operation_controller.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <cstring>

namespace {
bool hasArgument(int argc,char **argv,const char *wanted){for(int i=1;i<argc;++i)if(std::strcmp(argv[i],wanted)==0)return true;return false;}
void configureApplication(){QCoreApplication::setApplicationName("Win-To-Linux");QCoreApplication::setApplicationVersion("0.2.0");QCoreApplication::setOrganizationName("Win-To-Linux");}
void printSystemInfo(const SystemInfo &system){
 qInfo().noquote()<<"Distribution:"<<system.distribution();
 qInfo().noquote()<<"ID:"<<system.distroId();
 qInfo().noquote()<<"Family:"<<system.family();
 qInfo().noquote()<<"Version:"<<system.version();
 qInfo().noquote()<<"Package manager:"<<system.packageManager();
 qInfo().noquote()<<"Manager available:"<<(system.managerAvailable()?"yes":"no");
 qInfo().noquote()<<"Kernel:"<<system.kernel();
 qInfo().noquote()<<"Architecture:"<<system.architecture();
 qInfo().noquote()<<"Native tools:"<<system.nativeTools().size();
}
}
int main(int argc,char**argv){
 if(hasArgument(argc,argv,"--system")){
  QCoreApplication app(argc,argv);configureApplication();SystemInfo system;printSystemInfo(system);return 0;
 }
 QGuiApplication app(argc,argv);configureApplication();
 SystemInfo system;ProcessMonitor processMonitor;PackageOperationController packageOperations;
 QQmlApplicationEngine engine;
 engine.rootContext()->setContextProperty("systemInfo",&system);
 engine.rootContext()->setContextProperty("processMonitor",&processMonitor);
 engine.rootContext()->setContextProperty("packageOperations",&packageOperations);
 QObject::connect(&engine,&QQmlApplicationEngine::objectCreationFailed,&app,[](){QCoreApplication::exit(1);},Qt::QueuedConnection);
 engine.load(QUrl("qrc:/qml/Main.qml"));
 if(engine.rootObjects().isEmpty()){qCritical()<<"Unable to load Win-To-Linux shell.";return 1;}
 if(hasArgument(argc,argv,"--smoke-test")){qInfo()<<"QML shell loaded successfully.";return 0;}
 return app.exec();
}
