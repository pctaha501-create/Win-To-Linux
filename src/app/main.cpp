#include "system_info.h"
#include "process_monitor.h"
#include "package_operation_controller.h"
#include "file_manager.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QDebug>
#include <QWindow>
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
 qInfo().noquote()<<"Session type:"<<system.sessionType();
 qInfo().noquote()<<"Desktop environment:"<<system.desktopEnvironment();
 qInfo().noquote()<<"Wayland display:"<<system.waylandDisplay();
 qInfo().noquote()<<"X11 display:"<<system.xDisplay();
 qInfo().noquote()<<"Runtime directory:"<<system.runtimeDirectory();
 qInfo().noquote()<<"Runtime directory check:"<<system.runtimeDirectoryStatus();
 qInfo().noquote()<<"Compositors found:"<<system.availableCompositors().size();
 for(const QVariant &entry:system.availableCompositors())qInfo().noquote()<<"Compositor:"<<entry.toMap().value(QStringLiteral("name")).toString()<<entry.toMap().value(QStringLiteral("path")).toString();
}
}
int main(int argc,char**argv){
 if(hasArgument(argc,argv,"--system")||hasArgument(argc,argv,"--session-check")||hasArgument(argc,argv,"--diagnostic-report")||hasArgument(argc,argv,"--preview-check")){
  QCoreApplication app(argc,argv);configureApplication();SystemInfo system;
  if(hasArgument(argc,argv,"--diagnostic-report"))qInfo().noquote()<<system.diagnosticReport();
  else if(hasArgument(argc,argv,"--preview-check")){
   const QString readiness=system.previewReadiness();qInfo().noquote()<<readiness;
   return readiness.startsWith(QStringLiteral("Ready for nested preview"))?0:2;
  }else printSystemInfo(system);
  return 0;
 }
 QGuiApplication app(argc,argv);configureApplication();
 SystemInfo system;ProcessMonitor processMonitor;PackageOperationController packageOperations;FileManager fileManager;
 QQmlApplicationEngine engine;
 engine.rootContext()->setContextProperty("systemInfo",&system);
 engine.rootContext()->setContextProperty("processMonitor",&processMonitor);
 engine.rootContext()->setContextProperty("packageOperations",&packageOperations);
 engine.rootContext()->setContextProperty("fileManager",&fileManager);
 QObject::connect(&engine,&QQmlApplicationEngine::objectCreationFailed,&app,[](){QCoreApplication::exit(1);},Qt::QueuedConnection);
 engine.load(QUrl("qrc:/qml/Main.qml"));
 if(engine.rootObjects().isEmpty()){qCritical()<<"Unable to load Win-To-Linux shell.";return 1;}
 if(hasArgument(argc,argv,"--smoke-test")){
  QObject *root=engine.rootObjects().constFirst();
  if(!root->findChild<QObject *>(QStringLiteral("workspaceSwitcher"))||
     !root->findChild<QObject *>(QStringLiteral("workspaceSwitcherShortcut"))||
     !root->findChild<QObject *>(QStringLiteral("previousWorkspaceShortcut"))){
   qCritical()<<"Workspace switcher or keyboard shortcuts were not instantiated.";return 1;
  }
  const QStringList pageUrls{QStringLiteral("qrc:/qml/FileManagerPage.qml"),QStringLiteral("qrc:/qml/PackageManagerPage.qml")};
  for(const QString &url:pageUrls){
   QQmlComponent component(&engine,QUrl(url));
   if(component.isError()){
    qCritical().noquote()<<"QML component failed:"<<url;
    for(const QQmlError &error:component.errors())qCritical().noquote()<<error.toString();
    return 1;
   }
   QObject *page=component.create(engine.rootContext());
   if(!page){
    qCritical().noquote()<<"QML component could not be instantiated:"<<url;
    for(const QQmlError &error:component.errors())qCritical().noquote()<<error.toString();
    return 1;
   }
   delete page;
  }
  qInfo()<<"QML shell and dynamic pages loaded successfully.";return 0;
 }
 if(hasArgument(argc,argv,"--fullscreen")){
  if(auto *window=qobject_cast<QWindow *>(engine.rootObjects().constFirst()))window->showFullScreen();
  else qWarning()<<"Could not switch the shell window to full-screen mode.";
 }
 return app.exec();
}
