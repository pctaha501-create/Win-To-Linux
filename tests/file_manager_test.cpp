#include "file_manager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QDebug>
#include <sys/stat.h>
namespace { bool expect(bool ok,const char *message){if(!ok)qCritical().noquote()<<"FAIL:"<<message;return ok;} }
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);
 QTemporaryDir temporary;
 if(!expect(temporary.isValid(),"temporary directory available"))return 1;
 QDir root(temporary.path());
 if(!expect(root.mkdir(QStringLiteral("Folder")),"create fixture folder"))return 1;
 QFile file(temporary.filePath(QStringLiteral("sample.txt")));
 if(!expect(file.open(QIODevice::WriteOnly),"create fixture file"))return 1;
 file.write("hello");file.close();
 FileManager manager;
 if(!expect(manager.navigate(temporary.path()),"navigate to readable directory"))return 1;
 if(!expect(manager.goHome(),"navigate to user home directory"))return 1;
 if(!expect(manager.currentPath()==QFileInfo(QDir::homePath()).canonicalFilePath(),"home path is canonical"))return 1;
 if(!expect(manager.navigate(temporary.path()),"return to test fixture directory"))return 1;
 if(!expect(manager.entries().size()==2,"list folder and file"))return 1;
 bool foundFolder=false,foundFile=false;
 for(const QVariant &entry:manager.entries()){
  const QVariantMap item=entry.toMap();
  if(item.value(QStringLiteral("name")).toString()==QStringLiteral("Folder"))foundFolder=item.value(QStringLiteral("isDir")).toBool();
  if(item.value(QStringLiteral("name")).toString()==QStringLiteral("sample.txt"))foundFile=!item.value(QStringLiteral("isDir")).toBool()&&item.value(QStringLiteral("sizeText")).toString()==QStringLiteral("5 B");
 }
 if(!expect(foundFolder,"folder metadata")||!expect(foundFile,"file metadata"))return 1;

 if(!expect(manager.createFolder(QStringLiteral("New Folder")),"create folder"))return 1;
 if(!expect(QDir(temporary.path()).exists(QStringLiteral("New Folder")),"created folder exists"))return 1;
 if(!expect(!manager.createFolder(QStringLiteral("../escape")),"reject path traversal folder name"))return 1;
 if(!expect(!manager.createFolder(QStringLiteral("bad\\\\name")),"reject backslash in folder name"))return 1;
 const QString samplePath=temporary.filePath(QStringLiteral("sample.txt"));
 if(!expect(manager.renameEntry(samplePath,QStringLiteral("renamed.txt")),"rename file"))return 1;
 if(!expect(QFileInfo::exists(temporary.filePath(QStringLiteral("renamed.txt"))),"renamed file exists"))return 1;
 if(!expect(!manager.renameEntry(temporary.filePath(QStringLiteral("renamed.txt")),QStringLiteral("New Folder")),"reject rename collision"))return 1;
 if(!expect(!manager.renameEntry(QDir::tempPath(),QStringLiteral("outside-renamed")),"reject renaming a path outside current directory"))return 1;

 if(!expect(root.mkdir(QStringLiteral("Destination")),"create destination folder"))return 1;
 QFile nested(temporary.filePath(QStringLiteral("Folder/nested.txt")));
 if(!expect(nested.open(QIODevice::WriteOnly),"create nested file"))return 1;
 nested.write("nested");nested.close();
 if(!expect(manager.copyEntry(temporary.filePath(QStringLiteral("Folder")),temporary.filePath(QStringLiteral("Destination"))),"copy directory tree"))return 1;
 if(!expect(QFileInfo::exists(temporary.filePath(QStringLiteral("Destination/Folder/nested.txt"))),"copied nested file exists"))return 1;
 if(!expect(!manager.copyEntry(temporary.filePath(QStringLiteral("Folder")),temporary.filePath(QStringLiteral("Destination"))),"reject overwriting copy destination"))return 1;
 if(!expect(manager.moveEntry(temporary.filePath(QStringLiteral("renamed.txt")),temporary.filePath(QStringLiteral("Destination"))),"move file"))return 1;
 if(!expect(!QFileInfo::exists(temporary.filePath(QStringLiteral("renamed.txt")))&&QFileInfo::exists(temporary.filePath(QStringLiteral("Destination/renamed.txt"))),"moved file path updated"))return 1;
 if(!expect(!manager.moveEntry(temporary.filePath(QStringLiteral("Folder")),temporary.filePath(QStringLiteral("Folder"))),"reject moving folder into itself"))return 1;
 // Exercise the EXDEV fallback when /dev/shm is a distinct writable filesystem.
 QTemporaryDir crossFilesystem(QStringLiteral("/dev/shm/wtl-move-XXXXXX"));
 if(crossFilesystem.isValid()){
  struct stat sourceStat{},destinationStat{};
  if(::stat(temporary.path().toLocal8Bit().constData(),&sourceStat)==0&&::stat(crossFilesystem.path().toLocal8Bit().constData(),&destinationStat)==0&&sourceStat.st_dev!=destinationStat.st_dev){
   QFile crossSource(temporary.filePath(QStringLiteral("cross-device.txt")));
   if(!expect(crossSource.open(QIODevice::WriteOnly),"create cross-filesystem move source"))return 1;crossSource.write("cross-device");crossSource.close();
   if(!expect(manager.moveEntry(crossSource.fileName(),crossFilesystem.path()),"move file across filesystems"))return 1;
   if(!expect(!QFileInfo::exists(crossSource.fileName())&&QFileInfo::exists(crossFilesystem.filePath(QStringLiteral("cross-device.txt"))),"cross-filesystem move removes source only after destination exists"))return 1;
   QFile moved(crossFilesystem.filePath(QStringLiteral("cross-device.txt")));if(!expect(moved.open(QIODevice::ReadOnly),"open cross-filesystem destination"))return 1;
   if(!expect(moved.readAll()==QByteArray("cross-device"),"cross-filesystem move preserves file contents"))return 1;moved.close();
  }
 }

 if(!expect(root.mkdir(QStringLiteral("SymlinkFolder")),"create symlink fixture folder"))return 1;
 if(!expect(QFile::link(temporary.filePath(QStringLiteral("Destination/renamed.txt")),temporary.filePath(QStringLiteral("SymlinkFolder/link.txt"))),"create symlink fixture"))return 1;
 if(!expect(!manager.copyEntry(temporary.filePath(QStringLiteral("SymlinkFolder")),temporary.filePath(QStringLiteral("Destination"))),"reject copying directory containing symlink"))return 1;
 if(!expect(!QFileInfo::exists(temporary.filePath(QStringLiteral("Destination/SymlinkFolder"))),"failed symlink copy leaves no partial destination"))return 1;


 if(!expect(!manager.deleteEntry(temporary.filePath(QStringLiteral("Folder"))),"directory deletion requires confirmation"))return 1;
 if(!expect(manager.deleteEntry(temporary.filePath(QStringLiteral("Folder")),true),"confirmed directory deletion"))return 1;
 if(!expect(!QDir(temporary.path()).exists(QStringLiteral("Folder")),"confirmed folder was removed"))return 1;
 if(!expect(!manager.deleteEntry(temporary.path()),"reject deleting current directory itself"))return 1;

 const QString before=manager.currentPath();
 if(!expect(!manager.navigate(temporary.filePath(QStringLiteral("missing"))),"reject missing directory"))return 1;
 if(!expect(manager.currentPath()==before,"failed navigation preserves current path"))return 1;
 if(!expect(manager.goUp(),"navigate to parent"))return 1;
 if(!expect(manager.currentPath()!=before,"parent path changed"))return 1;
 qInfo()<<"File manager tests passed";return 0;
}
