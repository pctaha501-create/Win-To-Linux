#include "system_info.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QThread>
#include <QSettings>
#include <QDebug>
#include <QTextStream>
#include <functional>

namespace {
bool expect(bool condition,const char *message){if(!condition)qCritical()<<"FAIL:"<<message;return condition;}
bool writeFile(const QString &path,const QByteArray &contents,QFile::Permissions permissions=QFile::ReadOwner|QFile::WriteOwner){
 QFile file(path);if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate))return false;
 if(file.write(contents)!=contents.size())return false;
 file.close();return file.setPermissions(permissions);
}
QString readText(const QString &path){QFile file(path);if(!file.open(QIODevice::ReadOnly|QIODevice::Text))return {};return QString::fromUtf8(file.readAll());}
}
int main(int argc,char **argv){
 QTemporaryDir temp;if(!temp.isValid()){qCritical()<<"Could not create temporary test directory.";return 1;}
 const QString home=temp.path();const QString dataHome=home+QStringLiteral("/.local/share");
 const QString apps=dataHome+QStringLiteral("/applications");const QString work=home+QStringLiteral("/working-directory");
 const QString bin=home+QStringLiteral("/bin");const QString log=home+QStringLiteral("/launch.log");
 if(!QDir().mkpath(apps)||!QDir().mkpath(work)||!QDir().mkpath(bin)){qCritical()<<"Could not create test directory tree.";return 1;}
 qputenv("HOME",home.toLocal8Bit());qputenv("XDG_DATA_HOME",dataHome.toLocal8Bit());qputenv("XDG_CONFIG_HOME",(home+QStringLiteral("/.config")).toLocal8Bit());
 const QByteArray oldPath=qgetenv("PATH");qputenv("PATH",QFile::encodeName(bin)+QByteArray(":")+oldPath);qputenv("WTL_LAUNCH_LOG",log.toLocal8Bit());
 QGuiApplication app(argc,argv);QCoreApplication::setApplicationName(QStringLiteral("Win-To-Linux-launch-test"));QCoreApplication::setOrganizationName(QStringLiteral("Win-To-Linux-tests"));
 const QString fake=bin+QStringLiteral("/wtl-launch-fixture");
 const QByteArray fakeScript="#!/bin/sh\nprintf 'cwd=%s\n' \"$PWD\" > \"$WTL_LAUNCH_LOG\"\nfor arg in \"$@\"; do printf 'arg=<%s>\n' \"$arg\" >> \"$WTL_LAUNCH_LOG\"; done\n";
 if(!writeFile(fake,fakeScript,QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner)){qCritical()<<"Could not create fake executable.";return 1;}
 const QString fakeWine=bin+QStringLiteral("/wine");
 if(!writeFile(fakeWine,fakeScript,QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner)){qCritical()<<"Could not create fake Wine executable.";return 1;}
 const QString desktop=apps+QStringLiteral("/fixture.desktop");
 const QByteArray desktopContents=QStringLiteral("[Desktop Entry]\nName=Fixture App\nIcon=fixture-icon\nExec=wtl-launch-fixture --name=%c --desktop=%k %i --literal=$HOME;touch %% %f\nTryExec=wtl-launch-fixture\nPath=%1\nTerminal=false\nHidden=false\nNoDisplay=false\n").arg(work).toUtf8();
 if(!writeFile(desktop,desktopContents)){qCritical()<<"Could not create fixture desktop entry.";return 1;}
 SystemInfo system;bool ok=true;
 ok &= expect(system.launchDesktopFile(desktop),"launch a valid desktop entry with fake executable");
 QElapsedTimer timer;timer.start();while(!QFileInfo::exists(log)&&timer.elapsed()<2500)QThread::msleep(10);
 ok &= expect(QFileInfo::exists(log),"fake desktop executable was started");
 const QString output=readText(log);
 ok &= expect(output.contains(QStringLiteral("cwd=%1\n").arg(QFileInfo(work).canonicalFilePath())),"honor desktop-entry working directory");
 ok &= expect(output.contains(QStringLiteral("arg=<--name=Fixture App>")),"expand application name without splitting the argument");
 ok &= expect(output.contains(QStringLiteral("arg=<--desktop=%1>").arg(QFileInfo(desktop).canonicalFilePath())),"expand canonical desktop file path");
 ok &= expect(output.contains(QStringLiteral("arg=<--icon>\narg=<fixture-icon>")),"expand icon field into two arguments");
 ok &= expect(output.contains(QStringLiteral("arg=<--literal=$HOME;touch>")),"pass shell metacharacters literally without shell evaluation");
 ok &= expect(output.contains(QStringLiteral("arg=<%>")),"expand literal percent field code");
 ok &= expect(!output.contains(QStringLiteral("arg=<%f>")),"omit file placeholder when no activation target is provided");
 const QString outside=home+QStringLiteral("/outside.desktop");
 ok &= expect(writeFile(outside,desktopContents),"create outside-path fixture");
 ok &= expect(!system.launchDesktopFile(outside),"reject a desktop entry outside allowed XDG application roots");
 ok &= expect(system.status().contains(QStringLiteral("Launch blocked")),"report blocked outside desktop-entry path");
 const QString missing=apps+QStringLiteral("/missing.desktop");
 const QByteArray missingContents=QStringLiteral("[Desktop Entry]\nName=Missing TryExec\nExec=wtl-launch-fixture\nTryExec=wtl-no-such-executable-for-test\nPath=%1\n").arg(work).toUtf8();
 ok &= expect(writeFile(missing,missingContents),"create missing-TryExec fixture");
 ok &= expect(!system.launchDesktopFile(missing),"reject a desktop entry with unavailable TryExec");
 ok &= expect(system.status().contains(QStringLiteral("TryExec")),"report missing TryExec reason");
 const QString exePath=home+QStringLiteral("/sample.exe");const QString msiPath=home+QStringLiteral("/sample.msi");const QString invalidPath=home+QStringLiteral("/sample.txt");
 ok &= expect(writeFile(exePath,QByteArray("MZ")),"create fake Windows executable file");
 ok &= expect(writeFile(msiPath,QByteArray("MSI")),"create fake Windows installer file");
 ok &= expect(writeFile(invalidPath,QByteArray("text")),"create invalid Windows file fixture");
 ok &= expect(system.wineAvailable(),"discover fake Wine executable from PATH");
 QFile::remove(log);
 ok &= expect(system.launchWindowsFile(exePath),"launch an existing .exe through fake Wine");
 timer.restart();while(!QFileInfo::exists(log)&&timer.elapsed()<2500)QThread::msleep(10);
 QString wineOutput=readText(log);
 ok &= expect(wineOutput.contains(QStringLiteral("cwd=%1\n").arg(QFileInfo(home).canonicalFilePath())),"launch Wine with the current user's home as working directory");
 ok &= expect(wineOutput.contains(QStringLiteral("arg=<%1>").arg(QFileInfo(exePath).canonicalFilePath())),"pass canonical .exe path as a single Wine argument");
 QFile::remove(log);
 ok &= expect(system.launchWindowsFile(msiPath),"launch an existing .msi through fake Wine");
 timer.restart();while(!QFileInfo::exists(log)&&timer.elapsed()<2500)QThread::msleep(10);
 wineOutput=readText(log);
 ok &= expect(wineOutput.contains(QStringLiteral("arg=<msiexec>\narg=</i>\narg=<%1>").arg(QFileInfo(msiPath).canonicalFilePath())),"launch MSI through wine msiexec /i with a canonical path");
 ok &= expect(!system.launchWindowsFile(invalidPath),"reject non-EXE/MSI files before invoking Wine");
 if(!ok)return 1;
 qInfo()<<"Desktop-entry launch integration checks passed.";return 0;
}
