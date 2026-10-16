#include "desktop_entry_parser.h"
#include <QProcess>
#include <algorithm>

DesktopEntryInfo DesktopEntryParser::parse(const QString &contents) {
 DesktopEntryInfo result;
 bool inDesktopEntry=false;
 for(const QString &rawLine:contents.split(QLatin1Char('\n'))){
  const QString line=rawLine.trimmed();
  if(line.isEmpty()||line.startsWith(QLatin1Char('#')))continue;
  if(line.startsWith(QLatin1Char('['))){
   inDesktopEntry=(line==QStringLiteral("[Desktop Entry]"));
   continue;
  }
  if(!inDesktopEntry)continue;
  const qsizetype separator=line.indexOf(QLatin1Char('='));
  if(separator<=0)continue;
  const QString key=line.left(separator);
  const QString value=line.mid(separator+1);
  if(key==QStringLiteral("Name"))result.name=value;
  else if(key==QStringLiteral("Comment"))result.comment=value;
  else if(key==QStringLiteral("Exec"))result.exec=value.trimmed();
  else if(key==QStringLiteral("TryExec"))result.tryExec=value.trimmed();
  else if(key==QStringLiteral("Path"))result.workingDirectory=value.trimmed();
  else if(key==QStringLiteral("Hidden"))result.hidden=(value.compare(QStringLiteral("true"),Qt::CaseInsensitive)==0);
  else if(key==QStringLiteral("NoDisplay"))result.noDisplay=(value.compare(QStringLiteral("true"),Qt::CaseInsensitive)==0);
  else if(key==QStringLiteral("Terminal"))result.terminal=(value.compare(QStringLiteral("true"),Qt::CaseInsensitive)==0);
 }
 return result;
}

bool DesktopEntryParser::splitExec(const QString &exec,QString *program,QStringList *arguments,QString *error) {
 if(program)program->clear();
 if(arguments)arguments->clear();
 auto fail=[error](const QString &message){if(error)*error=message;return false;};
 QStringList parts=QProcess::splitCommand(exec.trimmed());
 if(parts.isEmpty())return fail(QStringLiteral("Empty launch command."));
 QString executable=parts.takeFirst();
 if(executable.isEmpty()||executable.contains(QLatin1Char('%'))||executable.startsWith(QLatin1Char('-')))
  return fail(QStringLiteral("Unsupported launch command."));
 parts.erase(std::remove_if(parts.begin(),parts.end(),[](const QString &arg){
  return arg.startsWith(QLatin1Char('%'))||arg.contains(QLatin1Char('%'));
 }),parts.end());
 if(program)*program=executable;
 if(arguments)*arguments=parts;
 if(error)error->clear();
 return true;
}
