#include "file_manager.h"
#include <QDir>
#include <QDesktopServices>
#include <QFileInfo>
#include <QFile>
#include <QUrl>
FileManager::FileManager(QObject *parent):QObject(parent){
 QString home=QDir::homePath();QFileInfo info(home);m_currentPath=info.canonicalFilePath().isEmpty()?info.absoluteFilePath():info.canonicalFilePath();refresh();
}
QString FileManager::currentPath()const{return m_currentPath;}
QVariantList FileManager::entries()const{return m_entries;}
QString FileManager::status()const{return m_status;}
bool FileManager::canGoUp()const{return QDir(m_currentPath).absolutePath()!=QDir::rootPath();}
void FileManager::setStatus(const QString&s){if(m_status==s)return;m_status=s;emit statusChanged();}
bool FileManager::navigate(const QString&path){
 QFileInfo info(path);
 if(!info.exists()||!info.isDir()||!info.isReadable()){setStatus(QStringLiteral("Cannot open directory: %1").arg(path));return false;}
 QString canonical=info.canonicalFilePath();if(canonical.isEmpty())canonical=info.absoluteFilePath();
 if(m_currentPath!=canonical){m_currentPath=canonical;emit currentPathChanged();}
 refresh();return true;
}
bool FileManager::goUp(){QDir parent(m_currentPath);if(!parent.cdUp()){setStatus(QStringLiteral("Already at the top of this directory tree."));return false;}return navigate(parent.absolutePath());}
void FileManager::refresh(){
 QDir dir(m_currentPath);
 if(!dir.exists()){setStatus(QStringLiteral("Directory no longer exists."));return;}
 QVariantList next;const QFileInfoList list=dir.entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::Hidden|QDir::System,QDir::DirsFirst|QDir::Name|QDir::IgnoreCase);
 for(const QFileInfo&info:list){
  if(next.size()>=1000)break;
  QString path=info.absoluteFilePath();QString size;
  if(info.isDir())size=QStringLiteral("Folder");
  else{const qint64 bytes=info.size();if(bytes<1024)size=QStringLiteral("%1 B").arg(bytes);else if(bytes<1024*1024)size=QStringLiteral("%1 KiB").arg(bytes/1024.0,0,'f',1);else size=QStringLiteral("%1 MiB").arg(bytes/(1024.0*1024.0),0,'f',1);}
  next.append(QVariantMap{{QStringLiteral("name"),info.fileName()},{QStringLiteral("path"),path},{QStringLiteral("isDir"),info.isDir()},{QStringLiteral("sizeText"),size},{QStringLiteral("modified"),info.lastModified().toString(QStringLiteral("yyyy-MM-dd HH:mm"))},{QStringLiteral("readable"),info.isReadable()}});
 }
 m_entries=next;emit entriesChanged();setStatus(QStringLiteral("%1 entries in %2").arg(m_entries.size()).arg(m_currentPath));
}

bool FileManager::isSafeName(const QString &name) const {
 if(name.isEmpty()||name==QStringLiteral(".")||name==QStringLiteral(".."))return false;
 return !name.contains(QLatin1Char('/'))&&!name.contains(QLatin1Char('\\'))&&!name.contains(QChar::Null);
}
bool FileManager::isDirectChild(const QString &path,QFileInfo *result) const {
 QFileInfo info(path);
 if(!info.exists()||info.fileName().isEmpty())return false;
 const QString parent=QFileInfo(info.absolutePath()).canonicalFilePath();
 const QString current=QFileInfo(m_currentPath).canonicalFilePath();
 if(parent.isEmpty()||current.isEmpty()||parent!=current)return false;
 if(result)*result=info;
 return true;
}
bool FileManager::createFolder(const QString &name){
 if(!isSafeName(name)){setStatus(QStringLiteral("Folder name is invalid."));return false;}
 QDir dir(m_currentPath);
 if(dir.exists(name)){setStatus(QStringLiteral("An entry with that name already exists."));return false;}
 if(!dir.mkdir(name)){setStatus(QStringLiteral("Could not create folder. Check permissions and available storage."));return false;}
 refresh();setStatus(QStringLiteral("Created folder: %1").arg(name));return true;
}
bool FileManager::renameEntry(const QString &path,const QString &newName){
 QFileInfo info;
 if(!isDirectChild(path,&info)||!isSafeName(newName)){setStatus(QStringLiteral("Rename rejected: invalid entry or name."));return false;}
 if(info.fileName()==newName){setStatus(QStringLiteral("The name is unchanged."));return true;}
 QDir dir(m_currentPath);
 if(dir.exists(newName)){setStatus(QStringLiteral("An entry with that name already exists."));return false;}
 if(!dir.rename(info.fileName(),newName)){setStatus(QStringLiteral("Could not rename entry. Check permissions."));return false;}
 refresh();setStatus(QStringLiteral("Renamed to: %1").arg(newName));return true;
}
bool FileManager::deleteEntry(const QString &path,bool confirmDirectoryRemoval){
 QFileInfo info;
 if(!isDirectChild(path,&info)){setStatus(QStringLiteral("Delete rejected: entry is not a direct child of the current directory."));return false;}
 bool removed=false;
 if(info.isSymLink()||!info.isDir())removed=QFile::remove(info.absoluteFilePath());
 else{
  if(!confirmDirectoryRemoval){setStatus(QStringLiteral("Directory removal requires explicit confirmation."));return false;}
  QDir dir(info.absoluteFilePath());
  removed=dir.removeRecursively();
 }
 if(!removed){setStatus(QStringLiteral("Could not delete entry. Check permissions or whether it is in use."));return false;}
 refresh();setStatus(QStringLiteral("Entry deleted."));return true;
}

bool FileManager::openEntry(const QString&path){
 QFileInfo info(path);
 if(!info.exists()){setStatus(QStringLiteral("Entry no longer exists."));refresh();return false;}
 if(info.isDir())return navigate(info.absoluteFilePath());
 if(!info.isReadable()){setStatus(QStringLiteral("File is not readable."));return false;}
 const QUrl url=QUrl::fromLocalFile(info.absoluteFilePath());
 if(QDesktopServices::openUrl(url)){setStatus(QStringLiteral("Requested the host desktop to open %1.").arg(info.fileName()));return true;}
 setStatus(QStringLiteral("The host desktop could not open this file."));return false;
}
