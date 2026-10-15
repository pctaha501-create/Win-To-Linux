#include "system_info.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
int main(int argc,char**argv){
 QGuiApplication app(argc,argv);QCoreApplication::setApplicationName("Win-To-Linux");QCoreApplication::setApplicationVersion("0.2.0");QCoreApplication::setOrganizationName("Win-To-Linux");
 SystemInfo system;
 if(app.arguments().contains("--system")){qInfo().noquote()<<"Distribution:"<<system.distribution();qInfo().noquote()<<"ID:"<<system.distroId();qInfo().noquote()<<"Family:"<<system.family();qInfo().noquote()<<"Version:"<<system.version();qInfo().noquote()<<"Package manager:"<<system.packageManager();qInfo().noquote()<<"Manager available:"<<(system.managerAvailable()?"yes":"no");qInfo().noquote()<<"Kernel:"<<system.kernel();qInfo().noquote()<<"Architecture:"<<system.architecture();return 0;}
 QQmlApplicationEngine engine;engine.rootContext()->setContextProperty("systemInfo",&system);
 QObject::connect(&engine,&QQmlApplicationEngine::objectCreationFailed,&app,[](){QCoreApplication::exit(1);},Qt::QueuedConnection);
 engine.load(QUrl("qrc:/qml/Main.qml"));if(engine.rootObjects().isEmpty()){qCritical()<<"Unable to load Win-To-Linux shell.";return 1;}
 if(app.arguments().contains("--smoke-test")){qInfo()<<"QML shell loaded successfully.";return 0;}
 return app.exec();
}
