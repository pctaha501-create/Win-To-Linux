import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Item {
 id: root
 property var controller
 property string pendingAction: ""
 property string pendingPackage: ""
 Dialog {
  id: confirmDialog; modal: true; title: "Confirm package operation"
  standardButtons: Dialog.Ok | Dialog.Cancel; anchors.centerIn: parent
  contentItem: Label {
   width: 360; wrapMode: Text.WordWrap
   text: "You are about to " + root.pendingAction + " package \"" + root.pendingPackage + "\" on this Linux system. This can modify installed software. Continue only if you intend this change."
   color: "#f4f6fb"
  }
  onAccepted: {
   if (root.pendingAction === "install") root.controller.installPackage(root.pendingPackage)
   else if (root.pendingAction === "remove") root.controller.removePackage(root.pendingPackage)
  }
 }
 ColumnLayout {
  anchors.fill: parent; spacing: 12
  Label { text: "Package operations"; color: "#f4f6fb"; font.pixelSize: 19; font.bold: true }
  Label { text: "Manager: " + systemInfo.packageManager + (systemInfo.managerAvailable ? " · detected" : " · unavailable"); color: "#aab4c4"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
  RowLayout {
   Layout.fillWidth: true; spacing: 8
   TextField { id: packageName; Layout.fillWidth: true; placeholderText: "Package name (e.g. curl)"; enabled: !root.controller.running }
   Button { text: "Check updates"; enabled: !root.controller.running; onClicked: root.controller.checkUpdates() }
  }
  RowLayout {
   Layout.fillWidth: true; spacing: 8
   Button { text: "Install"; enabled: !root.controller.running && packageName.text.trim().length > 0; onClicked: { root.pendingAction="install"; root.pendingPackage=packageName.text.trim(); confirmDialog.open() } }
   Button { text: "Remove"; enabled: !root.controller.running && packageName.text.trim().length > 0; onClicked: { root.pendingAction="remove"; root.pendingPackage=packageName.text.trim(); confirmDialog.open() } }
   Item { Layout.fillWidth: true }
   Button { text: "Cancel operation"; visible: root.controller.running; onClicked: root.controller.cancel() }
  }
  Label { text: root.controller.status; color: root.controller.running ? "#f0c674" : "#aab4c4"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
  Label { text: "Command output"; color: "#f4f6fb"; font.bold: true }
  ScrollView {
   Layout.fillWidth: true; Layout.fillHeight: true; clip: true
   TextArea { text: root.controller.output; readOnly: true; selectByMouse: true; wrapMode: TextEdit.Wrap; color: "#f4f6fb"; placeholderText: "Output from the last package operation appears here."; background: Rectangle { color: "#10141b"; border.color: "#2b3340"; radius: 8 } }
  }
  Label { text: "Install/remove requires explicit confirmation and PolicyKit authentication. Nothing runs automatically."; color: "#aab4c4"; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 11 }
 }
}
