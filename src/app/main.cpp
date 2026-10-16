#include "system_info.h"
#include "process_monitor.h"
#include "package_operation_controller.h"
#include "file_manager.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QDebug>
#include <QWindow>
#include <QTimer>
#include <QKeyEvent>
#include <QTemporaryDir>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <cstring>
namespace {
bool hasArgument(int argc,char **argv,const char *wanted){for(int i=1;i<argc;++i)if(std::strcmp(argv[i],wanted)==0)return true;return false;}
void configureApplication(){QCoreApplication::setApplicationName("Win-To-Linux");QCoreApplication::setApplicationVersion(QStringLiteral(WTL_VERSION));QCoreApplication::setOrganizationName("Win-To-Linux");}
void printSystemInfo(const SystemInfo &system){
 qInfo().noquote()<<"Distribution:"<<system.distribution();
 qInfo().noquote()<<"ID:"<<system.distroId();
 qInfo().noquote()<<"Family:"<<system.family();
 qInfo().noquote()<<"Version:"<<system.version();
 qInfo().noquote()<<"Package manager:"<<system.packageManager();
 qInfo().noquote()<<"Manager available:"<<(system.managerAvailable()?"yes":"no");
 qInfo().noquote()<<"Kernel:"<<system.kernel();
 qInfo().noquote()<<"Architecture:"<<system.architecture();
 qInfo().noquote()<<"Native tools:"<<system.nativeTools().size();
 qInfo().noquote()<<"Session type:"<<system.sessionType();
 qInfo().noquote()<<"Desktop environment:"<<system.desktopEnvironment();
 qInfo().noquote()<<"Wayland display:"<<system.waylandDisplay();
 qInfo().noquote()<<"X11 display:"<<system.xDisplay();
 qInfo().noquote()<<"Runtime directory:"<<system.runtimeDirectory();
 qInfo().noquote()<<"Runtime directory check:"<<system.runtimeDirectoryStatus();
 qInfo().noquote()<<"Compositors found:"<<system.availableCompositors().size();
 for(const QVariant &entry:system.availableCompositors())qInfo().noquote()<<"Compositor:"<<entry.toMap().value(QStringLiteral("name")).toString()<<entry.toMap().value(QStringLiteral("path")).toString();
}
}
int main(int argc,char**argv){
 if(hasArgument(argc,argv,"--version")||hasArgument(argc,argv,"--system")||hasArgument(argc,argv,"--session-check")||hasArgument(argc,argv,"--diagnostic-report")||hasArgument(argc,argv,"--preview-check")||hasArgument(argc,argv,"--standalone-check")){
  QCoreApplication app(argc,argv);configureApplication();
  if(hasArgument(argc,argv,"--version")){qInfo().noquote()<<QCoreApplication::applicationName()<<QCoreApplication::applicationVersion();return 0;}
  SystemInfo system;
  if(hasArgument(argc,argv,"--diagnostic-report"))qInfo().noquote()<<system.diagnosticReport();
  else if(hasArgument(argc,argv,"--standalone-check")){const QString readiness=system.standaloneReadiness();qInfo().noquote()<<readiness;return readiness.startsWith(QStringLiteral("Ready for experimental standalone session"))?0:2;}
  else if(hasArgument(argc,argv,"--preview-check")){
   const QString readiness=system.previewReadiness();qInfo().noquote()<<readiness;
   return readiness.startsWith(QStringLiteral("Ready for nested preview"))?0:2;
  }else printSystemInfo(system);
  return 0;
 }
 QGuiApplication app(argc,argv);configureApplication();
 SystemInfo system;ProcessMonitor processMonitor;PackageOperationController packageOperations;FileManager fileManager;
 QQmlApplicationEngine engine;
 engine.rootContext()->setContextProperty("systemInfo",&system);
 engine.rootContext()->setContextProperty("processMonitor",&processMonitor);
 engine.rootContext()->setContextProperty("packageOperations",&packageOperations);
 engine.rootContext()->setContextProperty("fileManager",&fileManager);
 QObject::connect(&engine,&QQmlApplicationEngine::objectCreationFailed,&app,[](){QCoreApplication::exit(1);},Qt::QueuedConnection);
 engine.load(QUrl("qrc:/qml/Main.qml"));
 if(engine.rootObjects().isEmpty()){qCritical()<<"Unable to load Win-To-Linux shell.";return 1;}
 if(hasArgument(argc,argv,"--smoke-test")){
  QObject *root=engine.rootObjects().constFirst();
   if(root->findChild<QObject *>(QStringLiteral("previousWorkspaceShortcut"))){qCritical()<<"Win-To-Linux must not intercept Alt+Tab from the native window manager.";return 1;}
  root->setProperty("width",820);root->setProperty("height",560);QCoreApplication::processEvents();
  const QStringList requiredControls{
   QStringLiteral("workspaceSwitcher"),QStringLiteral("workspaceSwitcherShortcut"),
   QStringLiteral("startMenuPopup"),QStringLiteral("notificationCenterPopup"),QStringLiteral("quickSettingsPopup"),
   QStringLiteral("taskbarStartButton"),QStringLiteral("taskbarSearchButton"),QStringLiteral("taskbarLinuxButton"),
   QStringLiteral("taskbarPackagesButton"),QStringLiteral("taskbarFilesButton"),QStringLiteral("taskbarSettingsButton"),
   QStringLiteral("taskbarNotificationsButton"),QStringLiteral("taskbarQuickSettingsButton"),QStringLiteral("launchNestedPreviewButton")};
  for(const QString &name:requiredControls){
   if(!root->findChild<QObject *>(name)){qCritical()<<"Required shell/accessibility control was not instantiated:"<<name;return 1;}
  }
  auto clickButton=[&](const QString &buttonName)->bool{
   QObject *button=root->findChild<QObject *>(buttonName);
   if(!button||!QMetaObject::invokeMethod(button,"clicked",Qt::DirectConnection)){
    qCritical()<<"Could not trigger shell button:"<<buttonName;return false;
   }
   QCoreApplication::processEvents();return true;
  };
  auto popupVisible=[&](const QString &popupName)->bool{
   QObject *popup=root->findChild<QObject *>(popupName);
   return popup&&popup->property("visible").toBool();
  };
  auto closePopup=[&](const QString &popupName)->bool{
   QObject *popup=root->findChild<QObject *>(popupName);
   if(!popup||!QMetaObject::invokeMethod(popup,"close",Qt::DirectConnection))return false;
   QCoreApplication::processEvents();return true;
  };
  auto popupFitsWindow=[&](const QString &popupName)->bool{
   QObject *popup=root->findChild<QObject *>(popupName);if(!popup)return false;
   return popup->property("width").toReal()<=root->property("width").toReal()-16.0&&popup->property("height").toReal()<=root->property("height").toReal()-70.0;
  };
  QVariant launchResult;
  if(!QMetaObject::invokeMethod(root,"launchDesktopEntry",Qt::DirectConnection,Q_RETURN_ARG(QVariant,launchResult),Q_ARG(QVariant,QVariant(QStringLiteral("/invalid.desktop"))))||launchResult.toBool()||root->property("toastMessage").toString().isEmpty()){qCritical()<<"Failed application launch did not report visible UI feedback.";return 1;}
  if(!clickButton(QStringLiteral("taskbarStartButton"))||!popupVisible(QStringLiteral("startMenuPopup"))||!popupFitsWindow(QStringLiteral("startMenuPopup"))){qCritical()<<"Start menu interaction or responsive sizing failed.";return 1;}
  QObject *startScroll=root->findChild<QObject *>(QStringLiteral("startMenuScroll"));
  if(!startScroll||startScroll->property("contentHeight").toReal()<=startScroll->property("height").toReal()){qCritical()<<"Start menu content is not scrollable at minimum window height.";return 1;}
  if(!closePopup(QStringLiteral("startMenuPopup"))){qCritical()<<"Could not close Start menu after responsive-layout test.";return 1;}
  if(!clickButton(QStringLiteral("taskbarStartButton"))){qCritical()<<"Could not reopen Start for section search test.";return 1;}
  QObject *startSearchField=root->findChild<QObject *>(QStringLiteral("startSearchField"));
  if(!startSearchField){qCritical()<<"Start search field is missing.";return 1;}
  root->setProperty("startSearch",QStringLiteral("settings"));QCoreApplication::processEvents();
  if(startSearchField->property("text").toString()!=QStringLiteral("settings")){qCritical()<<"Start search field did not follow the shared search state.";return 1;}
  QObject *settingsSearchResult=root->findChild<QObject *>(QStringLiteral("startSearchSection_4"));
  if(!settingsSearchResult||!settingsSearchResult->property("visible").toBool()){qCritical()<<"Start search did not show the Settings section shortcut.";return 1;}
  if(!clickButton(QStringLiteral("startSearchSection_4"))||root->property("page").toInt()!=4||popupVisible(QStringLiteral("startMenuPopup"))){qCritical()<<"Selecting Settings from Start search failed.";return 1;}
  if(!clickButton(QStringLiteral("taskbarStartButton"))){qCritical()<<"Could not reopen Start for terminal search test.";return 1;}
  root->setProperty("startSearch",QStringLiteral("terminal"));QCoreApplication::processEvents();
  QObject *terminalSearchResult=root->findChild<QObject *>(QStringLiteral("startSearchSection_-1"));
  if(!terminalSearchResult||!terminalSearchResult->property("visible").toBool()){qCritical()<<"Start search did not show the Terminal action.";return 1;}
  if(!closePopup(QStringLiteral("startMenuPopup"))){qCritical()<<"Could not close Start after terminal search test.";return 1;}
  if(!clickButton(QStringLiteral("taskbarNotificationsButton"))||!popupVisible(QStringLiteral("notificationCenterPopup"))||!popupFitsWindow(QStringLiteral("notificationCenterPopup"))||!closePopup(QStringLiteral("notificationCenterPopup"))){qCritical()<<"Notifications button interaction or responsive sizing failed.";return 1;}
  if(!clickButton(QStringLiteral("taskbarQuickSettingsButton"))||!popupVisible(QStringLiteral("quickSettingsPopup"))||!popupFitsWindow(QStringLiteral("quickSettingsPopup"))||!closePopup(QStringLiteral("quickSettingsPopup"))){qCritical()<<"Quick settings button interaction or responsive sizing failed.";return 1;}
  const QList<QPair<QString,int>> navigationButtons{{QStringLiteral("taskbarLinuxButton"),2},{QStringLiteral("taskbarPackagesButton"),3},{QStringLiteral("taskbarFilesButton"),7},{QStringLiteral("taskbarSettingsButton"),4}};
  for(const auto &entry:navigationButtons){
   if(!clickButton(entry.first)||root->property("page").toInt()!=entry.second){qCritical()<<"Taskbar navigation failed:"<<entry.first;return 1;}
  }
  auto *shellWindow=qobject_cast<QWindow *>(root);
  auto sendShortcut=[&](Qt::Key key,Qt::KeyboardModifiers modifiers)->bool{
   if(!shellWindow)return false;
   QKeyEvent press(QEvent::KeyPress,key,modifiers);
   QCoreApplication::sendEvent(shellWindow,&press);
   QKeyEvent release(QEvent::KeyRelease,key,modifiers);
   QCoreApplication::sendEvent(shellWindow,&release);
   QCoreApplication::processEvents();return true;
  };
  if(!sendShortcut(Qt::Key_Space,Qt::ControlModifier)||!popupVisible(QStringLiteral("startMenuPopup"))||!closePopup(QStringLiteral("startMenuPopup"))){qCritical()<<"Ctrl+Space Start shortcut failed.";return 1;}
  if(!sendShortcut(Qt::Key_N,Qt::ControlModifier|Qt::ShiftModifier)||!popupVisible(QStringLiteral("notificationCenterPopup"))||!closePopup(QStringLiteral("notificationCenterPopup"))){qCritical()<<"Ctrl+Shift+N notification shortcut failed.";return 1;}
  if(!sendShortcut(Qt::Key_Comma,Qt::ControlModifier)||root->property("page").toInt()!=4){qCritical()<<"Ctrl+, Settings shortcut failed.";return 1;}
  if(!sendShortcut(Qt::Key_E,Qt::ControlModifier)||root->property("page").toInt()!=7){qCritical()<<"Ctrl+E File Explorer shortcut failed.";return 1;}
  if(!sendShortcut(Qt::Key_Escape,Qt::ControlModifier|Qt::ShiftModifier)||root->property("page").toInt()!=5){qCritical()<<"Ctrl+Shift+Esc Task Manager shortcut failed.";return 1;}
  if(!sendShortcut(Qt::Key_Tab,Qt::ControlModifier|Qt::AltModifier)||!popupVisible(QStringLiteral("workspaceSwitcher"))||!closePopup(QStringLiteral("workspaceSwitcher"))){qCritical()<<"Ctrl+Alt+Tab workspace switcher shortcut failed.";return 1;}
  const QStringList pageUrls{QStringLiteral("qrc:/qml/FileManagerPage.qml"),QStringLiteral("qrc:/qml/PackageManagerPage.qml")};
  for(const QString &url:pageUrls){
   QQmlComponent component(&engine,QUrl(url));
   if(component.isError()){
    qCritical().noquote()<<"QML component failed:"<<url;
    for(const QQmlError &error:component.errors())qCritical().noquote()<<error.toString();
    return 1;
   }
   QObject *page=component.create(engine.rootContext());
   if(!page){
    qCritical().noquote()<<"QML component could not be instantiated:"<<url;
    for(const QQmlError &error:component.errors())qCritical().noquote()<<error.toString();
    return 1;
   }
   if(url.endsWith(QStringLiteral("FileManagerPage.qml"))){
    if(!page->findChild<QObject *>(QStringLiteral("fileManagerPathField"))||!page->findChild<QObject *>(QStringLiteral("fileManagerFocusPathShortcut"))||!page->findChild<QObject *>(QStringLiteral("fileManagerRefreshShortcut"))){qCritical()<<"File Manager path field or Ctrl+L shortcut was not instantiated.";delete page;return 1;}
    QObject *homeButton=page->findChild<QObject *>(QStringLiteral("fileManagerHomeButton"));
    QObject *pathField=page->findChild<QObject *>(QStringLiteral("fileManagerPathField"));
    QObject *pathShortcut=page->findChild<QObject *>(QStringLiteral("fileManagerFocusPathShortcut"));
    QObject *refreshShortcut=page->findChild<QObject *>(QStringLiteral("fileManagerRefreshShortcut"));
    QTemporaryDir homeFixture;
    if(!homeButton||!pathField||!pathShortcut||!refreshShortcut||!homeFixture.isValid()||!fileManager.navigate(homeFixture.path())){qCritical()<<"File Manager navigation fixture could not be prepared.";delete page;return 1;}
    QFile newlyCreated(homeFixture.filePath(QStringLiteral("refreshed.txt")));
    if(!newlyCreated.open(QIODevice::WriteOnly)){qCritical()<<"Could not create File Manager refresh fixture.";delete page;return 1;}
    newlyCreated.write("refresh");newlyCreated.close();
    if(!QMetaObject::invokeMethod(refreshShortcut,"activated",Qt::DirectConnection)){qCritical()<<"File Manager F5 shortcut could not be triggered.";delete page;return 1;}
    QCoreApplication::processEvents();
    bool refreshFound=false;for(const QVariant &entry:fileManager.entries())if(entry.toMap().value(QStringLiteral("name")).toString()==QStringLiteral("refreshed.txt"))refreshFound=true;
    if(!refreshFound){qCritical()<<"F5 did not refresh the File Manager directory listing.";delete page;return 1;}
    if(!QMetaObject::invokeMethod(pathShortcut,"activated",Qt::DirectConnection)){qCritical()<<"File Manager Ctrl+L shortcut could not be triggered.";delete page;return 1;}
    QCoreApplication::processEvents();
    if(pathField->property("selectedText").toString()!=pathField->property("text").toString()){qCritical()<<"Ctrl+L did not select the full path in File Manager.";delete page;return 1;}
    if(!QMetaObject::invokeMethod(homeButton,"clicked",Qt::DirectConnection)){qCritical()<<"File Manager Home button could not be triggered.";delete page;return 1;}
    QCoreApplication::processEvents();
    const QString expectedHome=QFileInfo(QDir::homePath()).canonicalFilePath();
    if(fileManager.currentPath()!=expectedHome){qCritical()<<"File Manager Home button did not navigate to the canonical home directory.";delete page;return 1;}
   }
   delete page;
  }
  qInfo()<<"QML shell and dynamic pages loaded successfully.";return 0;
 }
 if(hasArgument(argc,argv,"--fullscreen")){
  if(auto *window=qobject_cast<QWindow *>(engine.rootObjects().constFirst()))window->showFullScreen();
  else qWarning()<<"Could not switch the shell window to full-screen mode.";
 }else if(hasArgument(argc,argv,"--maximized")){
  if(auto *window=qobject_cast<QWindow *>(engine.rootObjects().constFirst()))window->showMaximized();
  else qWarning()<<"Could not maximize the shell window.";
 }
 if(hasArgument(argc,argv,"--startup-smoke-test"))QTimer::singleShot(3000,&app,&QCoreApplication::quit);
 return app.exec();
}
