import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id: root
 property var controller: fileManager
 property bool lightAppearance: systemInfo.lightAppearance
 readonly property color fg: lightAppearance ? "#1a2432" : "#f4f6fb"
 readonly property color muted: lightAppearance ? "#647084" : "#aab4c4"
 readonly property color panel: lightAppearance ? "#ffffff" : "#191f29"
 readonly property color borderColor: lightAppearance ? "#dce2eb" : "#2b3340"
 ColumnLayout {
  anchors.fill: parent; spacing: 10
  RowLayout {
   Layout.fillWidth: true; spacing: 8
   Button { text: "Up"; enabled: root.controller.canGoUp; onClicked: root.controller.goUp() }
   TextField { id: pathField; Layout.fillWidth: true; text: root.controller.currentPath; onAccepted: root.controller.navigate(text); selectByMouse: true }
   Button { text: "Go"; onClicked: root.controller.navigate(pathField.text) }
   Button { text: "Refresh"; onClicked: root.controller.refresh() }
  }
  Connections {
   target: root.controller
   function onCurrentPathChanged() { pathField.text = root.controller.currentPath }
  }
  Label { text: root.controller.status; color: root.muted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
  RowLayout {
   Layout.fillWidth: true; spacing: 10
   Label { text: "Name"; color: root.muted; Layout.fillWidth: true }
   Label { text: "Modified"; color: root.muted; width: 140 }
   Label { text: "Size"; color: root.muted; width: 90; horizontalAlignment: Text.AlignRight }
  }
  ListView {
   Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 3
   model: root.controller.entries
   delegate: Rectangle {
    required property var modelData
    width: ListView.view.width; height: 48; radius: 7
    property bool hovered: false
    color: hovered ? (root.lightAppearance ? "#e8edf5" : "#252d39") : root.panel
    border.color: root.borderColor
    RowLayout {
     anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 10
     Text { text: modelData.isDir ? "📁" : "📄"; color: root.fg; font.pixelSize: 17 }
     Label { text: modelData.name; color: root.fg; elide: Text.ElideMiddle; Layout.fillWidth: true }
     Label { text: modelData.modified; color: root.muted; font.pixelSize: 10; width: 140 }
     Label { text: modelData.sizeText; color: root.muted; font.pixelSize: 10; width: 90; horizontalAlignment: Text.AlignRight }
     Button { text: "Open"; onClicked: root.controller.openEntry(modelData.path) }
    }
    HoverHandler { onHoveredChanged: parent.hovered = hovered }
   }
  }
  Label { text: "Read-only browser: file changes and deletion are intentionally not exposed in this preview."; color: root.muted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
 }
}
