#include "file_manager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QDebug>
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
 if(!expect(manager.entries().size()==2,"list folder and file"))return 1;
 bool foundFolder=false,foundFile=false;
 for(const QVariant &entry:manager.entries()){
  const QVariantMap item=entry.toMap();
  if(item.value(QStringLiteral("name")).toString()==QStringLiteral("Folder"))foundFolder=item.value(QStringLiteral("isDir")).toBool();
  if(item.value(QStringLiteral("name")).toString()==QStringLiteral("sample.txt"))foundFile=!item.value(QStringLiteral("isDir")).toBool()&&item.value(QStringLiteral("sizeText")).toString()==QStringLiteral("5 B");
 }
 if(!expect(foundFolder,"folder metadata")||!expect(foundFile,"file metadata"))return 1;
 const QString before=manager.currentPath();
 if(!expect(!manager.navigate(temporary.filePath(QStringLiteral("missing"))),"reject missing directory"))return 1;
 if(!expect(manager.currentPath()==before,"failed navigation preserves current path"))return 1;
 if(!expect(manager.goUp(),"navigate to parent"))return 1;
 if(!expect(manager.currentPath()!=before,"parent path changed"))return 1;
 qInfo()<<"File manager tests passed";return 0;
}
