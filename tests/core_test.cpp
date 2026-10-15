#include "distro_detector.h"
#include "package_command_planner.h"
#include <QCoreApplication>
#include <QDebug>
namespace {
bool expect(bool ok,const char *m){if(!ok)qCritical().noquote()<<"FAIL:"<<m;return ok;}
wtl::DistroInfo fake(const QString &n){wtl::DistroInfo d;d.packageManager=n;d.packageManagerPath=n.isEmpty()?QString():QStringLiteral("/fake/bin/")+n;d.packageManagerAvailable=!n.isEmpty();return d;}
}
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);
 const auto u=wtl::DistroDetector::parse(R"(NAME="Ubuntu"
ID=ubuntu
ID_LIKE=debian
VERSION_ID="24.04"
)");
 if(!expect(u.id=="ubuntu","Ubuntu ID")||!expect(u.family==wtl::DistroFamily::Debian,"Ubuntu family")||!expect(u.packageManager=="apt-get","apt-get selection"))return 1;
 if(!expect(wtl::DistroDetector::parse("ID=kali\n").family==wtl::DistroFamily::Debian,"Kali family"))return 1;
 if(!expect(wtl::DistroDetector::parse("ID=fedora\n").family==wtl::DistroFamily::RedHat,"Fedora family"))return 1;
 if(!expect(wtl::DistroDetector::parse("ID=arch\n").family==wtl::DistroFamily::Arch,"Arch family"))return 1;
 if(!expect(wtl::DistroDetector::parse("ID=opensuse-tumbleweed\n").family==wtl::DistroFamily::OpenSUSE,"openSUSE family"))return 1;
 const auto unknown=wtl::DistroDetector::parse("ID=unknown-os\n");
 if(!expect(unknown.family==wtl::DistroFamily::Generic,"unknown family")||!expect(!unknown.packageManagerAvailable,"unknown manager not fabricated"))return 1;
 if(!expect(wtl::DistroDetector::parse("ID=ubuntu\nID=fedora\n").family==wtl::DistroFamily::Debian,"first duplicate key wins"))return 1;
 const auto i=wtl::PackageCommandPlanner::install(fake("apt-get"),"curl");
 if(!expect(i.supported&&i.program=="pkexec","apt install uses auth")||!expect(i.arguments==QStringList{"/fake/bin/apt-get","install","-y","curl"},"arguments stay separated"))return 1;
 if(!expect(wtl::PackageCommandPlanner::remove(fake("dnf"),"curl").arguments.at(1)=="remove","dnf remove"))return 1;
 if(!expect(wtl::PackageCommandPlanner::install(fake("pacman"),"curl").arguments.at(1)=="-S","pacman install"))return 1;
 if(!expect(wtl::PackageCommandPlanner::checkUpdates(fake("zypper")).arguments.contains("list-updates"),"zypper updates"))return 1;
 if(!expect(!wtl::PackageCommandPlanner::install(fake("apt-get"),"curl;rm -rf /").supported,"reject shell metacharacters"))return 1;
 if(!expect(!wtl::PackageCommandPlanner::install(fake(""),"curl").supported,"no manager is safe"))return 1;
 if(!expect(!wtl::PackageCommandPlanner::install(fake("apt-get"),"").supported,"empty package rejected"))return 1;
 qInfo().noquote()<<"Win-To-Linux core tests passed";return 0;
}
