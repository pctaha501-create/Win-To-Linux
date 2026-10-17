#include "wine_prefix_controller.h"
#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTimer>
#include <QDebug>
#include <QElapsedTimer>
#include <QThread>
namespace {
bool expect(bool condition, const char *message) { if (!condition) qCritical().noquote() << "FAIL:" << message; return condition; }
bool writeExecutable(const QString &path, const QByteArray &content) {
 QFile file(path); if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
 if (file.write(content) != content.size()) return false; file.close();
 return file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup | QFileDevice::ExeGroup | QFileDevice::ReadOther | QFileDevice::ExeOther);
}
}
int main(int argc, char **argv) {
 QCoreApplication app(argc, argv); QTemporaryDir temporary;
 if (!expect(temporary.isValid(), "temporary test directory is available")) return 1;
 const QString root=temporary.filePath(QStringLiteral("managed-prefixes"));
 const QString wineboot=temporary.filePath(QStringLiteral("fake-wineboot"));
 const QString wine=temporary.filePath(QStringLiteral("fake-wine"));
 const QByteArray boot="#!/bin/sh\ntest -n \"$WINEPREFIX\" || exit 20\nmkdir -p \"$WINEPREFIX\" || exit 21\nprintf 'test registry\\n' > \"$WINEPREFIX/system.reg\" || exit 22\nexit 0\n";
 const QByteArray wineScript="#!/bin/sh\nprintf '%s\\n' \"$WINEPREFIX\" \"$@\" > \"$WTL_TEST_WINE_LOG\"\nexit 0\n";
 if (!expect(writeExecutable(wineboot,boot),"fake wineboot is executable") || !expect(writeExecutable(wine,wineScript),"fake wine is executable")) return 1;
 WinePrefixController controller(root,wineboot,wine);
 if (!expect(controller.validPrefixName(QStringLiteral("office-test_1.0")),"safe name accepted") || !expect(!controller.validPrefixName(QStringLiteral("../outside")),"path traversal rejected") || !expect(!controller.validPrefixName(QStringLiteral("with space")),"spaces rejected") || !expect(!controller.validPrefixName(QStringLiteral(".hidden")),"leading dot rejected") || !expect(!controller.validPrefixName(QString(41,QLatin1Char('a'))),"overlong name rejected")) return 1;
 if (!expect(controller.createPrefix(QStringLiteral("test-prefix")),"prefix initialization starts")) return 1;
 if (controller.busy()) { QEventLoop loop; QObject::connect(&controller,&WinePrefixController::busyChanged,&loop,[&](){if(!controller.busy())loop.quit();}); QTimer::singleShot(5000,&loop,&QEventLoop::quit); loop.exec(); }
 controller.refresh();
 if (!expect(!controller.busy(),"initialization finishes within timeout") || !expect(controller.prefixes().contains(QStringLiteral("test-prefix")),"initialized prefix discovered") || !expect(!controller.createPrefix(QStringLiteral("test-prefix")),"duplicate prefix rejected")) return 1;
 const QString installer=temporary.filePath(QStringLiteral("installer with spaces.msi"));
 QFile installerFile(installer); if(!installerFile.open(QIODevice::WriteOnly)||installerFile.write("fake msi")!=8)return 1; installerFile.close();
 const QString launchLog=temporary.filePath(QStringLiteral("wine-launch.log")); qputenv("WTL_TEST_WINE_LOG",launchLog.toUtf8());
 if(!expect(controller.launchWindowsFile(QStringLiteral("test-prefix"),installer),"MSI launch starts inside selected prefix"))return 1;
 QElapsedTimer launchTimer; launchTimer.start();
 while(!QFileInfo::exists(launchLog) && launchTimer.elapsed()<5000) { QCoreApplication::processEvents(); QThread::msleep(10); }
 QFile launchLogFile(launchLog); if(!expect(launchLogFile.open(QIODevice::ReadOnly),"fake Wine wrote launch arguments"))return 1;
 const QStringList launchLines=QString::fromUtf8(launchLogFile.readAll()).split(QLatin1Char('\n'),Qt::SkipEmptyParts); launchLogFile.close();
 const QString expectedPrefix=QFileInfo(QDir(root).filePath(QStringLiteral("test-prefix"))).canonicalFilePath();
 const QString expectedInstaller=QFileInfo(installer).canonicalFilePath();
 if(!expect(launchLines.size()==4,"MSI launch passes four distinct values") || !expect(launchLines.value(0)==expectedPrefix,"WINEPREFIX is scoped to the selected managed prefix") || !expect(launchLines.value(1)==QStringLiteral("msiexec"),"MSI is launched through msiexec") || !expect(launchLines.value(2)==QStringLiteral("/i"),"MSI install flag is passed separately") || !expect(launchLines.value(3)==expectedInstaller,"installer path with spaces remains one argument"))return 1;
 qunsetenv("WTL_TEST_WINE_LOG");
 const QString failingRoot=temporary.filePath(QStringLiteral("failed-prefixes"));
 const QString failingWineboot=temporary.filePath(QStringLiteral("fake-wineboot-failing"));
 if(!expect(writeExecutable(failingWineboot,QByteArray("#!/bin/sh\nmkdir -p \"$WINEPREFIX\"\necho 'wineboot fixture failure' >&2\nexit 23\n")),"failing wineboot fixture is executable"))return 1;
 WinePrefixController failingController(failingRoot,failingWineboot,wine);
 if(!expect(failingController.createPrefix(QStringLiteral("broken-prefix")),"failing prefix initialization starts"))return 1;
 if(failingController.busy()){QEventLoop loop;QObject::connect(&failingController,&WinePrefixController::busyChanged,&loop,[&](){if(!failingController.busy())loop.quit();});QTimer::singleShot(5000,&loop,&QEventLoop::quit);loop.exec();}
 if(!expect(!failingController.busy(),"failing wineboot operation finishes within timeout")||
    !expect(failingController.output().contains(QStringLiteral("wineboot fixture failure")),"wineboot stderr is captured for troubleshooting")||
    !expect(failingController.status().contains(QStringLiteral("cleaned up")),"failed prefix status explains cleanup")||
    !expect(!QFileInfo::exists(QDir(failingRoot).filePath(QStringLiteral("broken-prefix"))),"failed prefix directory is removed"))return 1;

 const QString timeoutRoot=temporary.filePath(QStringLiteral("timeout-prefixes"));
 const QString hangingWineboot=temporary.filePath(QStringLiteral("fake-wineboot-hanging"));
 if(!expect(writeExecutable(hangingWineboot,QByteArray("#!/bin/sh\nmkdir -p \"$WINEPREFIX\" || exit 31\ntrap '' TERM\nwhile :; do sleep 1; done\n")),"hanging wineboot fixture is executable"))return 1;
 WinePrefixController timeoutController(timeoutRoot,hangingWineboot,wine,nullptr,150);
 if(!expect(timeoutController.createPrefix(QStringLiteral("stuck-prefix")),"hanging prefix initialization starts"))return 1;
 QElapsedTimer timeoutTimer;timeoutTimer.start();
 while(timeoutController.busy()&&timeoutTimer.elapsed()<3000){QCoreApplication::processEvents();QThread::msleep(10);}
 if(!expect(!timeoutController.busy(),"stuck wineboot is terminated after the configured timeout")||
    !expect(timeoutController.status().contains(QStringLiteral("timed out")),"timeout reports a useful status")||
    !expect(!QFileInfo::exists(QDir(timeoutRoot).filePath(QStringLiteral("stuck-prefix"))),"timed-out incomplete prefix is cleaned up"))return 1;

 const QString outside=temporary.filePath(QStringLiteral("outside")); if(!expect(QDir().mkpath(outside),"outside fixture created"))return 1;
 QFile registry(QDir(outside).filePath(QStringLiteral("system.reg"))); if(!registry.open(QIODevice::WriteOnly)||registry.write("keep")!=4)return 1; registry.close();
 const QString linkPath=QDir(root).filePath(QStringLiteral("outside-link"));
 if(QFile::link(outside,linkPath)) { if(!expect(!controller.removePrefix(QStringLiteral("outside-link")),"symlinked prefix cannot be removed") || !expect(QFileInfo::exists(QDir(outside).filePath(QStringLiteral("system.reg"))),"outside target remains untouched"))return 1; }
 if(!expect(controller.removePrefix(QStringLiteral("test-prefix")),"managed prefix removed") || !expect(!QFileInfo::exists(QDir(root).filePath(QStringLiteral("test-prefix"))),"removed prefix no longer exists") || !expect(controller.prefixes().isEmpty(),"removed prefix disappears from listing"))return 1;
 qInfo()<<"Wine-prefix controller tests passed"; return 0;
}
