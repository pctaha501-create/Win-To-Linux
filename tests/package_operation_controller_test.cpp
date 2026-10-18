#include "package_operation_controller.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QEventLoop>
#include <QTimer>
#include <QThread>
#include <QFileInfo>
#include <QElapsedTimer>
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
 QTemporaryDir emptyPath;
 if(!expect(emptyPath.isValid(),"create isolated empty PATH"))return 1;
 const QByteArray originalPath=qgetenv("PATH");qputenv("PATH",emptyPath.path().toLocal8Bit());
 const bool started=controller.installPackage(QStringLiteral("curl"));const QString missingHelperStatus=controller.status();qputenv("PATH",originalPath);
 if(!expect(!started,"do not start package operation without pkexec"))return 1;
 if(!expect(missingHelperStatus.contains(QStringLiteral("pkexec")),"missing PolicyKit helper has a specific actionable message"))return 1;
 if(!expect(!controller.running(),"missing helper leaves controller idle"))return 1;
 // Exercise a real QProcess completion path without running a privileged command.
 QTemporaryDir fakeBin;
 if(!expect(fakeBin.isValid(),"create isolated fake PolicyKit directory"))return 1;
 const QString fakePkexec=QDir(fakeBin.path()).filePath(QStringLiteral("pkexec"));
 const QString commandLog=QDir(fakeBin.path()).filePath(QStringLiteral("command.log"));
 QFile fakeHelper(fakePkexec);
 const QByteArray helperScript=QByteArray("#!/bin/sh\nprintf 'fake package operation output\\n'\n: > \"$WTL_FAKE_PKEXEC_LOG\"\nfor arg in \"$@\"; do printf '<%s>\\n' \"$arg\" >> \"$WTL_FAKE_PKEXEC_LOG\"; done\nexit 0\n");
 if(!expect(fakeHelper.open(QIODevice::WriteOnly|QIODevice::Truncate),"create fake PolicyKit helper"))return 1;
 fakeHelper.write(helperScript);fakeHelper.close();
 if(!expect(fakeHelper.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner|QFileDevice::ReadGroup|QFileDevice::ExeGroup|QFileDevice::ReadOther|QFileDevice::ExeOther),"make fake PolicyKit helper executable"))return 1;
 wtl::DistroInfo fakeDistro;fakeDistro.id=QStringLiteral("test");fakeDistro.family=wtl::DistroFamily::Debian;fakeDistro.packageManager=QStringLiteral("apt-get");fakeDistro.packageManagerPath=QStringLiteral("/fake/apt-get");fakeDistro.packageManagerAvailable=true;
 PackageOperationController successfulController(fakeDistro);
 const QByteArray savedPath=qgetenv("PATH");const QByteArray savedLog=qgetenv("WTL_FAKE_PKEXEC_LOG");
 qputenv("PATH",fakeBin.path().toLocal8Bit());qputenv("WTL_FAKE_PKEXEC_LOG",commandLog.toLocal8Bit());
 const bool operationStarted=successfulController.installPackage(QStringLiteral("curl"));
 if(!expect(operationStarted,"valid package operation starts through the fake PolicyKit helper")){qputenv("PATH",savedPath);qputenv("WTL_FAKE_PKEXEC_LOG",savedLog);return 1;}
 QElapsedTimer operationTimer;operationTimer.start();
 while(successfulController.running()&&operationTimer.elapsed()<4000){QCoreApplication::processEvents();QThread::msleep(10);}
 qputenv("PATH",savedPath);if(savedLog.isNull())qunsetenv("WTL_FAKE_PKEXEC_LOG");else qputenv("WTL_FAKE_PKEXEC_LOG",savedLog);
 if(!expect(!successfulController.running(),"fake package operation reaches a terminal state")||
    !expect(successfulController.status().contains(QStringLiteral("completed successfully")),"successful package operation reports success")||
    !expect(successfulController.output().contains(QStringLiteral("fake package operation output")),"package manager output is captured"))return 1;
 QFile loggedArguments(commandLog);if(!expect(loggedArguments.open(QIODevice::ReadOnly),"fake PolicyKit helper records arguments"))return 1;
 const QByteArray arguments=loggedArguments.readAll();
 if(!expect(arguments.contains("</fake/apt-get>\n")&&arguments.contains("<install>\n")&&arguments.contains("<-y>\n")&&arguments.contains("<curl>\n"),"package command reaches PolicyKit as discrete arguments without shell interpretation"))return 1;
 qInfo()<<"Package operation controller validation tests passed";return 0;
}
