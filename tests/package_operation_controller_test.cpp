#include "package_operation_controller.h"
#include <QCoreApplication>
#include <QDebug>
namespace { bool expect(bool ok,const char *message){if(!ok)qCritical().noquote()<<"FAIL:"<<message;return ok;} }
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);
 PackageOperationController controller;
 if(!expect(!controller.running(),"controller starts idle"))return 1;
 if(!expect(!controller.installPackage(QStringLiteral("curl;touch /tmp/unexpected")),"reject shell metacharacters in install name"))return 1;
 if(!expect(!controller.running(),"invalid install never starts a process"))return 1;
 if(!expect(controller.status().contains(QStringLiteral("Invalid package name")),"install rejection explains validation failure"))return 1;
 if(!expect(!controller.removePackage(QStringLiteral("../important")),"reject path-like remove name"))return 1;
 if(!expect(!controller.running(),"invalid remove never starts a process"))return 1;
 if(!expect(controller.status().contains(QStringLiteral("Invalid package name")),"remove rejection explains validation failure"))return 1;
 qInfo()<<"Package operation controller validation tests passed";return 0;
}
