#include "system_info.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QClipboard>
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
 const QString apps=dataHome+QStringLiteral("/applications");const QString flatpakApps=dataHome+QStringLiteral("/flatpak/exports/share/applications");const QString customData=home+QStringLiteral("/custom-share");const QString customApps=customData+QStringLiteral("/applications");const QString work=home+QStringLiteral("/working-directory");
 const QString bin=home+QStringLiteral("/bin");const QString log=home+QStringLiteral("/launch.log");
 if(!QDir().mkpath(apps)||!QDir().mkpath(flatpakApps)||!QDir().mkpath(customApps)||!QDir().mkpath(work)||!QDir().mkpath(bin)){qCritical()<<"Could not create test directory tree.";return 1;}
 qputenv("HOME",home.toLocal8Bit());qputenv("XDG_DATA_HOME",dataHome.toLocal8Bit());qputenv("XDG_DATA_DIRS",customData.toLocal8Bit());qputenv("XDG_CONFIG_HOME",(home+QStringLiteral("/.config")).toLocal8Bit());
 const QByteArray oldPath=qgetenv("PATH");qputenv("PATH",QFile::encodeName(bin)+QByteArray(":")+oldPath);qputenv("WTL_LAUNCH_LOG",log.toLocal8Bit());
 QGuiApplication app(argc,argv);QCoreApplication::setApplicationName(QStringLiteral("Win-To-Linux-launch-test"));QCoreApplication::setOrganizationName(QStringLiteral("Win-To-Linux-tests"));
 QSettings persistentSettings;persistentSettings.clear();persistentSettings.sync();
 const QString fake=bin+QStringLiteral("/wtl-launch-fixture");
 const QByteArray fakeScript="#!/bin/sh\nprintf 'cwd=%s\n' \"$PWD\" > \"$WTL_LAUNCH_LOG\"\nfor arg in \"$@\"; do printf 'arg=<%s>\n' \"$arg\" >> \"$WTL_LAUNCH_LOG\"; done\n";
 if(!writeFile(fake,fakeScript,QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner)){qCritical()<<"Could not create fake executable.";return 1;}
 const QString fakeWine=bin+QStringLiteral("/wine");
 if(!writeFile(fakeWine,fakeScript,QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner)){qCritical()<<"Could not create fake Wine executable.";return 1;}
 const QString fakeTerminal=bin+QStringLiteral("/x-terminal-emulator");
 if(!writeFile(fakeTerminal,fakeScript,QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner)){qCritical()<<"Could not create fake terminal emulator.";return 1;}
 const QString desktop=apps+QStringLiteral("/fixture.desktop");
 const QByteArray desktopContents=QStringLiteral("[Desktop Entry]\nType=Application\nName=Fixture App\nIcon=fixture-icon\nExec=wtl-launch-fixture --name=%c --desktop=%k %i --literal=$HOME;touch %% %f\nTryExec=wtl-launch-fixture\nPath=%1\nTerminal=false\nHidden=false\nNoDisplay=false\n").arg(work).toUtf8();
 if(!writeFile(desktop,desktopContents)){qCritical()<<"Could not create fixture desktop entry.";return 1;}
 const QString flatpakDesktop=flatpakApps+QStringLiteral("/flatpak-fixture.desktop");
 const QByteArray flatpakContents=QStringLiteral("[Desktop Entry]\nType=Application\nName=Flatpak Fixture App\nExec=wtl-launch-fixture --flatpak\nTryExec=wtl-launch-fixture\nTerminal=false\nHidden=false\nNoDisplay=false\n").toUtf8();
 if(!writeFile(flatpakDesktop,flatpakContents)){qCritical()<<"Could not create Flatpak export fixture.";return 1;}
 const QString customDesktop=customApps+QStringLiteral("/custom-fixture.desktop");
 const QByteArray customContents=QStringLiteral("[Desktop Entry]\nType=Application\nName=Custom XDG Fixture App\nExec=wtl-launch-fixture --custom-xdg\nTryExec=wtl-launch-fixture\nTerminal=false\nHidden=false\nNoDisplay=false\n").toUtf8();
 if(!writeFile(customDesktop,customContents)){qCritical()<<"Could not create custom XDG application fixture.";return 1;}
 const QString nestedDirectory=customApps+QStringLiteral("/utilities");
 if(!QDir().mkpath(nestedDirectory)){qCritical()<<"Could not create nested XDG application directory.";return 1;}
 const QString nestedDesktop=nestedDirectory+QStringLiteral("/nested-fixture.desktop");
 if(!writeFile(nestedDesktop,QByteArray("[Desktop Entry]\nType=Application\nName=Nested XDG Fixture App\nExec=wtl-launch-fixture --nested-xdg\nTryExec=wtl-launch-fixture\n"))){qCritical()<<"Could not create nested XDG application fixture.";return 1;}
 const QString hiddenUserDesktop=apps+QStringLiteral("/masked-system-app.desktop");
 const QString hiddenSystemDesktop=customApps+QStringLiteral("/masked-system-app.desktop");
 if(!writeFile(hiddenUserDesktop,QByteArray("[Desktop Entry]\nName=Hidden User Override\nHidden=true\nExec=wtl-launch-fixture\n"))||
    !writeFile(hiddenSystemDesktop,QByteArray("[Desktop Entry]\nType=Application\nName=Visible System Fallback\nExec=wtl-launch-fixture\n"))){qCritical()<<"Could not create Hidden override fixtures.";return 1;}
 const QString visibleUserDesktop=apps+QStringLiteral("/visible-override.desktop");
 const QString visibleSystemDesktop=customApps+QStringLiteral("/visible-override.desktop");
 if(!writeFile(visibleUserDesktop,QByteArray("[Desktop Entry]\nType=Application\nName=User Override Wins\nExec=wtl-launch-fixture --user-override\n"))||
    !writeFile(visibleSystemDesktop,QByteArray("[Desktop Entry]\nName=System Duplicate Loses\nExec=wtl-launch-fixture --system-duplicate\n"))){qCritical()<<"Could not create duplicate desktop-entry fixtures.";return 1;}
 SystemInfo system;bool ok=true;
 const QVariantList discoveredApps=system.applications();bool foundFlatpak=false;for(const QVariant &application:discoveredApps)if(application.toMap().value(QStringLiteral("name")).toString()==QStringLiteral("Flatpak Fixture App"))foundFlatpak=true;
 ok &= expect(foundFlatpak,"discover per-user Flatpak exported desktop applications");
 bool foundCustomXdg=false;for(const QVariant &application:discoveredApps)if(application.toMap().value(QStringLiteral("name")).toString()==QStringLiteral("Custom XDG Fixture App"))foundCustomXdg=true;
 ok &= expect(foundCustomXdg,"discover applications from custom XDG_DATA_DIRS locations");
 bool foundNestedXdg=false;for(const QVariant &application:discoveredApps)if(application.toMap().value(QStringLiteral("name")).toString()==QStringLiteral("Nested XDG Fixture App"))foundNestedXdg=true;
 ok &= expect(foundNestedXdg,"discover desktop entries nested in XDG applications subdirectories");
 bool hiddenFallbackFound=false,userOverrideFound=false,systemDuplicateFound=false;
 for(const QVariant &application:discoveredApps){const QString name=application.toMap().value(QStringLiteral("name")).toString();if(name==QStringLiteral("Visible System Fallback"))hiddenFallbackFound=true;if(name==QStringLiteral("User Override Wins"))userOverrideFound=true;if(name==QStringLiteral("System Duplicate Loses"))systemDuplicateFound=true;}
 ok &= expect(!hiddenFallbackFound,"a higher-priority Hidden=true desktop entry masks the system fallback");
 ok &= expect(userOverrideFound,"a visible per-user desktop entry overrides a same-ID system entry");
 ok &= expect(!systemDuplicateFound,"duplicate lower-priority desktop entries are not shown twice");
 const QString linkDesktop=apps+QStringLiteral("/not-an-app.desktop");
 if(!writeFile(linkDesktop,QByteArray("[Desktop Entry]\nType=Link\nName=Not an app\nURL=https://example.invalid\nExec=wtl-launch-fixture --should-not-run\n"))){qCritical()<<"Could not create non-application desktop entry fixture.";return 1;}
 system.refreshApplications();
 bool linkWasListed=false;for(const QVariant &entry:system.applications())if(entry.toMap().value(QStringLiteral("desktopFile")).toString()==linkDesktop)linkWasListed=true;
 ok &= expect(!linkWasListed,"do not list Link entries as launchable applications");
 ok &= expect(!system.launchDesktopFile(linkDesktop),"refuse to launch a non-Application desktop entry");
 ok &= expect(system.launchDesktopFile(customDesktop),"launch a desktop entry from a configured XDG_DATA_DIRS root");
 QElapsedTimer customTimer;customTimer.start();while(!QFileInfo::exists(log)&&customTimer.elapsed()<2500)QThread::msleep(10);
 ok &= expect(readText(log).contains(QStringLiteral("arg=<--custom-xdg>")),"pass custom XDG application arguments safely");
 QFile::remove(log);
 ok &= expect(system.launchDesktopFile(flatpakDesktop),"launch an allowed per-user Flatpak exported desktop entry");
 QElapsedTimer flatpakTimer;flatpakTimer.start();while(!QFileInfo::exists(log)&&flatpakTimer.elapsed()<2500)QThread::msleep(10);
 ok &= expect(readText(log).contains(QStringLiteral("arg=<--flatpak>")),"launch Flatpak exported entry using safe argument handling");
 QFile::remove(log);
 ok &= expect(system.launchDesktopFile(nestedDesktop),"launch a desktop entry from a nested XDG applications subdirectory");
 QElapsedTimer nestedTimer;nestedTimer.start();while(!QFileInfo::exists(log)&&nestedTimer.elapsed()<2500)QThread::msleep(10);
 ok &= expect(readText(log).contains(QStringLiteral("arg=<--nested-xdg>")),"nested desktop launch passes its arguments safely");
 QFile::remove(log);
 QVariantList notificationFixture;
 for(int i=0;i<10;++i)notificationFixture.append(QVariantMap{{QStringLiteral("message"),QStringLiteral("notification-%1").arg(i)},{QStringLiteral("time"),QStringLiteral("12:%1").arg(i,2,10,QLatin1Char('0'))}});
 system.saveRecentNotifications(notificationFixture);
 const QVariantList savedNotifications=system.recentNotifications();
 ok &= expect(savedNotifications.size()==8,"persist at most eight recent notifications");
 ok &= expect(savedNotifications.first().toMap().value(QStringLiteral("message")).toString()==QStringLiteral("notification-0"),"preserve newest-first notification order");
 SystemInfo restartedSystem;
 ok &= expect(restartedSystem.recentNotifications()==savedNotifications,"restore recent notifications from persistent settings");
 const QString diagnostic=system.diagnosticReport();
 ok &= expect(diagnostic.contains(QStringLiteral("Runtime check:"))&&diagnostic.contains(QStringLiteral("Compositor executables:"))&&diagnostic.contains(QStringLiteral("Nested preview readiness:")),"diagnostic report includes runtime safety and compositor readiness");
 system.copyDiagnosticReport();
 ok &= expect(QGuiApplication::clipboard()->text()==diagnostic,"copy the full diagnostic report to the clipboard");
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
 const QByteArray missingContents=QStringLiteral("[Desktop Entry]\nType=Application\nName=Missing TryExec\nExec=wtl-launch-fixture\nTryExec=wtl-no-such-executable-for-test\nPath=%1\n").arg(work).toUtf8();
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
 QFile::remove(log);
 ok &= expect(system.launchTerminal(),"launch an installed terminal emulator from PATH");
 timer.restart();while(!QFileInfo::exists(log)&&timer.elapsed()<2500)QThread::msleep(10);
 const QString terminalOutput=readText(log);
 ok &= expect(terminalOutput.contains(QStringLiteral("cwd=%1\n").arg(QFileInfo(home).canonicalFilePath())),"launch terminal in the current user home directory");
 ok &= expect(system.status().contains(QStringLiteral("Launching terminal")),"report terminal launch status");
 QTemporaryDir emptyPath;
 ok &= expect(emptyPath.isValid(),"create empty PATH for missing-terminal test");
 const QByteArray savedPath=qgetenv("PATH");qputenv("PATH",QFile::encodeName(emptyPath.path()));
 ok &= expect(!system.launchTerminal(),"refuse terminal launch when no emulator exists on PATH");
 ok &= expect(system.status().contains(QStringLiteral("No terminal emulator was found")),"explain missing terminal emulator without installing packages");
 qputenv("PATH",savedPath);
 if(!ok)return 1;
 qunsetenv("WAYLAND_DISPLAY");qunsetenv("DISPLAY");qunsetenv("XDG_RUNTIME_DIR");
 SystemInfo headlessSystem;
 ok &= expect(!headlessSystem.launchNestedPreview(),"refuse to launch isolated preview without an active Wayland session");
 ok &= expect(!headlessSystem.previewRunning(),"headless preview refusal must not start a child process");
 ok &= expect(headlessSystem.status().contains(QStringLiteral("Not ready for nested preview")),"preview refusal explains readiness blockers");
 if(!ok)return 1;
 qInfo()<<"Desktop-entry launch and nested-preview safety checks passed.";return 0;
}
