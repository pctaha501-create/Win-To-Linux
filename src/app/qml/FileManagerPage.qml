import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id: root
 property var controller: fileManager
 property string selectedEntryPath: ""
 property string selectedEntryName: ""
 property bool selectedEntryIsDir: false
 property bool selectedEntryForMove: false
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
   Button { objectName: "fileManagerHomeButton"; text: "Home"; onClicked: root.controller.goHome() }
   TextField { id: pathField; Layout.fillWidth: true; text: root.controller.currentPath; onAccepted: root.controller.navigate(text); selectByMouse: true }
   Button { text: "Go"; onClicked: root.controller.navigate(pathField.text) }
   Button { text: "Refresh"; onClicked: root.controller.refresh() }
   Button { text: "New folder"; onClicked: { folderNameField.text = ""; createDialog.open() } }
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
     Button { text: "Rename"; onClicked: { root.selectedEntryPath = modelData.path; root.selectedEntryName = modelData.name; renameField.text = modelData.name; renameDialog.open() } }
     Button { text: "Delete"; onClicked: { root.selectedEntryPath = modelData.path; root.selectedEntryName = modelData.name; root.selectedEntryIsDir = modelData.isDir; deleteDialog.open() } }
     Button { text: "Copy"; onClicked: { root.selectedEntryPath = modelData.path; root.selectedEntryName = modelData.name; root.selectedEntryForMove = false; destinationField.text = root.controller.currentPath; transferDialog.open() } }
     Button { text: "Move"; onClicked: { root.selectedEntryPath = modelData.path; root.selectedEntryName = modelData.name; root.selectedEntryForMove = true; destinationField.text = root.controller.currentPath; transferDialog.open() } }
    }
    HoverHandler { onHoveredChanged: parent.hovered = hovered }
   }
  }
  Dialog {
   id: transferDialog
   width: 480
   modal: true
   title: root.selectedEntryForMove ? "Move entry" : "Copy entry"
   standardButtons: Dialog.Ok | Dialog.Cancel
   contentItem: ColumnLayout {
    width: 420
    Label { text: "Destination directory"; color: root.fg }
    TextField { id: destinationField; Layout.fillWidth: true; placeholderText: "/path/to/folder"; selectByMouse: true }
    Label { text: "Existing destinations are never overwritten."; color: root.muted; wrapMode: Text.WordWrap }
   }
   onAccepted: {
    if (root.selectedEntryForMove)
     root.controller.moveEntry(root.selectedEntryPath, destinationField.text)
    else
     root.controller.copyEntry(root.selectedEntryPath, destinationField.text)
   }
  }
  Dialog {
   id: createDialog
   modal: true
   title: "Create folder"
   standardButtons: Dialog.Ok | Dialog.Cancel
   contentItem: TextField { id: folderNameField; placeholderText: "Folder name"; selectByMouse: true }
   onAccepted: root.controller.createFolder(folderNameField.text)
  }
  Dialog {
   id: renameDialog
   modal: true
   title: "Rename entry"
   standardButtons: Dialog.Ok | Dialog.Cancel
   contentItem: TextField { id: renameField; placeholderText: "New name"; selectByMouse: true }
   onAccepted: root.controller.renameEntry(root.selectedEntryPath, renameField.text)
  }
  Dialog {
   id: deleteDialog
   modal: true
   title: "Confirm deletion"
   standardButtons: Dialog.Yes | Dialog.No
   contentItem: Label {
    text: root.selectedEntryIsDir
      ? "Delete folder and all its contents? This cannot be undone.\\n\\n" + root.selectedEntryName
      : "Delete this file? This cannot be undone.\\n\\n" + root.selectedEntryName
    color: root.fg
    padding: 16
    wrapMode: Text.WordWrap
   }
   onAccepted: root.controller.deleteEntry(root.selectedEntryPath, root.selectedEntryIsDir)
  }
 }
}
