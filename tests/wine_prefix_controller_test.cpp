#include "wine_prefix_controller.h"
#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTimer>
#include <QDebug>
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
 const QByteArray wineScript="#!/bin/sh\nexit 0\n";
 if (!expect(writeExecutable(wineboot,boot),"fake wineboot is executable") || !expect(writeExecutable(wine,wineScript),"fake wine is executable")) return 1;
 WinePrefixController controller(root,wineboot,wine);
 if (!expect(controller.validPrefixName(QStringLiteral("office-test_1.0")),"safe name accepted") || !expect(!controller.validPrefixName(QStringLiteral("../outside")),"path traversal rejected") || !expect(!controller.validPrefixName(QStringLiteral("with space")),"spaces rejected") || !expect(!controller.validPrefixName(QStringLiteral(".hidden")),"leading dot rejected") || !expect(!controller.validPrefixName(QString(41,QLatin1Char('a'))),"overlong name rejected")) return 1;
 if (!expect(controller.createPrefix(QStringLiteral("test-prefix")),"prefix initialization starts")) return 1;
 if (controller.busy()) { QEventLoop loop; QObject::connect(&controller,&WinePrefixController::busyChanged,&loop,[&](){if(!controller.busy())loop.quit();}); QTimer::singleShot(5000,&loop,&QEventLoop::quit); loop.exec(); }
 controller.refresh();
 if (!expect(!controller.busy(),"initialization finishes within timeout") || !expect(controller.prefixes().contains(QStringLiteral("test-prefix")),"initialized prefix discovered") || !expect(!controller.createPrefix(QStringLiteral("test-prefix")),"duplicate prefix rejected")) return 1;
 const QString outside=temporary.filePath(QStringLiteral("outside")); if(!expect(QDir().mkpath(outside),"outside fixture created"))return 1;
 QFile registry(QDir(outside).filePath(QStringLiteral("system.reg"))); if(!registry.open(QIODevice::WriteOnly)||registry.write("keep")!=4)return 1; registry.close();
 const QString linkPath=QDir(root).filePath(QStringLiteral("outside-link"));
 if(QFile::link(outside,linkPath)) { if(!expect(!controller.removePrefix(QStringLiteral("outside-link")),"symlinked prefix cannot be removed") || !expect(QFileInfo::exists(QDir(outside).filePath(QStringLiteral("system.reg"))),"outside target remains untouched"))return 1; }
 if(!expect(controller.removePrefix(QStringLiteral("test-prefix")),"managed prefix removed") || !expect(!QFileInfo::exists(QDir(root).filePath(QStringLiteral("test-prefix"))),"removed prefix no longer exists") || !expect(controller.prefixes().isEmpty(),"removed prefix disappears from listing"))return 1;
 qInfo()<<"Wine-prefix controller tests passed"; return 0;
}
