#include "desktop_entry_parser.h"
#include <QProcess>
#include <QLocale>
#include <QMap>
#include <algorithm>

DesktopEntryInfo DesktopEntryParser::parse(const QString &contents, const QString &requestedLocale) {
 DesktopEntryInfo result;
 QMap<QString,QString> localizedNames,localizedComments;
 QString baseName,baseComment;
 const QString localeName=requestedLocale.isEmpty()?QLocale::system().name():requestedLocale;
 const QString language=localeName.section(QLatin1Char('_'),0,0).section(QLatin1Char('@'),0,0);
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
  if(key==QStringLiteral("Type"))result.type=value.trimmed();
  else if(key==QStringLiteral("Name"))baseName=value;
  else if(key.startsWith(QStringLiteral("Name["))&&key.endsWith(QLatin1Char(']')))localizedNames.insert(key.mid(5,key.size()-6),value);
  else if(key==QStringLiteral("Comment"))baseComment=value;
  else if(key.startsWith(QStringLiteral("Comment["))&&key.endsWith(QLatin1Char(']')))localizedComments.insert(key.mid(8,key.size()-9),value);
  else if(key==QStringLiteral("Exec"))result.exec=value.trimmed();
  else if(key==QStringLiteral("TryExec"))result.tryExec=value.trimmed();
  else if(key==QStringLiteral("Path"))result.workingDirectory=value.trimmed();
  else if(key==QStringLiteral("Icon"))result.icon=value.trimmed();
  else if(key==QStringLiteral("Hidden"))result.hidden=(value.compare(QStringLiteral("true"),Qt::CaseInsensitive)==0);
  else if(key==QStringLiteral("NoDisplay"))result.noDisplay=(value.compare(QStringLiteral("true"),Qt::CaseInsensitive)==0);
  else if(key==QStringLiteral("Terminal"))result.terminal=(value.compare(QStringLiteral("true"),Qt::CaseInsensitive)==0);
 }
 result.name=localizedNames.value(localeName,localizedNames.value(language,baseName));
 result.comment=localizedComments.value(localeName,localizedComments.value(language,baseComment));
 return result;
}

bool DesktopEntryParser::splitExec(const QString &exec,QString *program,QStringList *arguments,QString *error,
                                  const QString &applicationName,const QString &iconName,const QString &desktopFile) {
 if(program)program->clear();
 if(arguments)arguments->clear();
 auto fail=[error](const QString &message){if(error)*error=message;return false;};
 QStringList parts=QProcess::splitCommand(exec.trimmed());
 if(parts.isEmpty())return fail(QStringLiteral("Empty launch command."));
 QString executable=parts.takeFirst();
 if(executable.isEmpty()||executable.contains(QLatin1Char('%'))||executable.startsWith(QLatin1Char('-')))
  return fail(QStringLiteral("Unsupported launch command."));
 QStringList expanded;
 for(const QString &arg:parts){
  if(arg==QStringLiteral("%f")||arg==QStringLiteral("%F")||arg==QStringLiteral("%u")||arg==QStringLiteral("%U"))continue;
  if(arg==QStringLiteral("%i")){if(!iconName.isEmpty())expanded<<QStringLiteral("--icon")<<iconName;continue;}
  QString value;value.reserve(arg.size());
  for(qsizetype i=0;i<arg.size();++i){
   if(arg.at(i)!=QLatin1Char('%')){value+=arg.at(i);continue;}
   if(i+1>=arg.size())return fail(QStringLiteral("Exec contains an incomplete field code."));
   const QChar code=arg.at(++i);
   if(code==QLatin1Char('%'))value+=QLatin1Char('%');
   else if(code==QLatin1Char('c'))value+=applicationName;
   else if(code==QLatin1Char('k'))value+=desktopFile;
   else return fail(QStringLiteral("Unsupported or misplaced desktop-entry field code: %1").arg(code));
  }
  if(!value.isEmpty())expanded<<value;
 }
 if(program)*program=executable;
 if(arguments)*arguments=expanded;
 if(error)error->clear();
 return true;
}
