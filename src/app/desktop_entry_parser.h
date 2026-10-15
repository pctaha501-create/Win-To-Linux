#pragma once
#include <QString>
#include <QStringList>

struct DesktopEntryInfo {
 QString name;
 QString comment;
 QString exec;
 bool hidden = false;
 bool noDisplay = false;
 bool terminal = false;
};

class DesktopEntryParser final {
public:
 static DesktopEntryInfo parse(const QString &contents);
 static bool splitExec(const QString &exec, QString *program, QStringList *arguments, QString *error=nullptr);
};
