#include "system_info.h"
#include "distro_detector.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QSysInfo>
#include <QSet>
#include <algorithm>
SystemInfo::SystemInfo(QObject *p):QObject(p){
 auto d=wtl::DistroDetector::detect();m_distribution=d.name.isEmpty()?QStringLiteral("Unknown Linux"):d.name;m_id=d.id.isEmpty()?QStringLiteral("unknown"):d.id;m_family=d.familyName;m_version=d.versionId.isEmpty()?QStringLiteral("Unknown"):d.versionId;m_manager=d.packageManager.isEmpty()?QStringLiteral("Not detected"):d.packageManager;m_managerAvailable=d.packageManagerAvailable;m_kernel=QSysInfo::kernelVersion();m_arch=QSysInfo::currentCpuArchitecture();m_host=QSysInfo::machineHostName();
 QSettings settings;m_lightAppearance=settings.value(QStringLiteral("appearance/light"),false).toBool();refreshApplications();
}
QString SystemInfo::distribution()const{return m_distribution;} QString SystemInfo::distroId()const{return m_id;} QString SystemInfo::family()const{return m_family;} QString SystemInfo::version()const{return m_version;}
QString SystemInfo::packageManager()const{return m_manager;} bool SystemInfo::managerAvailable()const{return m_managerAvailable;} QString SystemInfo::kernel()const{return m_kernel;} QString SystemInfo::architecture()const{return m_arch;} QString SystemInfo::hostName()const{return m_host;} QVariantList SystemInfo::applications()const{return m_apps;} QString SystemInfo::status()const{return m_status;} bool SystemInfo::lightAppearance()const{return m_lightAppearance;}
void SystemInfo::setLightAppearance(bool enabled){if(m_lightAppearance==enabled)return;m_lightAppearance=enabled;QSettings settings;settings.setValue(QStringLiteral("appearance/light"),enabled);emit appearanceChanged();}
void SystemInfo::setStatus(const QString&s){if(m_status==s)return;m_status=s;emit statusChanged();}
void SystemInfo::refreshApplications(){
 const QStringList roots={QDir::homePath()+QStringLiteral("/.local/share/applications"),QStringLiteral("/usr/local/share/applications"),QStringLiteral("/usr/share/applications")};QSet<QString> seen;QVariantList list;
 for(const auto&root:roots){QDir dir(root);for(const QFileInfo&fi:dir.entryInfoList({QStringLiteral("*.desktop")},QDir::Files|QDir::Readable,QDir::Name)){
  QString path=fi.canonicalFilePath();if(path.isEmpty())path=fi.absoluteFilePath();if(seen.contains(path))continue;seen.insert(path);
  QFile f(fi.absoluteFilePath());if(!f.open(QIODevice::ReadOnly|QIODevice::Text))continue;QString name,comment,exec;bool hidden=false,noDisplay=false,entry=false;
  while(!f.atEnd()){QString line=QString::fromUtf8(f.readLine()).trimmed();if(line.startsWith(QLatin1Char('['))){entry=line==QStringLiteral("[Desktop Entry]");continue;}if(!entry||line.isEmpty()||line.startsWith(QLatin1Char('#')))continue;int eq=line.indexOf(QLatin1Char('='));if(eq<1)continue;QString k=line.left(eq),v=line.mid(eq+1);if(k==QStringLiteral("Name"))name=v;else if(k==QStringLiteral("Comment"))comment=v;else if(k==QStringLiteral("Exec"))exec=v;else if(k==QStringLiteral("Hidden")&&v.compare(QStringLiteral("true"),Qt::CaseInsensitive)==0)hidden=true;else if(k==QStringLiteral("NoDisplay")&&v.compare(QStringLiteral("true"),Qt::CaseInsensitive)==0)noDisplay=true;}
  if(hidden||noDisplay||name.isEmpty()||exec.isEmpty())continue;
  list.append(QVariantMap{{QStringLiteral("name"),name},{QStringLiteral("comment"),comment},{QStringLiteral("desktopFile"),fi.absoluteFilePath()}});
 }}
 std::sort(list.begin(),list.end(),[](const QVariant&a,const QVariant&b){return a.toMap().value(QStringLiteral("name")).toString().toCaseFolded()<b.toMap().value(QStringLiteral("name")).toString().toCaseFolded();});
 if(list.size()>250)list=list.mid(0,250);m_apps=list;emit applicationsChanged();setStatus(QStringLiteral("Found %1 desktop entries.").arg(m_apps.size()));
}
bool SystemInfo::launchDesktopFile(const QString&desktopFile){
 QFileInfo fi(desktopFile);QString path=fi.absoluteFilePath();bool allowed=false;
 const QStringList roots={QDir::homePath()+QStringLiteral("/.local/share/applications"),QStringLiteral("/usr/local/share/applications"),QStringLiteral("/usr/share/applications")};
 for(const QString&root:roots)if(path.startsWith(QDir(root).absolutePath()+QLatin1Char('/'))){allowed=true;break;}
 if(!allowed||!fi.isFile()||fi.suffix()!=QStringLiteral("desktop")){setStatus(QStringLiteral("Launch blocked: invalid desktop entry path."));return false;}
 QFile f(path);if(!f.open(QIODevice::ReadOnly|QIODevice::Text)){setStatus(QStringLiteral("Could not read desktop entry."));return false;}
 QString exec;bool entry=false,hidden=false,terminal=false;
 while(!f.atEnd()){QString line=QString::fromUtf8(f.readLine()).trimmed();if(line.startsWith(QLatin1Char('['))){entry=line==QStringLiteral("[Desktop Entry]");continue;}if(!entry)continue;if(line.startsWith(QStringLiteral("Exec=")))exec=line.mid(5).trimmed();if(line.compare(QStringLiteral("Hidden=true"),Qt::CaseInsensitive)==0)hidden=true;if(line.compare(QStringLiteral("Terminal=true"),Qt::CaseInsensitive)==0)terminal=true;}
 if(hidden||terminal||exec.isEmpty()){setStatus(QStringLiteral("Entry hidden, terminal-based, or has no launch command."));return false;}
 QStringList args=QProcess::splitCommand(exec);if(args.isEmpty()){setStatus(QStringLiteral("Empty launch command."));return false;}QString program=args.takeFirst();
 args.erase(std::remove_if(args.begin(),args.end(),[](const QString&a){return a.startsWith(QLatin1Char('%'))||a.contains(QLatin1Char('%'));}),args.end());
 if(program.contains(QLatin1Char('%'))||program.startsWith(QLatin1Char('-'))){setStatus(QStringLiteral("Unsupported launch command."));return false;}
 if(QFileInfo(program).isAbsolute()){if(!QFileInfo(program).isExecutable()){setStatus(QStringLiteral("Executable is not available."));return false;}}
 else{QString resolved=QStandardPaths::findExecutable(program);if(resolved.isEmpty()){setStatus(QStringLiteral("Executable not found: %1").arg(program));return false;}program=resolved;}
 if(QProcess::startDetached(program,args,QDir::homePath())){setStatus(QStringLiteral("Started %1.").arg(QFileInfo(program).fileName()));return true;}setStatus(QStringLiteral("Failed to start application."));return false;
}
