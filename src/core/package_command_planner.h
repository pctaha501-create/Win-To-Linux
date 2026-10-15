#pragma once
#include "distro_detector.h"
#include <QString>
#include <QStringList>
namespace wtl {
struct CommandPlan { QString program; QStringList arguments; int timeoutMs=30000; bool supported=false; QString error; };
class PackageCommandPlanner final {
public: static CommandPlan checkUpdates(const DistroInfo&); static CommandPlan install(const DistroInfo&,const QString&); static CommandPlan remove(const DistroInfo&,const QString&); static bool isValidPackageName(const QString&);
};
}
