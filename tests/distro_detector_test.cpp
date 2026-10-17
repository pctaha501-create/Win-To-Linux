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

    const auto debian = wtl::DistroDetector::parse(
        R"(NAME="Debian GNU/Linux"
ID=debian
VERSION_ID="13"
)", noExecutables);
    ok &= expect(debian.family == wtl::DistroFamily::Debian, "Debian is recognized without ID_LIKE");
    ok &= expect(debian.packageManager == "apt-get", "Debian selects apt-get");

    const auto fedora = wtl::DistroDetector::parse(
        R"(NAME="Fedora Linux"
ID=fedora
VERSION_ID="42"
)", noExecutables);
    ok &= expect(fedora.family == wtl::DistroFamily::RedHat, "Fedora is recognized without ID_LIKE");
    ok &= expect(fedora.packageManager == "dnf", "Fedora selects dnf");

    const auto mint = wtl::DistroDetector::parse(
        R"(NAME="Linux Mint"
ID=linuxmint
ID_LIKE="ubuntu debian"
VERSION_ID="22"
)", noExecutables);
    ok &= expect(mint.family == wtl::DistroFamily::Debian, "Linux Mint follows the Debian family");
    ok &= expect(mint.packageManager == "apt-get", "Linux Mint selects apt-get");

    const auto manjaro = wtl::DistroDetector::parse(
        R"(NAME="Manjaro Linux"
ID=manjaro
ID_LIKE=arch
VERSION_ID="25.0"
)", noExecutables);
    ok &= expect(manjaro.family == wtl::DistroFamily::Arch, "Manjaro follows the Arch family");
    ok &= expect(manjaro.packageManager == "pacman", "Manjaro selects pacman");

    const auto alma = wtl::DistroDetector::parse(
        R"(NAME="AlmaLinux"
ID=almalinux
ID_LIKE="rhel centos fedora"
VERSION_ID="9.5"
)", noExecutables);
    ok &= expect(alma.family == wtl::DistroFamily::RedHat, "AlmaLinux follows the Red Hat family");
    ok &= expect(alma.packageManager == "dnf", "AlmaLinux selects dnf");

    const auto endeavouros = wtl::DistroDetector::parse(
        R"(NAME="EndeavourOS"
ID=endeavouros
ID_LIKE=arch
)", noExecutables);
    ok &= expect(endeavouros.family == wtl::DistroFamily::Arch, "EndeavourOS follows the Arch family");
    ok &= expect(endeavouros.packageManager == "pacman", "EndeavourOS selects pacman");

    const auto alpine = wtl::DistroDetector::parse(
        R"(NAME="Alpine Linux"
ID=alpine
VERSION_ID="3.22.0"
)" , noExecutables);
    ok &= expect(alpine.family == wtl::DistroFamily::Alpine, "Alpine Linux is recognized as its own family");
    ok &= expect(alpine.familyName == QStringLiteral("alpine"), "Alpine family label is stable");
    ok &= expect(alpine.packageManager == QStringLiteral("apk"), "Alpine selects apk");
    ok &= expect(!alpine.packageManagerAvailable, "Alpine does not claim apk is installed when PATH has no apk");

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
