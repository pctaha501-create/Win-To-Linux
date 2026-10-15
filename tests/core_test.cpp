#include "distro_detector.h"
#include "package_command_planner.h"
#include <QCoreApplication>
#include <QDebug>
namespace { bool expect(bool ok,const char*m){if(!ok)qCritical()<<"FAIL:"<<m;return ok;} wtl::DistroInfo fake(QString n){wtl::DistroInfo d;d.packageManager=n;d.packageManagerPath=n.isEmpty()?QString():QString("/fake/bin/")+n;d.packageManagerAvailable=!n.isEmpty();return d;} }
int main(int argc,char**argv){QCoreApplication app(argc,argv);
auto u=wtl::DistroDetector::parse("NAME=\\"Ubuntu\\"\\nID=ubuntu\\nID_LIKE=debian\\n");if(!expect(u.family==wtl::DistroFamily::Debian,"Ubuntu detection")||!expect(u.packageManager=="apt-get","apt selection"))return 1;
if(!expect(wtl::DistroDetector::parse("ID=kali\\n").family==wtl::DistroFamily::Debian,"Kali family"))return 1;
if(!expect(wtl::DistroDetector::parse("ID=fedora\\n").family==wtl::DistroFamily::RedHat,"Fedora family"))return 1;
if(!expect(wtl::DistroDetector::parse("ID=arch\\n").family==wtl::DistroFamily::Arch,"Arch family"))return 1;
if(!expect(wtl::DistroDetector::parse("ID=opensuse-tumbleweed\\n").family==wtl::DistroFamily::OpenSUSE,"openSUSE family"))return 1;
auto unknown=wtl::DistroDetector::parse("ID=unknown-os\\n");if(!expect(unknown.family==wtl::DistroFamily::Generic,"unknown fallback")||!expect(!unknown.packageManagerAvailable,"no fabricated manager"))return 1;
auto i=wtl::PackageCommandPlanner::install(fake("apt-get"),"curl");if(!expect(i.supported&&i.program=="pkexec","apt install plan")||!expect(i.arguments==QStringList{"/fake/bin/apt-get","install","-y","curl"},"argument list safe"))return 1;
auto d=wtl::PackageCommandPlanner::remove(fake("dnf"),"curl");if(!expect(d.supported&&d.arguments.at(1)=="remove","dnf remove"))return 1;
auto p=wtl::PackageCommandPlanner::install(fake("pacman"),"curl");if(!expect(p.supported&&p.arguments.at(1)=="-S","pacman install"))return 1;
auto z=wtl::PackageCommandPlanner::checkUpdates(fake("zypper"));if(!expect(z.supported&&z.arguments.contains("list-updates"),"zypper update check"))return 1;
if(!expect(!wtl::PackageCommandPlanner::install(fake("apt-get"),"curl;rm -rf /").supported,"reject shell metacharacters"))return 1;
if(!expect(!wtl::PackageCommandPlanner::install(fake(""),"curl").supported,"unsupported package manager"))return 1;
qInfo()<<"Win-To-Linux core tests passed";return 0;}
