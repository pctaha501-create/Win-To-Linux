#include "distro_detector.h"
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QRegularExpression>
namespace wtl {
namespace {
QString unquote(QString value) {
 value=value.trimmed();
 if(value.size()<2)return value;
 const QChar quote=value.front();
 if((quote!='"'&&quote!='\'')||value.back()!=quote)return value;
 value=value.mid(1,value.size()-2);
 if(quote!='"')return value;
 QString decoded;decoded.reserve(value.size());bool escaped=false;
 for(const QChar ch:value){
  if(escaped){
   if(ch=='\\'||ch=='"'||ch=='$'||ch=='`')decoded+=ch;
   else{decoded+='\\';decoded+=ch;}
   escaped=false;
  }else if(ch=='\\')escaped=true;
  else decoded+=ch;
 }
 if(escaped)decoded+='\\';
 return decoded;
}
QStringList words(const QString &s) { return s.toLower().split(QRegularExpression(QStringLiteral("\\s+")),Qt::SkipEmptyParts); }
DistroFamily familyForToken(const QString &token) {
 if(token=="ubuntu"||token=="debian"||token=="kali"||token=="linuxmint"||token=="pop"||token=="elementary") return DistroFamily::Debian;
 if(token=="fedora"||token=="rhel"||token=="redhat"||token=="centos"||token=="rocky"||token=="almalinux"||token=="ol") return DistroFamily::RedHat;
 if(token=="arch"||token=="manjaro"||token=="endeavouros") return DistroFamily::Arch;
 if(token=="opensuse"||token=="opensuse-leap"||token=="opensuse-tumbleweed"||token=="suse") return DistroFamily::OpenSUSE;
 if(token=="alpine") return DistroFamily::Alpine;
 if(token=="void") return DistroFamily::Void;
 return DistroFamily::Generic;
}
QString findExe(const QString &p,const QStringList &dirs) { for(const auto &d:dirs) { QFileInfo f(d+QLatin1Char('/')+p); if(f.isFile()&&f.isExecutable()) return f.canonicalFilePath().isEmpty()?f.absoluteFilePath():f.canonicalFilePath(); } return {}; }
}
QString DistroDetector::familyName(DistroFamily f) { switch(f) { case DistroFamily::Debian:return "debian"; case DistroFamily::RedHat:return "redhat"; case DistroFamily::Arch:return "arch"; case DistroFamily::OpenSUSE:return "opensuse"; case DistroFamily::Alpine:return "alpine"; case DistroFamily::Void:return "void"; case DistroFamily::Generic:return "generic"; } return "generic"; }
DistroInfo DistroDetector::detect(const QString &path,const QStringList &paths) { QFile f(path); if(!f.open(QIODevice::ReadOnly|QIODevice::Text)) { DistroInfo d; d.error=QStringLiteral("Cannot read distribution metadata: %1").arg(path); return d; } return parse(f.readAll(),paths); }
DistroInfo DistroDetector::parse(const QByteArray &data,const QStringList &requestedPaths) {
 QString id,idLike,name,version; static const QRegularExpression re(QStringLiteral("^([A-Z][A-Z0-9_]*)=(.*)$"));
 for(QString line:QString::fromUtf8(data).split('\n')) { line=line.trimmed(); if(line.isEmpty()||line.startsWith('#')) continue; auto m=re.match(line); if(!m.hasMatch()) continue; QString k=m.captured(1); QString v=unquote(m.captured(2)); if(k=="ID") id=v.toLower(); else if(k=="ID_LIKE") idLike=v.toLower(); else if(k=="NAME") name=v; else if(k=="VERSION_ID") version=v; }
 // ID is the concrete distribution and always takes precedence. If it is
 // unknown, os-release specifies ID_LIKE as an ordered compatibility list:
 // the first recognized token is the closest family, so do not flatten it
 // into a QSet or impose an unrelated global family priority.
 DistroFamily family=familyForToken(id);
 if(family==DistroFamily::Generic) {
  for(const QString &token:words(idLike)) {
   family=familyForToken(token);
   if(family!=DistroFamily::Generic) break;
  }
 }
 DistroInfo d; d.id=id; d.idLike=idLike; d.name=name; d.versionId=version; d.family=family; d.familyName=familyName(family);
 switch(family) { case DistroFamily::Debian:d.packageManager="apt-get";break; case DistroFamily::RedHat:d.packageManager="dnf";break; case DistroFamily::Arch:d.packageManager="pacman";break; case DistroFamily::OpenSUSE:d.packageManager="zypper";break; case DistroFamily::Alpine:d.packageManager="apk";break; case DistroFamily::Void:d.packageManager="xbps-install";break; case DistroFamily::Generic:break; }
 QStringList paths=requestedPaths; if(paths.isEmpty()) paths=QProcessEnvironment::systemEnvironment().value("PATH").split(':',Qt::SkipEmptyParts);
 if(!d.packageManager.isEmpty()) { QString p=findExe(d.packageManager,paths); if(p.isEmpty()&&family==DistroFamily::Debian) { p=findExe("apt",paths); if(!p.isEmpty()) d.packageManager="apt"; } if(p.isEmpty()&&family==DistroFamily::RedHat) { p=findExe("yum",paths); if(!p.isEmpty()) d.packageManager="yum"; } d.packageManagerPath=p; d.packageManagerAvailable=!p.isEmpty(); if(family==DistroFamily::Void)d.packageManagerRemovePath=findExe("xbps-remove",paths); }
 return d;
}
}
