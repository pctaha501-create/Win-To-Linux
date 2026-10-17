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


    const auto dnf = distro(QStringLiteral("dnf"), QStringLiteral("/usr/bin/dnf"));
    const auto dnfInstall = wtl::PackageCommandPlanner::install(dnf, QStringLiteral("vim"));
    ok &= expect(dnfInstall.supported, "dnf install plan is supported");
    ok &= expect(dnfInstall.program == QStringLiteral("pkexec"), "dnf install uses PolicyKit");
    ok &= expect(dnfInstall.arguments == QStringList{QStringLiteral("/usr/bin/dnf"), QStringLiteral("install"), QStringLiteral("-y"), QStringLiteral("vim")},
                 "dnf install arguments are discrete and exact");
    const auto dnfRemove = wtl::PackageCommandPlanner::remove(dnf, QStringLiteral("vim"));
    ok &= expect(dnfRemove.arguments == QStringList{QStringLiteral("/usr/bin/dnf"), QStringLiteral("remove"), QStringLiteral("-y"), QStringLiteral("vim")},
                 "dnf remove arguments are discrete and exact");
    const auto dnfUpdates = wtl::PackageCommandPlanner::checkUpdates(dnf);
    ok &= expect(dnfUpdates.program == QStringLiteral("/usr/bin/dnf") &&
                 dnfUpdates.arguments == QStringList{QStringLiteral("check-update")},
                 "dnf update check uses the native non-installing command");

    const auto zypper = distro(QStringLiteral("zypper"), QStringLiteral("/usr/bin/zypper"));
    const auto zypperInstall = wtl::PackageCommandPlanner::install(zypper, QStringLiteral("firefox"));
    ok &= expect(zypperInstall.arguments == QStringList{QStringLiteral("/usr/bin/zypper"), QStringLiteral("--non-interactive"),
                                                         QStringLiteral("install"), QStringLiteral("firefox")},
                 "zypper install arguments are discrete and non-interactive");
    const auto zypperRemove = wtl::PackageCommandPlanner::remove(zypper, QStringLiteral("firefox"));
    ok &= expect(zypperRemove.arguments == QStringList{QStringLiteral("/usr/bin/zypper"), QStringLiteral("--non-interactive"),
                                                        QStringLiteral("remove"), QStringLiteral("firefox")},
                 "zypper remove arguments are discrete and non-interactive");
    const auto zypperUpdates = wtl::PackageCommandPlanner::checkUpdates(zypper);
    ok &= expect(zypperUpdates.arguments == QStringList{QStringLiteral("--non-interactive"), QStringLiteral("list-updates")},
                 "zypper update check uses list-updates");

    const auto aptBinary = distro(QStringLiteral("apt"), QStringLiteral("/usr/bin/apt"));
    const auto aptUpdates = wtl::PackageCommandPlanner::checkUpdates(aptBinary);
    ok &= expect(aptUpdates.arguments == QStringList{QStringLiteral("list"), QStringLiteral("--upgradable")},
                 "apt update check only lists upgradable packages");

    const auto alpine = distro(QStringLiteral("apk"), QStringLiteral("/sbin/apk"));
    const auto apkUpdates = wtl::PackageCommandPlanner::checkUpdates(alpine);
    ok &= expect(apkUpdates.supported && apkUpdates.program == QStringLiteral("/sbin/apk"),
                 "Alpine update check uses the detected apk executable");
    ok &= expect(apkUpdates.arguments == QStringList{QStringLiteral("version"), QStringLiteral("-l"), QStringLiteral("<")},
                 "Alpine update check lists locally known outdated packages without installing anything");
    const auto apkInstall = wtl::PackageCommandPlanner::install(alpine, QStringLiteral("curl"));
    ok &= expect(apkInstall.program == QStringLiteral("pkexec") &&
                 apkInstall.arguments == QStringList{QStringLiteral("/sbin/apk"), QStringLiteral("add"), QStringLiteral("curl")},
                 "Alpine installs packages through PolicyKit and apk add");
    const auto apkRemove = wtl::PackageCommandPlanner::remove(alpine, QStringLiteral("curl"));
    ok &= expect(apkRemove.program == QStringLiteral("pkexec") &&
                 apkRemove.arguments == QStringList{QStringLiteral("/sbin/apk"), QStringLiteral("del"), QStringLiteral("curl")},
                 "Alpine removes packages through PolicyKit and apk del");

    if (!ok) return 1;
    qInfo() << "Win-To-Linux package planner safety tests passed";
    return 0;
}
