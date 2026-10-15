#pragma once
#include <QString>
#include <QStringList>
namespace wtl {
enum class DistroFamily { Debian, RedHat, Arch, OpenSUSE, Generic };
struct DistroInfo {
 QString id, idLike, name, versionId;
 DistroFamily family = DistroFamily::Generic;
 QString familyName = QStringLiteral("generic");
 QString packageManager;
 bool packageManagerAvailable = false;
 QString packageManagerPath, error;
};
class DistroDetector final {
public:
 static DistroInfo detect(const QString &osReleasePath = QStringLiteral("/etc/os-release"), const QStringList &searchPaths = {});
 static DistroInfo parse(const QByteArray &contents, const QStringList &searchPaths = {});
 static QString familyName(DistroFamily family);
};
}
