#include "distro_detector.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTemporaryDir>

namespace {
bool expect(bool condition, const char *message) {
    if (!condition) qCritical().noquote() << "FAIL:" << message;
    return condition;
}
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    bool ok = true;
    const QStringList noExecutables{QStringLiteral("/path-that-does-not-exist")};

    const auto ubuntu = wtl::DistroDetector::parse(
        R"(NAME="Ubuntu"
ID=ubuntu
ID_LIKE=debian
VERSION_ID="24.04"
)", noExecutables);
    ok &= expect(ubuntu.id == "ubuntu", "preserve Ubuntu ID");
    ok &= expect(ubuntu.family == wtl::DistroFamily::Debian, "Ubuntu uses Debian family");
    ok &= expect(ubuntu.packageManager == "apt-get", "Ubuntu selects apt-get");
    ok &= expect(!ubuntu.packageManagerAvailable, "do not invent a missing package manager");

    const auto kali = wtl::DistroDetector::parse(
        R"(NAME="Kali GNU/Linux"
ID=kali
ID_LIKE="debian"
VERSION_ID="2026.1"
)", noExecutables);
    ok &= expect(kali.id == "kali", "preserve the concrete distribution ID");
    ok &= expect(kali.family == wtl::DistroFamily::Debian, "Kali follows Debian family");
    ok &= expect(kali.packageManager == "apt-get", "Kali selects apt-get");

    const auto rocky = wtl::DistroDetector::parse(
        R"(NAME="Rocky Linux"
ID=rocky
ID_LIKE="rhel centos fedora"
VERSION_ID="9.5"
)", noExecutables);
    ok &= expect(rocky.family == wtl::DistroFamily::RedHat, "Rocky follows the Red Hat family");
    ok &= expect(rocky.packageManager == "dnf", "Rocky selects dnf");

    const auto arch = wtl::DistroDetector::parse(
        R"(NAME="Arch Linux"
ID=arch
)", noExecutables);
    ok &= expect(arch.family == wtl::DistroFamily::Arch, "Arch Linux is recognized");
    ok &= expect(arch.packageManager == "pacman", "Arch selects pacman");

    const auto opensuse = wtl::DistroDetector::parse(
        R"(NAME="openSUSE Tumbleweed"
ID="opensuse-tumbleweed"
ID_LIKE="opensuse suse"
)", noExecutables);
    ok &= expect(opensuse.family == wtl::DistroFamily::OpenSUSE, "openSUSE variant is recognized");
    ok &= expect(opensuse.packageManager == "zypper", "openSUSE selects zypper");

    const auto unknown = wtl::DistroDetector::parse(
        R"(NAME="Example OS"
ID=example-os
)", noExecutables);
    ok &= expect(unknown.family == wtl::DistroFamily::Generic, "unknown distro remains generic");
    ok &= expect(unknown.packageManager.isEmpty(), "unknown distro never guesses a package manager");
    ok &= expect(!unknown.packageManagerAvailable, "unknown distro never fabricates manager availability");

    QTemporaryDir temp;
    ok &= expect(temp.isValid(), "create isolated temporary directory");
    if (temp.isValid()) {
        const QString fakeDnf = temp.filePath("dnf");
        QFile executable(fakeDnf);
        if (executable.open(QIODevice::WriteOnly)) {
            executable.write("#!/bin/sh\nexit 0\n");
            executable.close();
            executable.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                      QFileDevice::ExeOwner);
            const auto detected = wtl::DistroDetector::parse(
                R"(NAME="Fedora Linux"
ID=fedora
VERSION_ID="42"
)", QStringList{temp.path()});
            ok &= expect(detected.packageManagerAvailable, "manager availability follows executable discovery");
            ok &= expect(detected.packageManagerPath == fakeDnf, "resolved manager path is reported exactly");
        } else {
            ok &= expect(false, "create isolated fake package-manager executable");
        }
    }

    if (!ok) return 1;
    qInfo().noquote() << "Win-To-Linux distro detector tests passed";
    return 0;
}
