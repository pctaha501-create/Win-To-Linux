#include "distro_detector.h"
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSet>
namespace wtl {
namespace {
QString unquote(QString v) { v=v.trimmed(); if(v.size()>=2) { QChar a=v.front(), b=v.back(); if((a=='"'&&b=='"')||(a=='\''&&b=='\'')) v=v.mid(1,v.size()-2); } return v; }
QStringList words(const QString &s) { return s.toLower().split(QRegularExpression(QStringLiteral("\\s+")),Qt::SkipEmptyParts); }
QString findExe(const QString &p,const QStringList &dirs) { for(const auto &d:dirs) { QFileInfo f(d+QLatin1Char('/')+p); if(f.isFile()&&f.isExecutable()) return f.canonicalFilePath().isEmpty()?f.absoluteFilePath():f.canonicalFilePath(); } return {}; }
}
QString DistroDetector::familyName(DistroFamily f) { switch(f) { case DistroFamily::Debian:return "debian"; case DistroFamily::RedHat:return "redhat"; case DistroFamily::Arch:return "arch"; case DistroFamily::OpenSUSE:return "opensuse"; case DistroFamily::Alpine:return "alpine"; case DistroFamily::Generic:return "generic"; } return "generic"; }
DistroInfo DistroDetector::detect(const QString &path,const QStringList &paths) { QFile f(path); if(!f.open(QIODevice::ReadOnly|QIODevice::Text)) { DistroInfo d; d.error=QStringLiteral("Cannot read distribution metadata: %1").arg(path); return d; } return parse(f.readAll(),paths); }
DistroInfo DistroDetector::parse(const QByteArray &data,const QStringList &requestedPaths) {
 QSet<QString> seen; QString id,idLike,name,version; static const QRegularExpression re(QStringLiteral("^([A-Z][A-Z0-9_]*)=(.*)$"));
 for(QString line:QString::fromUtf8(data).split('\n')) { line=line.trimmed(); if(line.isEmpty()||line.startsWith('#')) continue; auto m=re.match(line); if(!m.hasMatch()) continue; QString k=m.captured(1); if(seen.contains(k)) continue; seen.insert(k); QString v=unquote(m.captured(2)); if(k=="ID") id=v.toLower(); else if(k=="ID_LIKE") idLike=v.toLower(); else if(k=="NAME") name=v; else if(k=="VERSION_ID") version=v; }
 QStringList ids{id}; ids.append(words(idLike)); QSet<QString> s(ids.begin(),ids.end()); DistroFamily family=DistroFamily::Generic;
 if(s.contains("ubuntu")||s.contains("debian")||s.contains("kali")||s.contains("linuxmint")||s.contains("pop")||s.contains("elementary")) family=DistroFamily::Debian;
 else if(s.contains("fedora")||s.contains("rhel")||s.contains("redhat")||s.contains("centos")||s.contains("rocky")||s.contains("almalinux")) family=DistroFamily::RedHat;
 else if(s.contains("arch")||s.contains("manjaro")||s.contains("endeavouros")) family=DistroFamily::Arch;
 else if(s.contains("opensuse")||s.contains("opensuse-leap")||s.contains("opensuse-tumbleweed")||s.contains("suse")) family=DistroFamily::OpenSUSE;
 else if(s.contains("alpine")) family=DistroFamily::Alpine;
 DistroInfo d; d.id=id; d.idLike=idLike; d.name=name; d.versionId=version; d.family=family; d.familyName=familyName(family);
 switch(family) { case DistroFamily::Debian:d.packageManager="apt-get";break; case DistroFamily::RedHat:d.packageManager="dnf";break; case DistroFamily::Arch:d.packageManager="pacman";break; case DistroFamily::OpenSUSE:d.packageManager="zypper";break; case DistroFamily::Alpine:d.packageManager="apk";break; case DistroFamily::Generic:break; }
 QStringList paths=requestedPaths; if(paths.isEmpty()) paths=QProcessEnvironment::systemEnvironment().value("PATH").split(':',Qt::SkipEmptyParts);
 if(!d.packageManager.isEmpty()) { QString p=findExe(d.packageManager,paths); if(p.isEmpty()&&family==DistroFamily::Debian) { p=findExe("apt",paths); if(!p.isEmpty()) d.packageManager="apt"; } if(p.isEmpty()&&family==DistroFamily::RedHat) { p=findExe("yum",paths); if(!p.isEmpty()) d.packageManager="yum"; } d.packageManagerPath=p; d.packageManagerAvailable=!p.isEmpty(); }
 return d;
}
}
