import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
ApplicationWindow {
 id: win; visible: true; width: 1120; height: 720; minimumWidth: 820; minimumHeight: 560
 title: "Win-To-Linux"; color: light ? "#f1f4f8" : "#10141b"
 property int page: 0
 property bool light: systemInfo.lightAppearance
 property string search: ""
 property var selectedProcess: ({})
 property string pendingWindowsFile: ""
 property string startSearch: ""
 property string toastMessage: ""
 property var notifications: systemInfo.recentNotifications()
 function showToast(message) {
  toastMessage = message
  notifications = [{message: message, time: Qt.formatDateTime(new Date(), "hh:mm")}].concat(notifications).slice(0, 8)
  systemInfo.saveRecentNotifications(notifications)
  toastTimer.restart()
 }
 property string clockText: Qt.formatDateTime(new Date(), "ddd d MMM · hh:mm")
 readonly property color fg: light ? "#1a2432" : "#f4f6fb"
 readonly property color muted: light ? "#647084" : "#aab4c4"
 readonly property color panel: light ? "#ffffff" : "#191f29"
 readonly property color borderColor: light ? "#dce2eb" : "#2b3340"
 header: Rectangle {
  visible: win.page !== 0
  height: visible ? 62 : 0; color: win.panel; border.color: win.borderColor
  RowLayout { anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 20
   Rectangle { width: 34; height: 34; radius: 9; color: "#5b8cff"; Text { anchors.centerIn: parent; text: "W"; color: "white"; font.bold: true; font.pixelSize: 20 } }
   ColumnLayout { spacing: 0; Label { text: "Win-To-Linux"; color: win.fg; font.bold: true; font.pixelSize: 17 } Label { text: "Linux desktop workspace"; color: win.muted; font.pixelSize: 11 } }
   Item { Layout.fillWidth: true }
   Label { text: systemInfo.distribution; color: win.muted }
  }
 }
 Timer { interval: 30000; running: true; repeat: true; onTriggered: win.clockText = Qt.formatDateTime(new Date(), "ddd d MMM · hh:mm") }
 Timer { interval: 2000; running: win.page === 5; repeat: true; onTriggered: processMonitor.refreshProcesses() }
 Timer { id: toastTimer; interval: 3200; repeat: false; onTriggered: win.toastMessage = "" }
 Dialog {
  id: stopDialog; modal: true; title: "Stop process?"; standardButtons: Dialog.Ok | Dialog.Cancel; anchors.centerIn: parent
  Label { text: "Send SIGTERM to " + (win.selectedProcess.name || "process") + " (PID " + (win.selectedProcess.pid || "—") + ")? Unsaved work in that process may be lost."; wrapMode: Text.WordWrap; width: 330 }
  onAccepted: {
   if (processMonitor.terminateProcess(win.selectedProcess.pid, win.selectedProcess.startTicks))
    win.showToast("Stop signal sent to " + (win.selectedProcess.name || "process"))
   else win.showToast("Could not stop this process")
  }
 }
 Dialog {
  id: standaloneReadinessDialog; modal: true; title: "Standalone session readiness"; standardButtons: Dialog.Ok; anchors.centerIn: parent
  Label { text: systemInfo.standaloneReadiness(); wrapMode: Text.WordWrap; width: 460; color: win.fg }
 }
 Dialog {
  id: windowsDialog; modal: true; title: "Launch Windows program?"; standardButtons: Dialog.Ok | Dialog.Cancel; anchors.centerIn: parent
  Label { text: "Run this file through Wine?\n" + win.pendingWindowsFile + "\nOnly proceed if you trust the file. Wine compatibility is not guaranteed."; wrapMode: Text.Wrap; width: 380 }
  onAccepted: systemInfo.launchWindowsFile(win.pendingWindowsFile)
 }
 FileDialog {
  id: windowsFileDialog; title: "Select a Windows installer or program"; fileMode: FileDialog.OpenFile
  nameFilters: ["Windows programs (*.exe *.msi)", "All files (*)"]
  onAccepted: { win.pendingWindowsFile = selectedFile.toLocalFile(); windowsDialog.open() }
 }
 Popup {
  id: quickSettings; objectName: "quickSettingsPopup"
  width: Math.min(340, win.width - 16); height: Math.min(350, win.height - 70); modal: false; focus: true
  x: Math.max(8, win.width-width-12)
  y: Math.max(8, win.height-height-82)
  closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
  background: Rectangle { radius: 16; color: win.light ? "#f7f8fc" : "#20232b"; border.color: win.borderColor }
  contentItem: ColumnLayout {
   anchors.fill: parent; anchors.margins: 18; spacing: 12
   Label { text: "Quick settings"; color: win.fg; font.pixelSize: 18; font.bold: true }
   Rectangle {
    Layout.fillWidth: true; height: 74; radius: 11; color: win.panel; border.color: win.borderColor
    RowLayout {
     anchors.fill: parent; anchors.margins: 12; spacing: 10
     Rectangle { width: 38; height: 38; radius: 10; color: "#3978d4"; Label { anchors.centerIn: parent; text: win.light ? "☀" : "☾"; color: "white"; font.pixelSize: 20 } }
     ColumnLayout { Layout.fillWidth: true; spacing: 2; Label { text: "Light appearance"; color: win.fg; font.bold: true } Label { text: win.light ? "Light mode enabled" : "Dark mode enabled"; color: win.muted; font.pixelSize: 10 } }
     Switch { checked: systemInfo.lightAppearance; onToggled: systemInfo.lightAppearance = checked }
    }
   }
   Label { text: "Device information"; color: win.fg; font.bold: true }
   Rectangle {
    Layout.fillWidth: true; height: 82; radius: 10; color: win.panel; border.color: win.borderColor
    ColumnLayout { anchors.fill: parent; anchors.margins: 11; spacing: 4
     Label { text: systemInfo.distribution + " " + systemInfo.version; color: win.fg; font.pixelSize: 12; font.bold: true; elide: Text.ElideRight; Layout.fillWidth: true }
     Label { text: systemInfo.desktopEnvironment + " · " + systemInfo.sessionType; color: win.muted; font.pixelSize: 11; elide: Text.ElideRight; Layout.fillWidth: true }
     Label { text: systemInfo.kernel; color: win.muted; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true }
    }
   }
   RowLayout {
    Layout.fillWidth: true; spacing: 8
    Button { text: "Settings"; Layout.fillWidth: true; onClicked: { win.page=4; quickSettings.close() } }
    Button { text: "Applications"; Layout.fillWidth: true; onClicked: { win.page=1; systemInfo.refreshApplications(); quickSettings.close() } }
   }
   Item { Layout.fillHeight: true }
   Label { text: win.clockText; color: win.muted; font.pixelSize: 11; Layout.alignment: Qt.AlignRight }
  }
 }
 Popup {
  id: startMenu; objectName: "startMenuPopup"
  width: Math.min(560, win.width - 16); height: Math.min(590, win.height - 70); modal: true; focus: true
  x: Math.max(8, (win.width-width)/2)
  y: Math.max(8, win.height-height-55)
  closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
  background: Rectangle { radius: 18; color: win.light ? "#f7f8fc" : "#20232b"; border.color: win.borderColor }
  contentItem: Flickable {
   id: startMenuScroll; objectName: "startMenuScroll"; anchors.fill: parent; clip: true
   contentWidth: width; contentHeight: Math.max(startMenuColumn.implicitHeight + 48, startMenuColumn.childrenRect.height + 48, height + 1)
   boundsBehavior: Flickable.StopAtBounds
   ColumnLayout {
    id: startMenuColumn; x: 24; y: 24; width: Math.max(0, startMenuScroll.width - 48); height: implicitHeight; spacing: 13
   TextField {
    id: startSearchField; objectName: "startSearchField"
    placeholderText: "Search apps and Win-To-Linux sections"
    text: win.startSearch; onTextChanged: win.startSearch=text
    Layout.fillWidth: true; implicitHeight: 42
    background: Rectangle { radius: 7; color: win.panel; border.color: win.light ? "#9aa8bc" : "#586273" }
   }
   RowLayout {
    Layout.fillWidth: true
    Label { text: win.startSearch.length ? "Search results" : "Pinned"; color: win.fg; font.pixelSize: 17; font.bold: true }
    Item { Layout.fillWidth: true }
    Button { text: "All apps  ›"; onClicked: { win.page=1; startMenu.close(); systemInfo.refreshApplications() } }
   }
   GridLayout {
    visible: win.startSearch.length === 0
    Layout.fillWidth: true; columns: 4; columnSpacing: 8; rowSpacing: 6
    Repeater {
     model: [{label:"Explorer",glyph:"📁",page:7},{label:"Settings",glyph:"⚙",page:4},{label:"Terminal",glyph:"›_",page:2},{label:"Applications",glyph:"▦",page:1},{label:"Processes",glyph:"▥",page:5},{label:"Packages",glyph:"▤",page:3},{label:"Windows apps",glyph:"▣",page:6},{label:"Overview",glyph:"⌂",page:0}].filter(function(item){return item.label.toLowerCase().indexOf(win.startSearch.toLowerCase())>=0})
     delegate: Button {
      required property var modelData
      Layout.fillWidth: true; implicitHeight: 82; padding: 5
      background: Rectangle { radius: 8; color: parent.hovered ? (win.light ? "#e7ebf4" : "#343945") : "transparent" }
      contentItem: ColumnLayout {
       spacing: 4
       Label { text: modelData.glyph; color: win.fg; font.pixelSize: 24; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true }
       Label { text: modelData.label; color: win.fg; font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true; elide: Text.ElideRight }
      }
      onClicked: { win.page=modelData.page; startMenu.close(); if(win.page===1)systemInfo.refreshApplications(); if(win.page===5)processMonitor.refreshProcesses() }
     }
    }
   }
   Rectangle { Layout.fillWidth: true; height: 1; color: win.borderColor }
   RowLayout {
    Layout.fillWidth: true
    Label { text: win.startSearch.length ? "Installed app results" : "Recommended"; color: win.fg; font.pixelSize: 16; font.bold: true }
    Item { Layout.fillWidth: true }
    Button { visible: win.startSearch.length === 0; text: "More"; onClicked: { win.page=1; startMenu.close(); systemInfo.refreshApplications() } }
   }
   ColumnLayout {
    Layout.fillWidth: true; spacing: 2
    Repeater {
     model: systemInfo.applications.filter(function(app) {
      var query = win.startSearch.trim().toLowerCase()
      return query.length === 0 || app.name.toLowerCase().indexOf(query) !== -1 || app.comment.toLowerCase().indexOf(query) !== -1
     }).slice(0, win.startSearch.length ? 5 : 3)
     delegate: Button {
      required property var modelData
      Layout.fillWidth: true; implicitHeight: 43; flat: true
      background: Rectangle { radius: 7; color: parent.hovered ? (win.light ? "#e7ebf4" : "#343945") : "transparent" }
      contentItem: RowLayout {
       spacing: 10
       Rectangle { width: 30; height: 30; radius: 7; color: "#3978d4"; Label { anchors.centerIn: parent; text: modelData.name.charAt(0).toUpperCase(); color: "white"; font.bold: true } }
       ColumnLayout {
        spacing: 1
        Label { text: modelData.name; color: win.fg; font.pixelSize: 12; elide: Text.ElideRight; Layout.fillWidth: true }
        Label { text: modelData.comment; color: win.muted; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true }
       }
      }
      onClicked: { systemInfo.launchDesktopFile(modelData.desktopFile); startMenu.close() }
     }
    }
   }
   ColumnLayout {
    visible: win.startSearch.trim().length > 0
    Layout.fillWidth: true; spacing: 2
    Button {
     objectName: "startSearchSection_4"
     visible: win.startSearch.trim().toLowerCase().indexOf("settings") !== -1 || "appearance and diagnostics".indexOf(win.startSearch.trim().toLowerCase()) !== -1
     Layout.fillWidth: true; implicitHeight: 40; flat: true
     background: Rectangle { radius: 7; color: parent.hovered ? (win.light ? "#e7ebf4" : "#343945") : "transparent" }
     contentItem: RowLayout { spacing: 10
      Rectangle { width: 28; height: 28; radius: 7; color: "#315f9e"; Label { anchors.centerIn: parent; text: "⚙"; color: "white"; font.bold: true } }
      ColumnLayout { spacing: 0; Layout.fillWidth: true
       Label { text: "Settings"; color: win.fg; font.pixelSize: 12; Layout.fillWidth: true }
       Label { text: "Appearance and diagnostics"; color: win.muted; font.pixelSize: 10; Layout.fillWidth: true }
      }
     }
     onClicked: { win.page = 4; startMenu.close() }
    }
    Repeater {
     model: [{label:"Linux System",comment:"System and distribution details",page:2},{label:"Package Manager",comment:"Install or remove packages",page:3},{label:"Files",comment:"Browse folders and files",page:7},{label:"Processes",comment:"Task Manager and process list",page:5},{label:"Windows Apps",comment:"Run compatible EXE/MSI files",page:6}]
     delegate: Button {
      required property var modelData
      objectName: "startSearchSection_" + modelData.page
      visible: win.startSearch.trim().length > 0 && (modelData.label.toLowerCase().indexOf(win.startSearch.trim().toLowerCase()) !== -1 || modelData.comment.toLowerCase().indexOf(win.startSearch.trim().toLowerCase()) !== -1)
      Layout.fillWidth: true; implicitHeight: 40; flat: true
      background: Rectangle { radius: 7; color: parent.hovered ? (win.light ? "#e7ebf4" : "#343945") : "transparent" }
      contentItem: RowLayout {
       spacing: 10
       Rectangle { width: 28; height: 28; radius: 7; color: "#315f9e"; Label { anchors.centerIn: parent; text: "›"; color: "white"; font.bold: true } }
       ColumnLayout { spacing: 0; Layout.fillWidth: true
        Label { text: modelData.label; color: win.fg; font.pixelSize: 12; Layout.fillWidth: true }
        Label { text: modelData.comment; color: win.muted; font.pixelSize: 10; Layout.fillWidth: true }
       }
      }
      onClicked: { win.page = modelData.page; startMenu.close(); if(win.page===1)systemInfo.refreshApplications(); if(win.page===5)processMonitor.refreshProcesses() }
     }
    }
   }
   Label {
    visible: win.startSearch.trim().length > 0 && systemInfo.applications.filter(function(app) {
     var query = win.startSearch.trim().toLowerCase()
     return app.name.toLowerCase().indexOf(query) !== -1 || app.comment.toLowerCase().indexOf(query) !== -1
    }).length === 0
    text: "No installed apps matched. Try another search; section shortcuts may still be available."
    color: win.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true
   }
   Item { Layout.fillHeight: true }
   Rectangle {
    Layout.fillWidth: true; height: 46; radius: 9; color: win.light ? "#e9edf5" : "#292d36"
    RowLayout {
     anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10
     Rectangle { width: 30; height: 30; radius: 15; color: "#3978d4"; Label { anchors.centerIn: parent; text: "U"; color: "white"; font.bold: true } }
     Label { text: systemInfo.hostName || "Linux user"; color: win.fg; font.pixelSize: 12; Layout.fillWidth: true }
     Label { text: systemInfo.distribution; color: win.muted; font.pixelSize: 10 }
    }
   }
   }
  }
 }
 Shortcut { sequence: "Ctrl+Space"; onActivated: { startMenu.open(); Qt.callLater(function(){ startSearchField.forceActiveFocus() }) } }
 Shortcut { sequence: "Ctrl+Shift+N"; onActivated: notificationCenter.open() }
 Shortcut { sequence: "Ctrl+,"; onActivated: win.page = 4 }
 Shortcut { sequence: "F11"; onActivated: win.visibility = win.visibility === Window.FullScreen ? Window.Windowed : Window.FullScreen }
 Shortcut { sequence: "Ctrl+Shift+Esc"; onActivated: { win.page=5; processMonitor.refreshProcesses() } }
 Shortcut { sequence: "Ctrl+E"; onActivated: win.page=7 }
 Shortcut { objectName: "workspaceSwitcherShortcut"; sequence: "Ctrl+Alt+Tab"; onActivated: workspaceSwitcher.open() }
 Popup {
  id: workspaceSwitcher
  objectName: "workspaceSwitcher"
  width: 520; height: 330; modal: true; focus: true
  x: Math.max(8, (win.width-width)/2); y: Math.max(8, (win.height-height)/2)
  closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
  background: Rectangle { radius: 16; color: win.light ? "#f7f8fc" : "#20232b"; border.color: win.borderColor }
  contentItem: ColumnLayout {
   anchors.fill: parent; anchors.margins: 18; spacing: 12
   Label { text: "Section switcher"; color: win.fg; font.pixelSize: 18; font.bold: true }
   Label { text: "Choose a Win-To-Linux section. Use Alt+Tab for application windows in your desktop compositor."; color: win.muted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
   GridLayout { Layout.fillWidth: true; Layout.fillHeight: true; columns: 2; rowSpacing: 8; columnSpacing: 8
    Repeater { model: [{label:"Overview",glyph:"⌂"},{label:"Applications",glyph:"▦"},{label:"Linux System",glyph:"⌘"},{label:"Package Manager",glyph:"▤"},{label:"Settings",glyph:"⚙"},{label:"Processes",glyph:"▥"},{label:"Windows Apps",glyph:"▣"},{label:"Files",glyph:"📁"}]
     delegate: Button { required property int index; required property var modelData; Layout.fillWidth: true; Layout.fillHeight: true
      background: Rectangle { radius: 9; color: win.page===index ? (win.light?"#dbe6ff":"#2b3e60") : (parent.hovered?(win.light?"#e8edf5":"#252d39"):win.panel); border.color: win.borderColor }
      contentItem: RowLayout { spacing: 10; Label { text: modelData.glyph; color: win.fg; font.pixelSize: 21 } Label { text: modelData.label; color: win.fg; font.pixelSize: 12; Layout.fillWidth: true } Label { visible: win.page===index; text: "●"; color: "#5b8cff" } }
      onClicked: { win.page=index; if(index===1)systemInfo.refreshApplications(); if(index===5)processMonitor.refreshProcesses(); workspaceSwitcher.close() }
     }
    }
   }
  }
 }
 Popup {
  id: notificationCenter; objectName: "notificationCenterPopup"
  width: Math.min(360, win.width - 16); height: Math.min(390, win.height - 70); modal: false; focus: true
  x: Math.max(8, win.width - width - 12)
  y: Math.max(8, win.height - height - 82)
  closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
  background: Rectangle { radius: 16; color: win.light ? "#f7f8fc" : "#20232b"; border.color: win.borderColor }
  contentItem: ColumnLayout {
   anchors.fill: parent; anchors.margins: 16; spacing: 10
   RowLayout {
    Layout.fillWidth: true
    Label { text: "Notifications"; color: win.fg; font.pixelSize: 18; font.bold: true; Layout.fillWidth: true }
    Button { text: "Clear all"; enabled: win.notifications.length > 0; onClicked: { win.notifications = []; systemInfo.saveRecentNotifications([]); win.toastMessage = ""; toastTimer.stop() } }
   }
   Label { visible: win.notifications.length === 0; Layout.fillWidth: true; Layout.fillHeight: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; text: "You're all caught up"; color: win.muted }
   ListView {
    Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 6
    model: win.notifications
    delegate: Rectangle {
     required property var modelData
     width: ListView.view.width; height: notificationText.implicitHeight + 30; radius: 10
     color: win.panel; border.color: win.borderColor
     ColumnLayout {
      anchors.fill: parent; anchors.margins: 10; spacing: 4
      RowLayout { Layout.fillWidth: true; Label { text: "Win-To-Linux"; color: win.fg; font.bold: true; Layout.fillWidth: true } Label { text: modelData.time; color: win.muted; font.pixelSize: 10 } }
      Label { id: notificationText; Layout.fillWidth: true; text: modelData.message; color: win.muted; wrapMode: Text.WordWrap }
     }
    }
   }
  }
 }
 Rectangle {
  z: 1000
  visible: win.toastMessage.length > 0
  width: 340
  height: Math.max(52, toastLabel.implicitHeight + 24)
  x: win.width - width - 24
  y: win.height - height - 24
  radius: 12
  color: win.light ? "#ffffff" : "#252d3a"
  border.color: win.light ? "#cbd5e4" : "#46556b"
  Label { id: toastLabel; anchors.fill: parent; anchors.margins: 14; text: win.toastMessage; color: win.fg; wrapMode: Text.WordWrap; verticalAlignment: Text.AlignVCenter }
 }
 RowLayout {
  anchors.fill: parent; spacing: 0
  Rectangle {
   Layout.preferredWidth: 205; Layout.fillHeight: true; color: win.panel; border.color: win.borderColor
   ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 6
    Label { text: "SECTIONS"; color: win.muted; font.pixelSize: 10; leftPadding: 10; bottomPadding: 8 }
    Repeater { model: ["Overview","Applications","Linux System","Package Manager","Settings","Processes","Windows Apps","Files"]
     delegate: Button {
      required property int index; required property string modelData
      Layout.fillWidth: true; implicitHeight: 43; flat: true
      background: Rectangle { radius: 8; color: win.page===index ? (win.light?"#dbe6ff":"#2b3e60") : (parent.hovered?(win.light?"#e8edf5":"#252d39"):"transparent") }
      contentItem: Text { text: modelData; color: win.fg; font.pixelSize: 13; verticalAlignment: Text.AlignVCenter; leftPadding: 10 }
      onClicked: { win.page=index; if(index===1)systemInfo.refreshApplications(); if(index===5)processMonitor.refreshProcesses() }
     }
    }
    Item { Layout.fillHeight: true }
    Label { text: "Preview · 0.2.0"; color: win.muted; font.pixelSize: 10; leftPadding: 10 }
   }
  }
  ColumnLayout {
   Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: win.page === 0 ? 0 : 24; spacing: win.page === 0 ? 0 : 15
   RowLayout {
    visible: win.page !== 0
    Layout.fillWidth: true
    ColumnLayout { spacing: 2
     Label { text: ["Overview","Applications","Linux System","Package Manager","Settings","Processes","Windows Apps","Files"][win.page]; color: win.fg; font.pixelSize: 26; font.bold: true }
     Label { text: ["Your Linux workspace at a glance.","Find and launch desktop applications.","Host system, session and native tool diagnostics.","Install, remove, or check packages with explicit controls.","Adjust the shell appearance.","Inspect processes running on this Linux host.","Detect Wine and launch compatible Windows installers.","Browse files and folders using the host desktop."][win.page]; color: win.muted; font.pixelSize: 12 }
    }
    Item { Layout.fillWidth: true }
    TextField { visible: win.page===1; placeholderText: "Search apps"; text: win.search; onTextChanged: win.search=text; implicitWidth: 200 }
    Button { visible: win.page===5; text: "Refresh"; onClicked: processMonitor.refreshProcesses() }
    Button { visible: win.page===6; text: "Choose .exe / .msi"; enabled: systemInfo.wineAvailable; onClicked: windowsFileDialog.open() }
   }
   StackLayout {
    Layout.fillWidth: true; Layout.fillHeight: true; currentIndex: win.page
    Item {
     Layout.fillWidth: true; Layout.fillHeight: true; clip: true
     Rectangle { anchors.fill: parent; gradient: Gradient { GradientStop { position: 0; color: "#061632" } GradientStop { position: 0.55; color: "#0c3470" } GradientStop { position: 1; color: "#071a3c" } } }
     Rectangle { x: parent.width*0.37; y: parent.height*0.04; width: parent.width*0.62; height: parent.height*0.88; radius: width/2; rotation: -24; opacity: 0.9; gradient: Gradient { GradientStop { position: 0; color: "#6bb8ff" } GradientStop { position: 0.55; color: "#2469d2" } GradientStop { position: 1; color: "#143b91" } } }
     Rectangle { x: parent.width*0.52; y: parent.height*0.23; width: parent.width*0.49; height: parent.height*0.68; radius: width/2; rotation: 20; opacity: 0.88; gradient: Gradient { GradientStop { position: 0; color: "#d0e7ff" } GradientStop { position: 0.28; color: "#5799f5" } GradientStop { position: 1; color: "#2049ae" } } }
     Rectangle { x: parent.width*0.48; y: parent.height*0.36; width: parent.width*0.45; height: parent.height*0.50; radius: width/2; rotation: -12; opacity: 0.72; gradient: Gradient { GradientStop { position: 0; color: "#76c6ff" } GradientStop { position: 1; color: "#17428f" } } }
     RowLayout {
      anchors.left: parent.left; anchors.top: parent.top; anchors.margins: 12; spacing: 7
      ColumnLayout {
       spacing: 5
       Repeater {
        model: [{label:"This PC",glyph:"▣",page:7},{label:"Home",glyph:"⌂",page:7},{label:"Applications",glyph:"▦",page:1},{label:"Packages",glyph:"▤",page:3},{label:"Settings",glyph:"⚙",page:4},{label:"Task Manager",glyph:"▥",page:5},{label:"Windows apps",glyph:"▣",page:6}]
        delegate: Button {
         required property var modelData
         Layout.preferredWidth: 82; Layout.preferredHeight: 64; padding: 3
         background: Rectangle { radius: 7; color: parent.hovered ? "#334b78" : "transparent"; border.color: parent.hovered ? "#91baff" : "transparent" }
         contentItem: ColumnLayout {
          spacing: 2
          Label { text: modelData.glyph; color: "white"; font.pixelSize: 23; horizontalAlignment: Text.AlignHCenter; Layout.fillWidth: true }
          Label { text: modelData.label; color: "white"; font.pixelSize: 9; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.Wrap; Layout.fillWidth: true }
         }
         onClicked: { win.page=modelData.page; if(modelData.label==="This PC")fileManager.navigate("/"); if(modelData.label==="Home")fileManager.goHome(); if(win.page===1)systemInfo.refreshApplications(); if(win.page===5)processMonitor.refreshProcesses() }
        }
       }
      }
      ColumnLayout {
       spacing: 5
       Repeater {
        model: systemInfo.applications.slice(0, 6)
        delegate: Button {
         required property var modelData
         Layout.preferredWidth: 82; Layout.preferredHeight: 64; padding: 3
         background: Rectangle { radius: 7; color: parent.hovered ? "#334b78" : "transparent"; border.color: parent.hovered ? "#91baff" : "transparent" }
         contentItem: ColumnLayout {
          spacing: 2
          Rectangle { Layout.alignment: Qt.AlignHCenter; width: 27; height: 27; radius: 7; color: "#3978d4"; Label { anchors.centerIn: parent; text: modelData.name.charAt(0).toUpperCase(); color: "white"; font.bold: true; font.pixelSize: 15 } }
          Label { text: modelData.name; color: "white"; font.pixelSize: 9; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.Wrap; maximumLineCount: 2; elide: Text.ElideRight; Layout.fillWidth: true }
         }
         onClicked: systemInfo.launchDesktopFile(modelData.desktopFile)
        }
       }
      }
     }
     Label { anchors.right: parent.right; anchors.bottom: parent.bottom; anchors.margins: 18; text: "Win-To-Linux"; color: "#c8ddff"; opacity: 0.7; font.pixelSize: 12 }
    }
    ColumnLayout {
     spacing: 10
     RowLayout { Layout.fillWidth: true; Label { text: "Installed desktop entries"; color: win.fg; font.bold: true } Item { Layout.fillWidth: true } Button { text: "Refresh"; onClicked: systemInfo.refreshApplications() } }
     Label { text: systemInfo.status; color: win.muted; font.pixelSize: 11 }
     ListView {
      Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 4
      model: systemInfo.applications.filter(function(a){return a.name.toLowerCase().indexOf(win.search.toLowerCase())>=0||a.comment.toLowerCase().indexOf(win.search.toLowerCase())>=0})
      delegate: Rectangle { required property var modelData; width: ListView.view.width; height: 56; radius: 8; color: win.panel; border.color: win.borderColor
       RowLayout { anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 10; spacing: 10
        Rectangle { width: 32; height: 32; radius: 8; color: "#5b8cff"; Text { anchors.centerIn: parent; text: modelData.name.charAt(0).toUpperCase(); color: "white"; font.bold: true } }
        ColumnLayout { Layout.fillWidth: true; spacing: 1; Label { text: modelData.name; color: win.fg; elide: Text.ElideRight; Layout.fillWidth: true } Label { text: modelData.comment; color: win.muted; font.pixelSize: 10; elide: Text.ElideRight; Layout.fillWidth: true } }
        Button { text: "Open"; onClicked: systemInfo.launchDesktopFile(modelData.desktopFile) }
       }
      }
     }
    }
    Flickable {
     clip: true; contentWidth: width; contentHeight: sys.implicitHeight
     ColumnLayout { id: sys; width: parent.width; spacing: 9
      Repeater { model: [{k:"Distribution",v:systemInfo.distribution},{k:"Distribution ID",v:systemInfo.distroId},{k:"Family",v:systemInfo.family},{k:"Version",v:systemInfo.version},{k:"Kernel",v:systemInfo.kernel},{k:"Architecture",v:systemInfo.architecture},{k:"Host name",v:systemInfo.hostName},{k:"Package manager",v:systemInfo.packageManager},{k:"Manager available",v:systemInfo.managerAvailable?"Yes":"No"},{k:"Session type",v:systemInfo.sessionType},{k:"Desktop environment",v:systemInfo.desktopEnvironment},{k:"Wayland display",v:systemInfo.waylandDisplay},{k:"X11 display",v:systemInfo.xDisplay},{k:"Runtime directory",v:systemInfo.runtimeDirectory},{k:"Runtime directory check",v:systemInfo.runtimeDirectoryStatus},{k:"Available compositors",v:String(systemInfo.availableCompositors.length)}, {k:"Qt Wayland plugin",v:systemInfo.qtWaylandPluginAvailable?"Available":"Missing"}]
       delegate: Rectangle { required property var modelData; Layout.fillWidth: true; Layout.preferredHeight: 58; radius: 10; color: win.panel; border.color: win.borderColor
        RowLayout { anchors.fill: parent; anchors.margins: 14; Label { text: modelData.k; color: win.muted; Layout.fillWidth: true } Label { text: modelData.v; color: win.fg; font.bold: true; horizontalAlignment: Text.AlignRight; Layout.maximumWidth: 340; wrapMode: Text.Wrap } }
       }
      }
      Label { text: "Native tools found on PATH ("+systemInfo.nativeTools.length+")"; color: win.fg; font.pixelSize: 18; font.bold: true; topPadding: 14 }
      Label { text: "Only tools actually found on this host are shown. Nothing is installed automatically."; color: win.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
      Repeater { model: systemInfo.nativeTools
       delegate: Rectangle { required property var modelData; Layout.fillWidth: true; Layout.preferredHeight: 54; radius: 9; color: win.panel; border.color: win.borderColor
        RowLayout { anchors.fill: parent; anchors.margins: 12; spacing: 12
         ColumnLayout { Layout.fillWidth: true; spacing: 2
          Label { text: modelData.name+" ("+modelData.command+")"; color: win.fg; font.bold: true; font.pixelSize: 12 }
          Label { text: modelData.category+" · "+modelData.path; color: win.muted; font.pixelSize: 10; elide: Text.ElideMiddle; Layout.fillWidth: true }
         }
         Label { text: "Detected"; color: "#64c99b"; font.pixelSize: 10 }
        }
       }
      }
      Label { visible: systemInfo.nativeTools.length===0; text: "No known native tools were found on PATH."; color: win.muted }
      Label { text: "Compositor executables detected ("+systemInfo.availableCompositors.length+")"; color: win.fg; font.pixelSize: 18; font.bold: true; topPadding: 14 }
      Repeater { model: systemInfo.availableCompositors
       delegate: Rectangle { required property var modelData; Layout.fillWidth: true; Layout.preferredHeight: 48; radius: 9; color: win.panel; border.color: win.borderColor
        RowLayout { anchors.fill: parent; anchors.margins: 12; Label { text: modelData.name; color: win.fg; font.bold: true; Layout.fillWidth: true } Label { text: modelData.path; color: win.muted; font.pixelSize: 10; elide: Text.ElideMiddle; Layout.maximumWidth: 320 } }
       }
      }
      Label { visible: systemInfo.availableCompositors.length===0; text: "No known standalone Wayland compositor executable was found on PATH."; color: win.muted }
      Label { text: "Detection only: this preview never starts a compositor or changes your login manager."; color: win.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 11 }
     }
    }
    PackageManagerPage { controller: packageOperations; Layout.fillWidth: true; Layout.fillHeight: true }
    ColumnLayout { Layout.alignment: Qt.AlignTop; spacing: 12
     Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 90; radius: 12; color: win.panel; border.color: win.borderColor
      RowLayout { anchors.fill: parent; anchors.margins: 17; ColumnLayout { Layout.fillWidth: true; Label { text: "Light appearance"; color: win.fg; font.bold: true } Label { text: "Switch and save the shell appearance."; color: win.muted; font.pixelSize: 11 } } Switch { checked: systemInfo.lightAppearance; onToggled: systemInfo.lightAppearance=checked } }
     }
     Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 105; radius: 12; color: win.panel; border.color: win.borderColor
      ColumnLayout { anchors.fill: parent; anchors.margins: 17; spacing: 5; Label { text: "About Win-To-Linux"; color: win.fg; font.bold: true } Label { text: "Version 0.2.0 · C++20 · Qt 6"; color: win.muted } Label { text: "Runs inside the current desktop; it is not a replacement login session."; color: win.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true } }
     }
     Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 220; radius: 12; color: win.panel; border.color: win.borderColor
      ColumnLayout { anchors.fill: parent; anchors.margins: 17; spacing: 7
       Label { text: "System diagnostics"; color: win.fg; font.bold: true }
       Label { text: "Copy detected distribution, session, runtime, available compositors and compatibility details for troubleshooting."; color: win.muted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
       Label { text: systemInfo.previewReadiness(); color: win.muted; font.pixelSize: 10; wrapMode: Text.WordWrap; Layout.fillWidth: true }
       RowLayout { Layout.fillWidth: true
        Button { text: "Copy diagnostic report"; onClicked: { systemInfo.copyDiagnosticReport(); win.showToast("Diagnostic report copied to clipboard") } }
        Button { text: "Refresh applications"; onClicked: { systemInfo.refreshApplications(); win.showToast("Applications refreshed") } }
       }
       RowLayout { Layout.fillWidth: true
        Button { objectName: "launchNestedPreviewButton"; text: systemInfo.previewRunning ? "Preview running" : "Launch isolated preview"; enabled: !systemInfo.previewRunning; onClicked: { if(systemInfo.launchNestedPreview()) win.showToast("Launching isolated preview..."); else win.showToast(systemInfo.status) } }
        Button { text: "Standalone readiness"; onClicked: standaloneReadinessDialog.open() }
       }
      }
     }
     Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 210; radius: 12; color: win.panel; border.color: win.borderColor
      ColumnLayout { anchors.fill: parent; anchors.margins: 17; spacing: 6
       Label { text: "Keyboard shortcuts"; color: win.fg; font.bold: true }
       Repeater { model: [{key:"Ctrl + Space",action:"Open Start and focus search"},{key:"Ctrl + Shift + N",action:"Open notifications"},{key:"Ctrl + ,",action:"Open Settings"},{key:"Ctrl + Shift + Esc",action:"Open Task Manager"},{key:"Ctrl + E",action:"Open File Explorer"},{key:"Ctrl + L",action:"Focus and select the path field in File Explorer"},{key:"Alt + Tab",action:"Switch application windows (desktop compositor)"},{key:"Ctrl + Alt + Tab",action:"Open section switcher"},{key:"F11",action:"Toggle full screen"}]
        delegate: RowLayout { required property var modelData; Layout.fillWidth: true; Label { text: modelData.key; color: win.fg; font.pixelSize: 10; font.bold: true; Layout.preferredWidth: 150 } Label { text: modelData.action; color: win.muted; font.pixelSize: 10; Layout.fillWidth: true } }
       }
      }
     }
    }
    ColumnLayout {
     spacing: 8
     Label { text: processMonitor.status; color: win.muted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
     RowLayout {
      Layout.fillWidth: true; spacing: 12
       ColumnLayout {
        spacing: 1
        Label { text: "CPU"; color: win.muted; font.pixelSize: 10 }
        Label { text: processMonitor.totalCpuPercent.toFixed(1) + "%"; color: win.fg; font.pixelSize: 18; font.bold: true }
       }
      ProgressBar { from: 0; to: 100; value: processMonitor.totalCpuPercent; Layout.fillWidth: true }
      ColumnLayout {
       spacing: 1
       Label { text: "Memory"; color: win.muted; font.pixelSize: 10 }
       Label { text: processMonitor.memoryText; color: win.fg; font.pixelSize: 12; font.bold: true }
      }
      ProgressBar { from: 0; to: 100; value: processMonitor.memoryPercent; Layout.fillWidth: true }
      Label { text: processMonitor.processes.length + " processes"; color: win.muted; font.pixelSize: 11 }
     }
     RowLayout {
      Layout.fillWidth: true; spacing: 8
      TextField { id: processSearch; Layout.fillWidth: true; placeholderText: "Search process name, PID, or PPID"; selectByMouse: true }
      ComboBox { id: processSort; model: ["Name", "CPU", "RAM"]; implicitWidth: 105 }
      Button { text: "Refresh"; onClicked: { processMonitor.refreshProcesses(); win.showToast("Process list refreshed") } }
     }
     ListView {
      Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 4
      model: processMonitor.processes.filter(function(process) {
       var query = processSearch.text.toLowerCase()
       return query.length === 0 || process.name.toLowerCase().indexOf(query) !== -1
         || String(process.pid).indexOf(query) !== -1
         || String(process.parentPid).indexOf(query) !== -1
      }).sort(function(a, b) {
       if (processSort.currentIndex === 1) return b.cpuPercent - a.cpuPercent
       if (processSort.currentIndex === 2) return b.memoryKiB - a.memoryKiB
       return a.name.localeCompare(b.name)
      })
      delegate: Rectangle { required property var modelData; width: ListView.view.width; height: 52; radius: 8; color: win.panel; border.color: win.borderColor
       RowLayout { anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 10; spacing: 10
        ColumnLayout { Layout.fillWidth: true; spacing: 1; Label { text: modelData.name; color: win.fg; font.pixelSize: 12; Layout.leftMargin: Math.min(modelData.depth, 8) * 12; elide: Text.ElideRight; Layout.fillWidth: true } Label { text: "PID "+modelData.pid+" · PPID "+modelData.parentPid+" · CPU "+modelData.cpuText+" · RAM "+modelData.memoryText; color: win.muted; font.pixelSize: 10 } }
        Button { visible: modelData.canTerminate; text: "Stop"; onClicked: { win.selectedProcess=modelData; stopDialog.open() } }
       }
      }
     }
    }
    ColumnLayout {
     spacing: 12
     Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 130; radius: 12; color: win.panel; border.color: win.borderColor
      ColumnLayout { anchors.fill: parent; anchors.margins: 18; spacing: 8
       Label { text: "Windows compatibility"; color: win.fg; font.pixelSize: 17; font.bold: true }
       Label { text: systemInfo.wineAvailable ? ("Wine detected: "+systemInfo.winePath) : "Wine was not found on PATH. Win-To-Linux will not install it automatically."; color: win.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
       Label { text: systemInfo.bottlesAvailable ? ("Bottles detected: "+systemInfo.bottlesPath) : "Bottles: not detected"; color: win.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
      }
     }
     Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 120; radius: 12; color: win.panel; border.color: win.borderColor
      ColumnLayout { anchors.fill: parent; anchors.margins: 18; spacing: 8
       Label { text: "Run an existing Windows file"; color: win.fg; font.bold: true }
       Label { text: "Select an .exe or .msi file, then confirm before it is passed to Wine. Compatibility, dependencies, and installer behavior vary by application."; color: win.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
       Button { text: "Choose .exe / .msi"; enabled: systemInfo.wineAvailable; onClicked: windowsFileDialog.open() }
      }
     }
    }
    FileManagerPage { controller: fileManager; Layout.fillWidth: true; Layout.fillHeight: true }
   }
   Rectangle {
    Layout.fillWidth: true; Layout.preferredHeight: 42; radius: 10; color: win.panel; border.color: win.borderColor
    RowLayout { anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 0
     Item { Layout.fillWidth: true }
     RowLayout {
      spacing: 5
      Button {
       objectName: "taskbarStartButton"; Accessible.name: "Start menu"; text: "⊞"; implicitWidth: 38; implicitHeight: 36; padding: 0
       ToolTip.visible: hovered; ToolTip.text: "Start"
       background: Rectangle { radius: 8; color: parent.hovered ? (win.light ? "#e5ebf6" : "#303847") : "transparent" }
       onClicked: startMenu.open()
      }
      Button {
       objectName: "taskbarSearchButton"; Accessible.name: "Search applications"; text: "⌕"; implicitWidth: 38; implicitHeight: 36; padding: 0
       ToolTip.visible: hovered; ToolTip.text: "Search apps"
       background: Rectangle { radius: 8; color: parent.hovered ? (win.light ? "#e5ebf6" : "#303847") : "transparent" }
       onClicked: { systemInfo.refreshApplications(); startMenu.open(); Qt.callLater(function(){ startSearchField.forceActiveFocus() }) }
      }
      Button {
       objectName: "taskbarLinuxButton"; Accessible.name: "Linux system"; text: "▦"; implicitWidth: 38; implicitHeight: 36; padding: 0
       ToolTip.visible: hovered; ToolTip.text: "Linux"
       background: Rectangle {
        radius: 8; color: parent.hovered ? (win.light ? "#e5ebf6" : "#303847") : "transparent"
        Rectangle { visible: win.page === 2; width: 12; height: 3; radius: 2; color: "#72a7ff"; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom }
       }
       onClicked: win.page=2
      }
      Button {
       objectName: "taskbarPackagesButton"; Accessible.name: "Package manager"; text: "▤"; implicitWidth: 38; implicitHeight: 36; padding: 0
       ToolTip.visible: hovered; ToolTip.text: "Packages"
       background: Rectangle {
        radius: 8; color: parent.hovered ? (win.light ? "#e5ebf6" : "#303847") : "transparent"
        Rectangle { visible: win.page === 3; width: 12; height: 3; radius: 2; color: "#72a7ff"; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom }
       }
       onClicked: win.page=3
      }
      Button {
       objectName: "taskbarFilesButton"; Accessible.name: "File Explorer"; text: "📁"; implicitWidth: 38; implicitHeight: 36; padding: 0
       ToolTip.visible: hovered; ToolTip.text: "File Explorer"
       background: Rectangle {
        radius: 8; color: parent.hovered ? (win.light ? "#e5ebf6" : "#303847") : "transparent"
        Rectangle { visible: win.page === 7; width: 12; height: 3; radius: 2; color: "#72a7ff"; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom }
       }
       onClicked: win.page=7
      }
      Button {
       objectName: "taskbarSettingsButton"; Accessible.name: "Settings"; text: "⚙"; implicitWidth: 38; implicitHeight: 36; padding: 0
       ToolTip.visible: hovered; ToolTip.text: "Settings"
       background: Rectangle {
        radius: 8; color: parent.hovered ? (win.light ? "#e5ebf6" : "#303847") : "transparent"
        Rectangle { visible: win.page === 4; width: 12; height: 3; radius: 2; color: "#72a7ff"; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom }
       }
       onClicked: win.page=4
      }
     }
     Item { Layout.fillWidth: true }
     Label { text: systemInfo.desktopEnvironment+" · "+systemInfo.sessionType; color: win.muted; font.pixelSize: 10; elide: Text.ElideRight; Layout.maximumWidth: 135 }
     Button { objectName: "taskbarNotificationsButton"; Accessible.name: "Notifications"; text: "🔔"; ToolTip.visible: hovered; ToolTip.text: "Notifications"; onClicked: notificationCenter.open() }
     Button { objectName: "taskbarQuickSettingsButton"; Accessible.name: "Clock and quick settings"; text: win.clockText; ToolTip.visible: hovered; ToolTip.text: "Clock and quick settings"; onClicked: quickSettings.open() }
    }
   }
   Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 28; radius: 7; color: win.panel
    RowLayout { anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; Label { text: win.page===5?processMonitor.status:win.page===3?packageOperations.status:win.page===7?fileManager.status:systemInfo.status; color: win.muted; font.pixelSize: 10; Layout.fillWidth: true; elide: Text.ElideRight } Label { text: systemInfo.hostName; color: win.muted; font.pixelSize: 10 } }
   }
  }
 }
}
