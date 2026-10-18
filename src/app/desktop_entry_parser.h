#pragma once
#include <QString>
#include <QStringList>

struct DesktopEntryInfo {
 QString name;
 QString type;
 QString comment;
 QString exec;
 QString tryExec;
 QString workingDirectory;
 QString icon;
 bool hidden = false;
 bool noDisplay = false;
 bool terminal = false;
};

class DesktopEntryParser final {
public:
 static DesktopEntryInfo parse(const QString &contents, const QString &localeName = {});
 static bool splitExec(const QString &exec, QString *program, QStringList *arguments, QString *error=nullptr,
                       const QString &applicationName={}, const QString &iconName={}, const QString &desktopFile={});
};
