#include "package_command_planner.h"

#include <QCoreApplication>
#include <QDebug>

namespace {
bool expect(bool ok, const char *message) {
    if (!ok) qCritical().noquote() << "FAIL:" << message;
    return ok;
}
wtl::DistroInfo distro(const QString &manager, const QString &path = QStringLiteral("/usr/bin/fake-manager")) {
    wtl::DistroInfo d;
    d.id = QStringLiteral("fixture");
    d.family = wtl::DistroFamily::Generic;
    d.packageManager = manager;
    d.packageManagerPath = path;
    d.packageManagerAvailable = !path.isEmpty();
    return d;
}
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    bool ok = true;

    ok &= expect(wtl::PackageCommandPlanner::isValidPackageName(QStringLiteral("firefox")),
                 "accept ordinary package names");
    ok &= expect(wtl::PackageCommandPlanner::isValidPackageName(QStringLiteral("libfoo++-dev:amd64")),
                 "accept common package punctuation");
    ok &= expect(!wtl::PackageCommandPlanner::isValidPackageName(QString()),
                 "reject empty package name");
    ok &= expect(!wtl::PackageCommandPlanner::isValidPackageName(QStringLiteral("../important")),
                 "reject path traversal");
    ok &= expect(!wtl::PackageCommandPlanner::isValidPackageName(QStringLiteral("curl; touch /tmp/pwned")),
                 "reject shell metacharacters and whitespace");
    ok &= expect(!wtl::PackageCommandPlanner::isValidPackageName(QStringLiteral("-y")),
                 "reject option-like package name");
    ok &= expect(!wtl::PackageCommandPlanner::isValidPackageName(QString(129, QLatin1Char('a'))),
                 "reject overlong package name");

    const auto apt = distro(QStringLiteral("apt-get"), QStringLiteral("/usr/bin/apt-get"));
    const auto aptInstall = wtl::PackageCommandPlanner::install(apt, QStringLiteral("curl"));
    ok &= expect(aptInstall.supported, "apt install plan is supported");
    ok &= expect(aptInstall.program == QStringLiteral("pkexec"), "privileged installs use PolicyKit");
    ok &= expect(aptInstall.arguments == QStringList{QStringLiteral("/usr/bin/apt-get"),
                 QStringLiteral("install"), QStringLiteral("-y"), QStringLiteral("curl")},
                 "apt install arguments remain separated and exact");

    const auto pacman = distro(QStringLiteral("pacman"), QStringLiteral("/usr/bin/pacman"));
    const auto pacmanRemove = wtl::PackageCommandPlanner::remove(pacman, QStringLiteral("nano"));
    ok &= expect(pacmanRemove.supported, "pacman remove plan is supported");
    ok &= expect(pacmanRemove.arguments == QStringList{QStringLiteral("/usr/bin/pacman"),
                 QStringLiteral("-R"), QStringLiteral("--noconfirm"), QStringLiteral("nano")},
                 "pacman remove uses discrete arguments");

    const auto unavailable = distro(QStringLiteral("dnf"), QString());
    ok &= expect(!wtl::PackageCommandPlanner::install(unavailable, QStringLiteral("vim")).supported,
                 "reject operations when manager path is unavailable");
    ok &= expect(!wtl::PackageCommandPlanner::remove(distro(QStringLiteral("mystery")),
                 QStringLiteral("vim")).supported, "unknown manager is never guessed");

    if (!ok) return 1;
    qInfo() << "Win-To-Linux package planner safety tests passed";
    return 0;
}
