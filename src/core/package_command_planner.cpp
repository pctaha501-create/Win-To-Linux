#include "package_command_planner.h"
#include <QRegularExpression>
#include <utility>
namespace wtl {
namespace {
CommandPlan noSupport() { return {{}, {}, 30000, false, QStringLiteral("No supported package manager is available.")}; }
CommandPlan make(QString program, QStringList arguments, int timeout = 30000) { return {std::move(program), std::move(arguments), timeout, true, {}}; }
}
bool PackageCommandPlanner::isValidPackageName(const QString &name) {
 static const QRegularExpression pattern(QStringLiteral("^[A-Za-z0-9][A-Za-z0-9+._:-]{0,127}$"));
 return pattern.match(name).hasMatch();
}
CommandPlan PackageCommandPlanner::checkUpdates(const DistroInfo &d) {
 if (!d.packageManagerAvailable || d.packageManagerPath.isEmpty()) return noSupport();
 const QString &n = d.packageManager;
 if (n == "apt-get") return make(d.packageManagerPath, {"-s", "upgrade"});
 if (n == "apt") return make(d.packageManagerPath, {"list", "--upgradable"});
 if (n == "dnf" || n == "yum") return make(d.packageManagerPath, {"check-update"}, 60000);
 if (n == "pacman") return make(d.packageManagerPath, {"-Qu"});
 if (n == "zypper") return make(d.packageManagerPath, {"--non-interactive", "list-updates"}, 60000);
 return noSupport();
}
CommandPlan PackageCommandPlanner::install(const DistroInfo &d, const QString &package) {
 if (!isValidPackageName(package)) return {{}, {}, 30000, false, QStringLiteral("Invalid package name.")};
 if (!d.packageManagerAvailable || d.packageManagerPath.isEmpty()) return noSupport();
 const QString &n = d.packageManager;
 if (n == "apt-get" || n == "apt" || n == "dnf" || n == "yum") return make("pkexec", {d.packageManagerPath, "install", "-y", package}, 120000);
 if (n == "pacman") return make("pkexec", {d.packageManagerPath, "-S", "--noconfirm", package}, 120000);
 if (n == "zypper") return make("pkexec", {d.packageManagerPath, "--non-interactive", "install", package}, 120000);
 return noSupport();
}
CommandPlan PackageCommandPlanner::remove(const DistroInfo &d, const QString &package) {
 if (!isValidPackageName(package)) return {{}, {}, 30000, false, QStringLiteral("Invalid package name.")};
 if (!d.packageManagerAvailable || d.packageManagerPath.isEmpty()) return noSupport();
 const QString &n = d.packageManager;
 if (n == "apt-get" || n == "apt" || n == "dnf" || n == "yum") return make("pkexec", {d.packageManagerPath, "remove", "-y", package}, 120000);
 if (n == "pacman") return make("pkexec", {d.packageManagerPath, "-R", "--noconfirm", package}, 120000);
 if (n == "zypper") return make("pkexec", {d.packageManagerPath, "--non-interactive", "remove", package}, 120000);
 return noSupport();
}
}
