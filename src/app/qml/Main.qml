import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ApplicationWindow {
 id: win; visible: true; width: 1120; height: 720; minimumWidth: 820; minimumHeight: 560
 title: "Win-To-Linux"; color: light ? "#f1f4f8" : "#10141b"
 property int page: 0
 property bool light: false
 property string search: ""
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
 RowLayout {
  anchors.fill: parent; spacing: 0
  Rectangle {
   Layout.preferredWidth: 205; Layout.fillHeight: true; color: win.panel; border.color: win.borderColor
   ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 6
    Label { text: "WORKSPACE"; color: win.muted; font.pixelSize: 10; leftPadding: 10; bottomPadding: 8 }
    Repeater { model: ["Overview","Applications","Linux System","Package Manager","Settings"]
     delegate: Button {
      required property int index; required property string modelData
      Layout.fillWidth: true; implicitHeight: 43; flat: true
      background: Rectangle { radius: 8; color: win.page===index ? (win.light?"#dbe6ff":"#2b3e60") : (parent.hovered?(win.light?"#e8edf5":"#252d39"):"transparent") }
      contentItem: Text { text: modelData; color: win.fg; font.pixelSize: 13; verticalAlignment: Text.AlignVCenter; leftPadding: 10 }
      onClicked: { win.page=index; if(index===1)systemInfo.refreshApplications() }
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
     Label { text: ["Overview","Applications","Linux System","Package Manager","Settings"][win.page]; color: win.fg; font.pixelSize: 26; font.bold: true }
     Label { text: ["Your Linux workspace at a glance.","Find and launch desktop applications.","Information detected from the host system.","Package actions are not executed in this preview.","Adjust the shell appearance."][win.page]; color: win.muted; font.pixelSize: 12 }
    }
    Item { Layout.fillWidth: true }
    TextField { visible: win.page===1; placeholderText: "Search apps"; text: win.search; onTextChanged: win.search=text; implicitWidth: 200 }
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
       Repeater { model: [{t:"Distribution",v:systemInfo.distribution,d:"Detected host"},{t:"Package manager",v:systemInfo.packageManager,d:systemInfo.managerAvailable?"Executable found":"Not available"},{t:"Desktop apps",v:String(systemInfo.applications.length),d:"Entries discovered"}]
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
        Label { text: "This preview does not replace your login session or run privileged package operations."; color: win.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
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
      Repeater { model: [{k:"Distribution",v:systemInfo.distribution},{k:"Distribution ID",v:systemInfo.distroId},{k:"Family",v:systemInfo.family},{k:"Version",v:systemInfo.version},{k:"Kernel",v:systemInfo.kernel},{k:"Architecture",v:systemInfo.architecture},{k:"Host name",v:systemInfo.hostName},{k:"Package manager",v:systemInfo.packageManager},{k:"Manager available",v:systemInfo.managerAvailable?"Yes":"No"}]
       delegate: Rectangle { required property var modelData; Layout.fillWidth: true; Layout.preferredHeight: 58; radius: 10; color: win.panel; border.color: win.borderColor
        RowLayout { anchors.fill: parent; anchors.margins: 14; Label { text: modelData.k; color: win.muted; Layout.fillWidth: true } Label { text: modelData.v; color: win.fg; font.bold: true; horizontalAlignment: Text.AlignRight; Layout.maximumWidth: 340; wrapMode: Text.Wrap } }
       }
      }
     }
    }
    Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; radius: 12; color: win.panel; border.color: win.borderColor
     ColumnLayout { anchors.fill: parent; anchors.margins: 22; spacing: 12
      Label { text: "Package operations"; color: win.fg; font.pixelSize: 18; font.bold: true }
      Label { text: "Distribution-aware command planning is implemented in the core. This page intentionally does not install or remove software until confirmation, cancellation, timeout, and error handling are implemented."; color: win.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
      Label { text: "Detected manager: "+systemInfo.packageManager+(systemInfo.managerAvailable?" (available)":" (not available)"); color: win.fg; font.bold: true }
     }
    }
    ColumnLayout { Layout.alignment: Qt.AlignTop; spacing: 12
     Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 90; radius: 12; color: win.panel; border.color: win.borderColor
      RowLayout { anchors.fill: parent; anchors.margins: 17; ColumnLayout { Layout.fillWidth: true; Label { text: "Light appearance"; color: win.fg; font.bold: true } Label { text: "Switch the shell's colors."; color: win.muted; font.pixelSize: 11 } } Switch { checked: win.light; onToggled: win.light=checked } }
     }
     Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 105; radius: 12; color: win.panel; border.color: win.borderColor
      ColumnLayout { anchors.fill: parent; anchors.margins: 17; spacing: 5; Label { text: "About Win-To-Linux"; color: win.fg; font.bold: true } Label { text: "Version 0.2.0 · C++20 · Qt 6"; color: win.muted } Label { text: "Runs inside the current desktop; it is not a replacement login session."; color: win.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true } }
     }
    }
   }
   Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 28; radius: 7; color: win.panel
    RowLayout { anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; Label { text: systemInfo.status; color: win.muted; font.pixelSize: 10; Layout.fillWidth: true; elide: Text.ElideRight } Label { text: systemInfo.hostName; color: win.muted; font.pixelSize: 10 } }
   }
  }
 }
}
