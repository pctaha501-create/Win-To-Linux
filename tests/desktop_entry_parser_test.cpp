#include "desktop_entry_parser.h"
#include <QCoreApplication>
#include <QDebug>
namespace { bool expect(bool ok,const char *message){if(!ok)qCritical().noquote()<<"FAIL:"<<message;return ok;} }
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);
 bool ok=true;
 const auto entry=DesktopEntryParser::parse(QStringLiteral(
  "# ignored comment\n[Desktop Entry]\nType=Application\nName=Demo App\nComment=A test app\nIcon=demo-icon\nExec=/usr/bin/demo --title \"Hello World\" %U\nTryExec=/usr/bin/demo\nPath=/tmp\nTerminal=false\nHidden=false\nNoDisplay=false\n[Desktop Action NewWindow]\nName=Ignored Action\nExec=ignored-command\n"));
 ok &= expect(entry.type==QStringLiteral("Application"),"parse application type");
 ok &= expect(entry.name==QStringLiteral("Demo App"),"parse desktop entry name");
 ok &= expect(entry.comment==QStringLiteral("A test app"),"parse desktop entry comment");
 ok &= expect(entry.exec.contains(QStringLiteral("/usr/bin/demo")),"parse desktop entry Exec");
 ok &= expect(entry.tryExec==QStringLiteral("/usr/bin/demo"),"parse TryExec availability guard");
 ok &= expect(entry.workingDirectory==QStringLiteral("/tmp"),"parse desktop entry working directory");
 ok &= expect(entry.icon==QStringLiteral("demo-icon"),"parse desktop entry icon");
 ok &= expect(!entry.hidden&&!entry.noDisplay&&!entry.terminal,"parse false visibility and terminal flags");
 QString program,error;QStringList args;
 ok &= expect(DesktopEntryParser::splitExec(entry.exec,&program,&args,&error),"split valid Exec command");
 ok &= expect(program==QStringLiteral("/usr/bin/demo"),"separate executable path");
 ok &= expect(args==QStringList{QStringLiteral("--title"),QStringLiteral("Hello World")},"preserve quoted argument and remove file field code");
 ok &= expect(DesktopEntryParser::splitExec(QStringLiteral("demo --name=%c --desktop=%k %% %i %f"),&program,&args,&error,QStringLiteral("Demo App"),QStringLiteral("demo-icon"),QStringLiteral("/tmp/demo.desktop")),"expand supported desktop-entry field codes");
 ok &= expect(args==QStringList{QStringLiteral("--name=Demo App"),QStringLiteral("--desktop=/tmp/demo.desktop"),QStringLiteral("%"),QStringLiteral("--icon"),QStringLiteral("demo-icon")},"expand name, desktop file, percent and icon fields safely");
 ok &= expect(!DesktopEntryParser::splitExec(QStringLiteral("demo %Z"),&program,&args,&error),"reject unknown desktop-entry field code");
 const auto missingTryExec=DesktopEntryParser::parse(QStringLiteral("[Desktop Entry]\nName=Missing\nExec=/usr/bin/demo\nTryExec=/missing/wtl-nonexistent-app\n"));
 ok &= expect(missingTryExec.tryExec==QStringLiteral("/missing/wtl-nonexistent-app"),"preserve missing TryExec for launch-time validation");
 const auto hidden=DesktopEntryParser::parse(QStringLiteral("[Desktop Entry]\nName=Hidden\nExec=/usr/bin/demo\nHidden=true\nNoDisplay=true\n"));
 ok &= expect(hidden.hidden&&hidden.noDisplay,"parse hidden and NoDisplay flags");
 const auto linkEntry=DesktopEntryParser::parse(QStringLiteral("[Desktop Entry]\nType=Link\nName=Web link\nURL=https://example.invalid\n"));
 ok &= expect(linkEntry.type==QStringLiteral("Link")&&linkEntry.exec.isEmpty(),"recognize non-application desktop entry types without inventing a command");
 const auto actionOnly=DesktopEntryParser::parse(QStringLiteral("[Desktop Action Test]\nName=Not an app\nExec=ignored\n"));
 ok &= expect(actionOnly.name.isEmpty()&&actionOnly.exec.isEmpty(),"ignore desktop action groups");
 ok &= expect(!DesktopEntryParser::splitExec(QString(),&program,&args,&error),"reject empty Exec");
 ok &= expect(!DesktopEntryParser::splitExec(QStringLiteral("-dangerous arg"),&program,&args,&error),"reject option-like executable");
 ok &= expect(!DesktopEntryParser::splitExec(QStringLiteral("demo%unsafe"),&program,&args,&error),"reject field code in executable");
 if(!ok)return 1;
 qInfo()<<"Desktop entry parser tests passed";return 0;
}
