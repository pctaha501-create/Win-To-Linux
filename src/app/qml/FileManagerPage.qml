import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id: root
 property var controller: fileManager
 ColumnLayout {
  anchors.fill: parent; spacing: 10
  RowLayout {
   Layout.fillWidth: true; spacing: 8
   Button { text: "Up"; enabled: root.controller.canGoUp; onClicked: root.controller.goUp() }
   TextField { id: pathField; Layout.fillWidth: true; text: root.controller.currentPath; onAccepted: root.controller.navigate(text); selectByMouse: true }
   Button { text: "Go"; onClicked: root.controller.navigate(pathField.text) }
   Button { text: "Refresh"; onClicked: root.controller.refresh() }
  }
  Label { text: root.controller.status; color: "#aab4c4"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
  RowLayout {
   Layout.fillWidth: true; spacing: 10
   Label { text: "Name"; color: "#aab4c4"; Layout.fillWidth: true }
   Label { text: "Modified"; color: "#aab4c4"; width: 140 }
   Label { text: "Size"; color: "#aab4c4"; width: 90; horizontalAlignment: Text.AlignRight }
  }
  ListView {
   Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 3
   model: root.controller.entries
   delegate: Rectangle {
    required property var modelData
    width: ListView.view.width; height: 48; radius: 7
    color: hovered ? "#252d39" : "#191f29"; border.color: "#2b3340"
    property bool hovered: false
    RowLayout {
     anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 10
     Text { text: modelData.isDir ? "📁" : "📄"; color: "#f4f6fb"; font.pixelSize: 17 }
     Label { text: modelData.name; color: "#f4f6fb"; elide: Text.ElideMiddle; Layout.fillWidth: true }
     Label { text: modelData.modified; color: "#aab4c4"; font.pixelSize: 10; width: 140 }
     Label { text: modelData.sizeText; color: "#aab4c4"; font.pixelSize: 10; width: 90; horizontalAlignment: Text.AlignRight }
     Button { text: "Open"; onClicked: root.controller.openEntry(modelData.path) }
    }
    HoverHandler { onHoveredChanged: parent.hovered = hovered }
   }
  }
  Label { text: "Read-only browser: file changes and deletion are intentionally not exposed in this preview."; color: "#aab4c4"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
 }
}
