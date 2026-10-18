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

    const auto escaped = wtl::DistroDetector::parse(
        R"(NAME="Example \"OS\""
ID=example
VERSION_ID="3.1\\beta"
)" , noExecutables);
    ok &= expect(escaped.name == QStringLiteral("Example \"OS\""), "decode escaped quotes in os-release NAME");
    ok &= expect(escaped.versionId == QStringLiteral("3.1\\beta"), "decode escaped backslashes in os-release VERSION_ID");

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

    const auto voidLinux = wtl::DistroDetector::parse(
        R"(NAME="Void Linux"
ID=void
VERSION_ID="rolling"
)", noExecutables);
    ok &= expect(voidLinux.family == wtl::DistroFamily::Void, "Void Linux is recognized as its own family");
    ok &= expect(voidLinux.packageManager == QStringLiteral("xbps-install"), "Void Linux selects XBPS");
    ok &= expect(!voidLinux.packageManagerAvailable, "Void does not claim XBPS is installed when PATH lacks it");

    const auto duplicateAssignments = wtl::DistroDetector::parse(
        R"(NAME="First name"
ID=ubuntu
ID=rocky
VERSION_ID="1"
)", noExecutables);
    ok &= expect(duplicateAssignments.id == QStringLiteral("ubuntu"),
                 "first duplicate os-release assignment is kept as the defensive parsing policy");
    ok &= expect(duplicateAssignments.family == wtl::DistroFamily::Debian,
                 "family mapping follows the first duplicate ID assignment");

    const auto orderedLike = wtl::DistroDetector::parse(
        R"(NAME="Derivative with ordered compatibility"
ID=example-os
ID_LIKE="rhel debian"
VERSION_ID="1"
)", noExecutables);
    ok &= expect(orderedLike.family == wtl::DistroFamily::RedHat,
                 "ID_LIKE chooses the first recognized compatibility family in declared order");
    ok &= expect(orderedLike.packageManager == QStringLiteral("dnf"),
                 "ordered ID_LIKE also selects the corresponding native package manager");

    const auto concreteIdWins = wtl::DistroDetector::parse(
        R"(NAME="Concrete Arch derivative"
ID=manjaro
ID_LIKE="debian rhel"
)", noExecutables);
    ok &= expect(concreteIdWins.family == wtl::DistroFamily::Arch,
                 "concrete distribution ID takes precedence over conflicting compatibility hints");

    const auto oracleLinux = wtl::DistroDetector::parse(
        R"(NAME="Oracle Linux"
ID=ol
ID_LIKE="fedora"
VERSION_ID="9"
)", noExecutables);
    ok &= expect(oracleLinux.family == wtl::DistroFamily::RedHat,
                 "Oracle Linux ID is recognized as Red Hat compatible");
    ok &= expect(oracleLinux.packageManager == QStringLiteral("dnf"),
                 "Oracle Linux selects the native DNF-family package manager");

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
        const QString osReleasePath = temp.filePath(QStringLiteral("os-release"));
        QFile osRelease(osReleasePath);
        if (osRelease.open(QIODevice::WriteOnly | QIODevice::Text)) {
            osRelease.write("NAME=\"Debian GNU/Linux\"\nID=debian\nVERSION_ID=\"13\"\n");
            osRelease.close();
            const auto fromFile = wtl::DistroDetector::detect(osReleasePath, noExecutables);
            ok &= expect(fromFile.error.isEmpty(), "readable os-release metadata has no error");
            ok &= expect(fromFile.id == QStringLiteral("debian") && fromFile.versionId == QStringLiteral("13"),
                         "detect reads concrete distribution metadata from the supplied file");
            ok &= expect(fromFile.family == wtl::DistroFamily::Debian,
                         "file-based detection uses the same family mapping as parsing");
        } else {
            ok &= expect(false, "create temporary os-release fixture");
        }
        const auto missingMetadata = wtl::DistroDetector::detect(temp.filePath(QStringLiteral("missing-os-release")), noExecutables);
        ok &= expect(!missingMetadata.error.isEmpty(), "missing os-release metadata returns an actionable error");
        ok &= expect(missingMetadata.family == wtl::DistroFamily::Generic,
                     "missing metadata fails closed instead of guessing a distro family");
    }
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

    if (temp.isValid()) {
        bool binariesCreated = true;
        for (const QString &name : {QStringLiteral("xbps-install"), QStringLiteral("xbps-remove")}) {
            QFile executable(temp.filePath(name));
            if (!executable.open(QIODevice::WriteOnly | QIODevice::Truncate)) { binariesCreated = false; break; }
            executable.write("#!/bin/sh\nexit 0\n"); executable.close();
            if (!executable.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner)) { binariesCreated = false; break; }
        }
        ok &= expect(binariesCreated, "create isolated fake XBPS install and remove executables");
        if (binariesCreated) {
            const auto detectedVoid = wtl::DistroDetector::parse(
                R"(NAME="Void Linux"
ID=void
VERSION_ID="rolling"
)", QStringList{temp.path()});
            ok &= expect(detectedVoid.packageManagerAvailable, "Void manager availability follows xbps-install discovery");
            ok &= expect(detectedVoid.packageManagerPath == temp.filePath("xbps-install"), "Void install path is resolved exactly");
            ok &= expect(detectedVoid.packageManagerRemovePath == temp.filePath("xbps-remove"), "Void remove path is resolved independently");
        }
    }

    if (!ok) return 1;
    qInfo().noquote() << "Win-To-Linux distro detector tests passed";
    return 0;
}
