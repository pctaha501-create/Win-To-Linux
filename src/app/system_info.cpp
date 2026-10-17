#include "system_info.h"
#include "distro_detector.h"
#include "desktop_entry_parser.h"
#include "session_backend.h"
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QGuiApplication>
#include <QClipboard>
#include <QStringList>
#include <QVariantMap>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSettings>
#include <QStandardPaths>
#include <QSysInfo>
#include <QSet>
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
namespace {
QStringList applicationRoots(){
 const QString dataHome=QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
 return {QDir(dataHome).filePath(QStringLiteral("applications")),QDir(dataHome).filePath(QStringLiteral("flatpak/exports/share/applications")),QStringLiteral("/var/lib/flatpak/exports/share/applications"),QStringLiteral("/var/lib/snapd/desktop/applications"),QStringLiteral("/usr/local/share/applications"),QStringLiteral("/usr/share/applications")};
}
bool detectQtWaylandPlugin(){
 QStringList roots=QCoreApplication::libraryPaths();roots.append(QLibraryInfo::path(QLibraryInfo::PluginsPath));
 const QStringList names{QStringLiteral("libqwayland-generic.so"),QStringLiteral("libqwayland-egl.so")};
 for(const QString &root:roots){const QDir platforms(QDir(root).filePath(QStringLiteral("platforms")));for(const QString &name:names)if(QFileInfo(platforms.filePath(name)).isFile())return true;}
 return false;
}
}
SystemInfo::SystemInfo(QObject *p):QObject(p){
 auto d=wtl::DistroDetector::detect();m_distribution=d.name.isEmpty()?QStringLiteral("Unknown Linux"):d.name;m_id=d.id.isEmpty()?QStringLiteral("unknown"):d.id;m_family=d.familyName;m_version=d.versionId.isEmpty()?QStringLiteral("Unknown"):d.versionId;m_manager=d.packageManager.isEmpty()?QStringLiteral("Not detected"):d.packageManager;m_managerAvailable=d.packageManagerAvailable;m_kernel=QSysInfo::kernelVersion();m_arch=QSysInfo::currentCpuArchitecture();m_host=QSysInfo::machineHostName();
 QSettings settings;m_lightAppearance=settings.value(QStringLiteral("appearance/light"),false).toBool();m_winePath=QStandardPaths::findExecutable(QStringLiteral("wine"));m_bottlesPath=QStandardPaths::findExecutable(QStringLiteral("bottles"));m_qtWaylandPluginAvailable=detectQtWaylandPlugin();discoverNativeTools();discoverSession();refreshApplications();
 m_previewProcess.setProcessChannelMode(QProcess::MergedChannels);
 connect(&m_previewProcess,&QProcess::started,this,[this](){emit previewStateChanged();setStatus(QStringLiteral("Nested preview launcher started. Close the preview window to return to this desktop."));});
 connect(&m_previewProcess,&QProcess::readyReadStandardOutput,this,[this](){m_previewOutput+=QString::fromLocal8Bit(m_previewProcess.readAllStandardOutput());if(m_previewOutput.size()>8192)m_previewOutput=m_previewOutput.right(8192);});
 connect(&m_previewProcess,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus exitStatus){emit previewStateChanged();if(exitStatus==QProcess::NormalExit&&code==0)setStatus(QStringLiteral("Nested preview closed."));else{QString detail=m_previewOutput.trimmed();if(detail.size()>240)detail=detail.right(240);setStatus(QStringLiteral("Nested preview exited (code %1)%2").arg(code).arg(detail.isEmpty()?QString():QStringLiteral(": ")+detail));}});
 connect(&m_previewProcess,&QProcess::errorOccurred,this,[this](QProcess::ProcessError error){if(error==QProcess::FailedToStart)setStatus(QStringLiteral("Could not start nested preview: %1").arg(m_previewProcess.errorString()));emit previewStateChanged();});
}
QString SystemInfo::distribution()const{return m_distribution;} QString SystemInfo::distroId()const{return m_id;} QString SystemInfo::family()const{return m_family;} QString SystemInfo::version()const{return m_version;}
QString SystemInfo::packageManager()const{return m_manager;} bool SystemInfo::managerAvailable()const{return m_managerAvailable;} QString SystemInfo::kernel()const{return m_kernel;} QString SystemInfo::architecture()const{return m_arch;} QString SystemInfo::hostName()const{return m_host;} QVariantList SystemInfo::applications()const{return m_apps;} QVariantList SystemInfo::nativeTools()const{return m_nativeTools;} QString SystemInfo::status()const{return m_status;} bool SystemInfo::previewRunning()const{return m_previewProcess.state()!=QProcess::NotRunning;}
QString SystemInfo::sessionType()const{return m_sessionType;} QString SystemInfo::desktopEnvironment()const{return m_desktopEnvironment;} QString SystemInfo::waylandDisplay()const{return m_waylandDisplay;} QString SystemInfo::xDisplay()const{return m_xDisplay;} QString SystemInfo::runtimeDirectory()const{return m_runtimeDirectory;} QString SystemInfo::runtimeDirectoryStatus()const{return m_runtimeDirectoryStatus;} QVariantList SystemInfo::availableCompositors()const{return m_availableCompositors;} bool SystemInfo::qtWaylandPluginAvailable()const{return m_qtWaylandPluginAvailable;}
QString SystemInfo::previewReadiness()const{
 SessionEnvironment environment;
 environment.runtimeDirectory=m_runtimeDirectory==QStringLiteral("Not set")?QString():m_runtimeDirectory;
 environment.runtimeDirectoryPrivate=m_runtimeDirectory!=QStringLiteral("Not set")&&m_runtimeDirectoryStatus==QStringLiteral("Directory exists and ownership/permissions look safe");
 if(m_waylandDisplay!=QStringLiteral("Not set")&&!m_waylandDisplay.isEmpty()){
  environment.waylandSocketPath=m_waylandDisplay.startsWith(QLatin1Char('/'))?m_waylandDisplay:QDir(m_runtimeDirectory).filePath(m_waylandDisplay);
  struct stat socketInfo{};
  const QByteArray encodedSocket=QFile::encodeName(environment.waylandSocketPath);
  environment.waylandSocketVerified=::lstat(encodedSocket.constData(),&socketInfo)==0&&S_ISSOCK(socketInfo.st_mode)&&socketInfo.st_uid==getuid();
 }
 environment.qtWaylandPluginAvailable=m_qtWaylandPluginAvailable;
 environment.westonAvailable=!QStandardPaths::findExecutable(QStringLiteral("weston")).isEmpty();
 environment.cageAvailable=!QStandardPaths::findExecutable(QStringLiteral("cage")).isEmpty();
 const QString appPath=QCoreApplication::applicationFilePath();
 environment.applicationExecutableVerified=!appPath.isEmpty()&&QFileInfo(appPath).isExecutable();
 const auto backend=createSessionBackend(QStringLiteral("weston-nested"));
 if(!backend)return QStringLiteral("Not ready for nested preview:\n• No supported session backend was selected.");
 return backend->probe(environment).summary;
}
bool SystemInfo::launchNestedPreview(){
 if(previewRunning()){setStatus(QStringLiteral("Nested preview is already running."));return false;}
 const QString readiness=previewReadiness();
 if(readiness.startsWith(QStringLiteral("Not ready"))){setStatus(readiness);return false;}
 const QString appDir=QCoreApplication::applicationDirPath();
 const QStringList candidates{
  QDir(appDir).filePath(QStringLiteral("win-to-linux-isolated-preview")),
  QDir(appDir).filePath(QStringLiteral("../lib/win-to-linux/start-isolated-preview.sh")),
  QDir(appDir).filePath(QStringLiteral("../../../scripts/start-isolated-preview.sh")),
  QStandardPaths::findExecutable(QStringLiteral("win-to-linux-isolated-preview"))
 };
 QString launcher;
 for(const QString &candidate:candidates){if(candidate.isEmpty())continue;const QFileInfo info(candidate);if(info.isFile()&&info.isExecutable()){launcher=info.canonicalFilePath();break;}}
 if(launcher.isEmpty()){setStatus(QStringLiteral("The isolated-preview launcher was not found. Reinstall Win-To-Linux or build it from the repository."));return false;}
 const QString appPath=QCoreApplication::applicationFilePath();
 if(appPath.isEmpty()||!QFileInfo(appPath).isExecutable()){setStatus(QStringLiteral("The Win-To-Linux executable path could not be verified."));return false;}
 QProcessEnvironment environment=QProcessEnvironment::systemEnvironment();environment.insert(QStringLiteral("WTL_BIN"),QFileInfo(appPath).canonicalFilePath());
 m_previewOutput.clear();m_previewProcess.setProcessEnvironment(environment);m_previewProcess.setWorkingDirectory(QDir::homePath());m_previewProcess.setProgram(launcher);m_previewProcess.setArguments({});m_previewProcess.start();
 if(!m_previewProcess.waitForStarted(2000)){const QString reason=m_previewProcess.errorString();setStatus(QStringLiteral("Could not start nested preview: %1").arg(reason));emit previewStateChanged();return false;}
 emit previewStateChanged();setStatus(QStringLiteral("Nested preview is starting. Your current desktop and login manager will not be changed."));return true;
}
QString SystemInfo::standaloneReadiness()const{
 SessionEnvironment environment;
#ifdef Q_OS_LINUX
 environment.linuxHost=true;
#endif
 environment.explicitOptIn=qEnvironmentVariable("WTL_ENABLE_DRM_SESSION")==QStringLiteral("1");
 environment.x11DisplaySet=!qEnvironmentVariable("DISPLAY").isEmpty();
 environment.waylandDisplaySet=!qEnvironmentVariable("WAYLAND_DISPLAY").isEmpty();
 const char *terminal=::ttyname(STDIN_FILENO);
 environment.ttyPath=terminal?QString::fromLocal8Bit(terminal):QString();
 environment.realVirtualTerminal=::isatty(STDIN_FILENO)&&environment.ttyPath.startsWith(QStringLiteral("/dev/tty"))&&!environment.ttyPath.startsWith(QStringLiteral("/dev/pts/"));
 const QByteArray runtimeBytes=qgetenv("XDG_RUNTIME_DIR");
 environment.runtimeDirectory=QString::fromLocal8Bit(runtimeBytes);
 struct stat runtimeInfo{};
 environment.runtimeDirectoryPrivate=!runtimeBytes.isEmpty()&&::stat(runtimeBytes.constData(),&runtimeInfo)==0&&S_ISDIR(runtimeInfo.st_mode)&&runtimeInfo.st_uid==getuid()&&(runtimeInfo.st_mode&0077)==0;
 environment.westonAvailable=!QStandardPaths::findExecutable(QStringLiteral("weston")).isEmpty();
 environment.qtWaylandPluginAvailable=m_qtWaylandPluginAvailable;
 const QString appPath=QCoreApplication::applicationFilePath();
 environment.applicationExecutableVerified=!appPath.isEmpty()&&QFileInfo(appPath).isExecutable();
 QDir drmDirectory(QStringLiteral("/dev/dri"));
 const QFileInfoList cards=drmDirectory.entryInfoList(QStringList{QStringLiteral("card*")},QDir::System|QDir::Files|QDir::Readable);
 for(const QFileInfo &card:cards){const QByteArray path=QFile::encodeName(card.absoluteFilePath());if(::access(path.constData(),R_OK|W_OK)==0){environment.drmDeviceAccessible=true;break;}}
 const auto backend=createSessionBackend(QStringLiteral("weston-drm"));
 if(!backend)return QStringLiteral("Not ready for standalone session:\n• No supported session backend was selected.");
 return backend->probe(environment).summary;
}
QString SystemInfo::diagnosticReport()const{
 const QString report=QStringLiteral("Win-To-Linux diagnostic report\nDistribution: %1 (%2)\nVersion: %3\nFamily: %4\nKernel: %5\nArchitecture: %6\nHost: %7\nDesktop environment: %8\nSession type: %9\nWayland display: %10\nX11 display: %11\nRuntime directory: %12\nRuntime check: %13\nPackage manager: %14\nPackage manager available: %15\nWine available: %16\nBottles available: %17\nDetected compositors: %18")
  .arg(m_distribution).arg(m_id).arg(m_version).arg(m_family).arg(m_kernel).arg(m_arch).arg(m_host).arg(m_desktopEnvironment).arg(m_sessionType)
  .arg(m_waylandDisplay).arg(m_xDisplay).arg(m_runtimeDirectory).arg(m_runtimeDirectoryStatus).arg(m_manager)
  .arg(m_managerAvailable?QStringLiteral("yes"):QStringLiteral("no"))
  .arg(wineAvailable()?QStringLiteral("yes"):QStringLiteral("no"))
  .arg(bottlesAvailable()?QStringLiteral("yes"):QStringLiteral("no"))
  .arg(QString::number(m_availableCompositors.size()));
 QStringList compositorDetails;
 for(const QVariant &entry:m_availableCompositors){const QVariantMap item=entry.toMap();compositorDetails.append(QStringLiteral("%1: %2").arg(item.value(QStringLiteral("name")).toString(),item.value(QStringLiteral("path")).toString()));}
 return report + QStringLiteral("\nQt Wayland platform plugin: %1\nCompositor executables:\n%2\nNested preview readiness:\n%3")
  .arg(m_qtWaylandPluginAvailable?QStringLiteral("available"):QStringLiteral("missing"))
  .arg(compositorDetails.isEmpty()?QStringLiteral("None detected"):compositorDetails.join(QLatin1Char('\n')),previewReadiness());
}
void SystemInfo::copyDiagnosticReport()const{QGuiApplication::clipboard()->setText(diagnosticReport());}
QVariantList SystemInfo::recentNotifications()const{QSettings settings;return settings.value(QStringLiteral("notifications/recent")).toList();}
void SystemInfo::saveRecentNotifications(const QVariantList &notifications){QSettings settings;settings.setValue(QStringLiteral("notifications/recent"),notifications.mid(0,8));}
bool SystemInfo::lightAppearance()const{return m_lightAppearance;}
void SystemInfo::setLightAppearance(bool enabled){if(m_lightAppearance==enabled)return;m_lightAppearance=enabled;QSettings settings;settings.setValue(QStringLiteral("appearance/light"),enabled);emit appearanceChanged();}
bool SystemInfo::wineAvailable()const{return !m_winePath.isEmpty();} QString SystemInfo::winePath()const{return m_winePath;} bool SystemInfo::bottlesAvailable()const{return !m_bottlesPath.isEmpty();} QString SystemInfo::bottlesPath()const{return m_bottlesPath;}
void SystemInfo::setStatus(const QString&s){if(m_status==s)return;m_status=s;emit statusChanged();}
void SystemInfo::discoverSession(){
 const QProcessEnvironment env=QProcessEnvironment::systemEnvironment();
 m_sessionType=env.value(QStringLiteral("XDG_SESSION_TYPE")).trimmed().toLower();
 m_waylandDisplay=env.value(QStringLiteral("WAYLAND_DISPLAY")).trimmed();
 m_xDisplay=env.value(QStringLiteral("DISPLAY")).trimmed();
 m_desktopEnvironment=env.value(QStringLiteral("XDG_CURRENT_DESKTOP")).trimmed();
 if(m_desktopEnvironment.isEmpty())m_desktopEnvironment=env.value(QStringLiteral("DESKTOP_SESSION")).trimmed();
 if(m_desktopEnvironment.isEmpty())m_desktopEnvironment=QStringLiteral("Unknown");
 if(m_sessionType.isEmpty()){
  if(!m_waylandDisplay.isEmpty())m_sessionType=QStringLiteral("wayland (inferred)");
  else if(!m_xDisplay.isEmpty())m_sessionType=QStringLiteral("x11 (inferred)");
  else m_sessionType=QStringLiteral("headless/unknown");
 }
 if(m_waylandDisplay.isEmpty())m_waylandDisplay=QStringLiteral("Not set");
 if(m_xDisplay.isEmpty())m_xDisplay=QStringLiteral("Not set");
 m_runtimeDirectory=env.value(QStringLiteral("XDG_RUNTIME_DIR")).trimmed();
 if(m_runtimeDirectory.isEmpty())m_runtimeDirectoryStatus=QStringLiteral("Not set");
 else{
  const QFileInfo runtime(m_runtimeDirectory);
  const QFile::Permissions unsafe=QFile::WriteGroup|QFile::WriteOther;
  const bool ownerMatches=runtime.ownerId()==static_cast<uint>(getuid());
  const bool safePermissions=!(QFile::permissions(m_runtimeDirectory)&unsafe);
  const bool valid=runtime.exists()&&runtime.isDir()&&runtime.isReadable()&&runtime.isWritable()&&ownerMatches&&safePermissions;
  m_runtimeDirectoryStatus=valid?QStringLiteral("Directory exists and ownership/permissions look safe"):QStringLiteral("Invalid or unsafe directory; isolated session startup must not proceed");
 }
 if(m_runtimeDirectory.isEmpty())m_runtimeDirectory=QStringLiteral("Not set");
 const QList<QPair<QString,QString>> candidates={
  {QStringLiteral("Weston"),QStringLiteral("weston")},
  {QStringLiteral("KWin Wayland"),QStringLiteral("kwin_wayland")},
  {QStringLiteral("Sway"),QStringLiteral("sway")},
  {QStringLiteral("Labwc"),QStringLiteral("labwc")},
  {QStringLiteral("Cage"),QStringLiteral("cage")},
  {QStringLiteral("Wayfire"),QStringLiteral("wayfire")},
  {QStringLiteral("River"),QStringLiteral("river")},
  {QStringLiteral("Hyprland"),QStringLiteral("Hyprland")},
  {QStringLiteral("Niri"),QStringLiteral("niri")}
 };
 for(const auto &candidate:candidates){
  const QString path=QStandardPaths::findExecutable(candidate.second);
  if(!path.isEmpty())m_availableCompositors.append(QVariantMap{{QStringLiteral("name"),candidate.first},{QStringLiteral("command"),candidate.second},{QStringLiteral("path"),path}});
 }
}
void SystemInfo::discoverNativeTools(){
 struct Candidate{QString name,command,category;};
 QList<Candidate> candidates={
  {QStringLiteral("Git"),QStringLiteral("git"),QStringLiteral("Development")},
  {QStringLiteral("Curl"),QStringLiteral("curl"),QStringLiteral("Network")},
  {QStringLiteral("Wget"),QStringLiteral("wget"),QStringLiteral("Network")},
  {QStringLiteral("Systemd manager"),QStringLiteral("systemctl"),QStringLiteral("System")},
  {QStringLiteral("Flatpak"),QStringLiteral("flatpak"),QStringLiteral("Applications")},
  {QStringLiteral("Snap"),QStringLiteral("snap"),QStringLiteral("Applications")},
  {QStringLiteral("Podman"),QStringLiteral("podman"),QStringLiteral("Containers")},
  {QStringLiteral("Docker"),QStringLiteral("docker"),QStringLiteral("Containers")},
  {QStringLiteral("Firewall control"),QStringLiteral("nft"),QStringLiteral("Security")}
 };
 if(m_family==QStringLiteral("debian")){
  candidates.append({QStringLiteral("APT"),QStringLiteral("apt-get"),QStringLiteral("Packages")});
  candidates.append({QStringLiteral("DPKG"),QStringLiteral("dpkg"),QStringLiteral("Packages")});
  candidates.append({QStringLiteral("UFW firewall"),QStringLiteral("ufw"),QStringLiteral("Security")});
 }else if(m_family==QStringLiteral("redhat")){
  candidates.append({QStringLiteral("DNF"),QStringLiteral("dnf"),QStringLiteral("Packages")});
  candidates.append({QStringLiteral("RPM"),QStringLiteral("rpm"),QStringLiteral("Packages")});
  candidates.append({QStringLiteral("FirewallD"),QStringLiteral("firewall-cmd"),QStringLiteral("Security")});
  candidates.append({QStringLiteral("SELinux tools"),QStringLiteral("semanage"),QStringLiteral("Security")});
 }else if(m_family==QStringLiteral("arch")){
  candidates.append({QStringLiteral("Pacman"),QStringLiteral("pacman"),QStringLiteral("Packages")});
  candidates.append({QStringLiteral("Yay helper"),QStringLiteral("yay"),QStringLiteral("Packages")});
  candidates.append({QStringLiteral("Paru helper"),QStringLiteral("paru"),QStringLiteral("Packages")});
 }else if(m_family==QStringLiteral("opensuse")){
  candidates.append({QStringLiteral("Zypper"),QStringLiteral("zypper"),QStringLiteral("Packages")});
  candidates.append({QStringLiteral("RPM"),QStringLiteral("rpm"),QStringLiteral("Packages")});
  candidates.append({QStringLiteral("FirewallD"),QStringLiteral("firewall-cmd"),QStringLiteral("Security")});
 }
 if(m_id==QStringLiteral("kali")){
  const QList<Candidate> security={
   {QStringLiteral("Nmap"),QStringLiteral("nmap"),QStringLiteral("Security testing")},
   {QStringLiteral("Metasploit console"),QStringLiteral("msfconsole"),QStringLiteral("Security testing")},
   {QStringLiteral("SQLmap"),QStringLiteral("sqlmap"),QStringLiteral("Security testing")},
   {QStringLiteral("Hydra"),QStringLiteral("hydra"),QStringLiteral("Security testing")},
   {QStringLiteral("Aircrack-ng"),QStringLiteral("aircrack-ng"),QStringLiteral("Wireless security")},
   {QStringLiteral("John the Ripper"),QStringLiteral("john"),QStringLiteral("Password auditing")},
   {QStringLiteral("Nikto"),QStringLiteral("nikto"),QStringLiteral("Web security")},
   {QStringLiteral("Gobuster"),QStringLiteral("gobuster"),QStringLiteral("Web security")},
   {QStringLiteral("Tcpdump"),QStringLiteral("tcpdump"),QStringLiteral("Network analysis")},
   {QStringLiteral("Hashcat"),QStringLiteral("hashcat"),QStringLiteral("Password auditing")}
  };
  candidates.append(security);
 }
 QSet<QString> seen;
 for(const Candidate &candidate:candidates){
  if(seen.contains(candidate.command))continue;seen.insert(candidate.command);
  const QString path=QStandardPaths::findExecutable(candidate.command);
  if(path.isEmpty())continue;
  m_nativeTools.append(QVariantMap{{QStringLiteral("name"),candidate.name},{QStringLiteral("command"),candidate.command},{QStringLiteral("category"),candidate.category},{QStringLiteral("path"),path}});
 }
}
void SystemInfo::refreshApplications(){
 const QStringList roots=applicationRoots();
 QSet<QString> seen;QVariantList list;
 for(const QString &root:roots){
  QDir dir(root);
  for(const QFileInfo &fi:dir.entryInfoList({QStringLiteral("*.desktop")},QDir::Files|QDir::Readable,QDir::Name)){
   QString path=fi.canonicalFilePath();if(path.isEmpty())path=fi.absoluteFilePath();
   if(seen.contains(path))continue;seen.insert(path);
   QFile file(fi.absoluteFilePath());if(!file.open(QIODevice::ReadOnly|QIODevice::Text))continue;
   const DesktopEntryInfo entry=DesktopEntryParser::parse(QString::fromUtf8(file.readAll()));
   if(entry.hidden||entry.noDisplay||entry.name.isEmpty()||entry.exec.isEmpty())continue;
   list.append(QVariantMap{{QStringLiteral("name"),entry.name},{QStringLiteral("comment"),entry.comment},{QStringLiteral("desktopFile"),fi.absoluteFilePath()}});
  }
 }
 std::sort(list.begin(),list.end(),[](const QVariant &a,const QVariant &b){
  return a.toMap().value(QStringLiteral("name")).toString().toCaseFolded()<b.toMap().value(QStringLiteral("name")).toString().toCaseFolded();
 });
 if(list.size()>250)list=list.mid(0,250);
 m_apps=list;emit applicationsChanged();setStatus(QStringLiteral("Found %1 desktop entries.").arg(m_apps.size()));
}
bool SystemInfo::launchDesktopFile(const QString &desktopFile){
 QFileInfo fi(desktopFile);const QString path=fi.canonicalFilePath();bool allowed=false;
 const QStringList roots=applicationRoots();
 for(const QString &root:roots){const QString canonicalRoot=QFileInfo(root).canonicalFilePath();if(!canonicalRoot.isEmpty()&&path.startsWith(canonicalRoot+QLatin1Char('/'))){allowed=true;break;}}
 if(path.isEmpty()||!allowed||!QFileInfo(path).isFile()||QFileInfo(path).suffix()!=QStringLiteral("desktop")){setStatus(QStringLiteral("Launch blocked: invalid desktop entry path."));return false;}
 QFile file(path);if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){setStatus(QStringLiteral("Could not read desktop entry."));return false;}
 const DesktopEntryInfo entry=DesktopEntryParser::parse(QString::fromUtf8(file.readAll()));
 if(entry.hidden||entry.noDisplay||entry.terminal||entry.exec.isEmpty()){setStatus(QStringLiteral("Entry hidden, terminal-based, or has no launch command."));return false;}
 if(!entry.tryExec.isEmpty()){
  QString tryExecutable=entry.tryExec;
  const QString resolvedTry=QFileInfo(tryExecutable).isAbsolute()?(QFileInfo(tryExecutable).isExecutable()?tryExecutable:QString()):QStandardPaths::findExecutable(tryExecutable);
  if(resolvedTry.isEmpty()){setStatus(QStringLiteral("Application is unavailable: TryExec executable '%1' was not found.").arg(entry.tryExec));return false;}
 }
 QString workingDirectory=QDir::homePath();
 if(!entry.workingDirectory.isEmpty()){
  const QFileInfo workInfo(entry.workingDirectory);
  if(workInfo.isDir()&&workInfo.isReadable())workingDirectory=workInfo.canonicalFilePath();
  else {setStatus(QStringLiteral("Application working directory is unavailable."));return false;}
 }
 QString program;QStringList args;QString parseError;
 if(!DesktopEntryParser::splitExec(entry.exec,&program,&args,&parseError,entry.name,entry.icon,path)){setStatus(parseError);return false;}
 if(QFileInfo(program).isAbsolute()){
  if(!QFileInfo(program).isExecutable()){setStatus(QStringLiteral("Executable is not available."));return false;}
 }else{
  const QString resolved=QStandardPaths::findExecutable(program);
  if(resolved.isEmpty()){setStatus(QStringLiteral("Executable not found: %1").arg(program));return false;}
  program=resolved;
 }
 if(QProcess::startDetached(program,args,workingDirectory)){setStatus(QStringLiteral("Started %1.").arg(QFileInfo(program).fileName()));return true;}
 setStatus(QStringLiteral("Failed to start application."));return false;
}
bool SystemInfo::launchTerminal(){
 const QStringList candidates{QStringLiteral("x-terminal-emulator"),QStringLiteral("kgx"),QStringLiteral("gnome-terminal"),QStringLiteral("konsole"),QStringLiteral("xfce4-terminal"),QStringLiteral("mate-terminal"),QStringLiteral("tilix"),QStringLiteral("terminator"),QStringLiteral("kitty"),QStringLiteral("alacritty"),QStringLiteral("foot"),QStringLiteral("wezterm"),QStringLiteral("xterm")};
 QString executable;
 for(const QString &candidate:candidates){executable=QStandardPaths::findExecutable(candidate);if(!executable.isEmpty())break;}
 if(executable.isEmpty()){setStatus(QStringLiteral("No terminal emulator was found on PATH. Install a terminal for your desktop environment; Win-To-Linux will not install one automatically."));return false;}
 if(!QProcess::startDetached(executable,{},QDir::homePath())){setStatus(QStringLiteral("Could not start terminal emulator: %1").arg(executable));return false;}
 setStatus(QStringLiteral("Launching terminal: %1").arg(QFileInfo(executable).fileName()));return true;
}

bool SystemInfo::launchWindowsFile(const QString&selectedPath){
 if(m_winePath.isEmpty()){setStatus(QStringLiteral("Wine was not found. Win-To-Linux will not install it automatically."));return false;}
 const QFileInfo info(selectedPath);const QString suffix=info.suffix().toLower();const QString path=info.canonicalFilePath();
 if(path.isEmpty()||!info.isFile()||(suffix!=QStringLiteral("exe")&&suffix!=QStringLiteral("msi"))){setStatus(QStringLiteral("Choose an existing .exe or .msi file."));return false;}
 QStringList args;
 if(suffix==QStringLiteral("msi"))args={QStringLiteral("msiexec"),QStringLiteral("/i"),path};else args={path};
 if(QProcess::startDetached(m_winePath,args,QDir::homePath())){setStatus(QStringLiteral("Started Windows file through Wine. Compatibility is not guaranteed."));return true;}
 setStatus(QStringLiteral("Wine could not start the selected file."));return false;
}
