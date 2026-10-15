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
 readonly property color fg: light ? "#1a2432" : "#f4f6fb"
 readonly property color muted: light ? "#647084" : "#aab4c4"
 readonly property color panel: light ? "#ffffff" : "#191f29"
 readonly property color borderColor: light ? "#dce2eb" : "#2b3340"
 header: Rectangle {
  height: 62; color: win.panel; border.color: win.borderColor
  RowLayout { anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 20
   Rectangle { width: 34; height: 34; radius: 9; color: "#5b8cff"; Text { anchors.centerIn: parent; text: "W"; color: "white"; font.bold: true; font.pixelSize: 20 } }
   ColumnLayout { spacing: 0; Label { text: "Win-To-Linux"; color: win.fg; font.bold: true; font.pixelSize: 17 } Label { text: "Linux desktop workspace"; color: win.muted; font.pixelSize: 11 } }
   Item { Layout.fillWidth: true }
   Label { text: systemInfo.distribution; color: win.muted }
  }
 }
 Dialog {
  id: stopDialog; modal: true; title: "Stop process?"; standardButtons: Dialog.Ok | Dialog.Cancel; anchors.centerIn: parent
  Label { text: "Send SIGTERM to " + (win.selectedProcess.name || "process") + " (PID " + (win.selectedProcess.pid || "—") + ")? Unsaved work in that process may be lost."; wrapMode: Text.WordWrap; width: 330 }
  onAccepted: processMonitor.terminateProcess(win.selectedProcess.pid, win.selectedProcess.startTicks)
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
  id: startMenu
  width: 380; height: 390; modal: true; focus: true
  x: Math.max(8, (win.width-width)/2)
  y: Math.max(8, win.height-height-55)
  closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
  background: Rectangle { radius: 16; color: win.panel; border.color: win.borderColor }
  contentItem: ColumnLayout {
   anchors.fill: parent; anchors.margins: 16; spacing: 12
   Label { text: "Start"; color: win.fg; font.pixelSize: 21; font.bold: true }
   TextField { placeholderText: "Search workspace"; text: win.startSearch; onTextChanged: win.startSearch=text; Layout.fillWidth: true }
   Label { text: "Quick launch"; color: win.muted; font.pixelSize: 11 }
   GridLayout {
    Layout.fillWidth: true; columns: 2; columnSpacing: 8; rowSpacing: 8
    Repeater {
     model: [{label:"Overview",page:0},{label:"Applications",page:1},{label:"Linux System",page:2},{label:"Packages",page:3},{label:"Settings",page:4},{label:"Processes",page:5},{label:"Windows Apps",page:6},{label:"Files",page:7}].filter(function(item){return item.label.toLowerCase().indexOf(win.startSearch.toLowerCase())>=0})
     delegate: Button {
      required property var modelData
      text: modelData.label; Layout.fillWidth: true
      onClicked: { win.page=modelData.page; startMenu.close(); if(win.page===1)systemInfo.refreshApplications(); if(win.page===5)processMonitor.refreshProcesses() }
     }
    }
   }
   Item { Layout.fillHeight: true }
   Label { text: "Win-To-Linux preview · does not replace your login session"; color: win.muted; font.pixelSize: 10; wrapMode: Text.WordWrap; Layout.fillWidth: true }
  }
 }
 Shortcut { sequence: "Ctrl+Space"; onActivated: startMenu.open() }
 RowLayout {
  anchors.fill: parent; spacing: 0
  Rectangle {
   Layout.preferredWidth: 205; Layout.fillHeight: true; color: win.panel; border.color: win.borderColor
   ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 6
    Label { text: "WORKSPACE"; color: win.muted; font.pixelSize: 10; leftPadding: 10; bottomPadding: 8 }
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
   Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: 24; spacing: 15
   RowLayout {
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
    Flickable {
     clip: true; contentWidth: width; contentHeight: dash.implicitHeight
     ColumnLayout { id: dash; width: parent.width; spacing: 14
      Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 160; radius: 16; gradient: Gradient { GradientStop { position: 0; color: "#315b9a" } GradientStop { position: 1; color: "#222c48" } }
       ColumnLayout { anchors.fill: parent; anchors.margins: 22; spacing: 8
        Label { text: "A familiar desktop. Your Linux system."; color: "white"; font.pixelSize: 22; font.bold: true; wrapMode: Text.WordWrap }
        Label { text: "Keep your native distribution and tools, with a consistent workspace on top."; color: "#e2eaff"; wrapMode: Text.WordWrap }
        Item { Layout.fillHeight: true }
        Button { text: "Browse applications"; onClicked: { win.page=1; systemInfo.refreshApplications() } }
       }
      }
      GridLayout { Layout.fillWidth: true; columns: width>720?3:2; columnSpacing: 12; rowSpacing: 12
       Repeater { model: [{t:"Distribution",v:systemInfo.distribution,d:"Detected host"},{t:"Package manager",v:systemInfo.packageManager,d:systemInfo.managerAvailable?"Executable found":"Not available"},{t:"Native tools",v:String(systemInfo.nativeTools.length),d:"Commands found on PATH"},{t:"Desktop apps",v:String(systemInfo.applications.length),d:"Entries discovered"}]
        delegate: Rectangle { required property var modelData; Layout.fillWidth: true; Layout.preferredHeight: 105; radius: 12; color: win.panel; border.color: win.borderColor
         ColumnLayout { anchors.fill: parent; anchors.margins: 15; spacing: 7
          Label { text: modelData.t; color: win.muted; font.pixelSize: 11 }
          Label { text: modelData.v; color: win.fg; font.pixelSize: 17; font.bold: true; elide: Text.ElideRight; Layout.fillWidth: true }
          Label { text: modelData.d; color: win.muted; font.pixelSize: 10 }
         }
        }
       }
      }
      Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 90; radius: 12; color: win.panel; border.color: win.borderColor
       ColumnLayout { anchors.fill: parent; anchors.margins: 15; spacing: 5
        Label { text: "Safety boundary"; color: win.fg; font.bold: true }
        Label { text: "This preview does not replace your login session. System package changes require explicit confirmation."; color: win.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
       }
      }
     }
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
      Repeater { model: [{k:"Distribution",v:systemInfo.distribution},{k:"Distribution ID",v:systemInfo.distroId},{k:"Family",v:systemInfo.family},{k:"Version",v:systemInfo.version},{k:"Kernel",v:systemInfo.kernel},{k:"Architecture",v:systemInfo.architecture},{k:"Host name",v:systemInfo.hostName},{k:"Package manager",v:systemInfo.packageManager},{k:"Manager available",v:systemInfo.managerAvailable?"Yes":"No"},{k:"Session type",v:systemInfo.sessionType},{k:"Desktop environment",v:systemInfo.desktopEnvironment},{k:"Wayland display",v:systemInfo.waylandDisplay},{k:"X11 display",v:systemInfo.xDisplay},{k:"Runtime directory",v:systemInfo.runtimeDirectory},{k:"Runtime directory check",v:systemInfo.runtimeDirectoryStatus},{k:"Available compositors",v:String(systemInfo.availableCompositors.length)}]
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
    }
    ColumnLayout {
     spacing: 8
     Label { text: processMonitor.status; color: win.muted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
     ListView {
      Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 4; model: processMonitor.processes
      delegate: Rectangle { required property var modelData; width: ListView.view.width; height: 52; radius: 8; color: win.panel; border.color: win.borderColor
       RowLayout { anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 10; spacing: 10
        ColumnLayout { Layout.fillWidth: true; spacing: 1; Label { text: modelData.name; color: win.fg; font.pixelSize: 12; elide: Text.ElideRight; Layout.fillWidth: true } Label { text: "PID "+modelData.pid+" · PPID "+modelData.parentPid+" · "+modelData.memoryText; color: win.muted; font.pixelSize: 10 } }
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
    RowLayout { anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 6
     Button { text: "⊞ Start"; onClicked: startMenu.open() }
     Button { text: "Apps"; onClicked: { win.page=1; systemInfo.refreshApplications() } }
     Button { text: "Linux"; onClicked: win.page=2 }
     Button { text: "Packages"; onClicked: win.page=3 }
     Button { text: "Files"; onClicked: win.page=7 }
     Item { Layout.fillWidth: true }
     Label { text: systemInfo.desktopEnvironment+" · "+systemInfo.sessionType; color: win.muted; font.pixelSize: 10; elide: Text.ElideRight }
    }
   }
   Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 28; radius: 7; color: win.panel
    RowLayout { anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; Label { text: win.page===5?processMonitor.status:win.page===3?packageOperations.status:win.page===7?fileManager.status:systemInfo.status; color: win.muted; font.pixelSize: 10; Layout.fillWidth: true; elide: Text.ElideRight } Label { text: systemInfo.hostName; color: win.muted; font.pixelSize: 10 } }
   }
  }
 }
}
