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
    property string searchText: ""
    property bool iconView: false
    property bool lightAppearance: systemInfo.lightAppearance
    readonly property color fg: lightAppearance ? "#202124" : "#f3f5f9"
    readonly property color muted: lightAppearance ? "#626975" : "#a8b0bf"
    readonly property color surface: lightAppearance ? "#ffffff" : "#202126"
    readonly property color surfaceAlt: lightAppearance ? "#f5f7fb" : "#292b31"
    readonly property color line: lightAppearance ? "#e0e4eb" : "#3a3d45"
    readonly property color accent: lightAppearance ? "#1768d2" : "#7bb7ff"

    function choose(entry) {
        selectedEntryPath = entry.path
        selectedEntryName = entry.name
        selectedEntryIsDir = entry.isDir
    }
    function openSelected() {
        if (selectedEntryPath.length) controller.openEntry(selectedEntryPath)
    }
    function filtered(entry) {
        return searchText.trim().length === 0 || entry.name.toLowerCase().indexOf(searchText.trim().toLowerCase()) >= 0
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Explorer command bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 58
            color: root.surface
            border.color: root.line
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 14; anchors.rightMargin: 14; spacing: 8
                Button {
                    objectName: "fileManagerBackButton"
                    text: "‹"; font.pixelSize: 25; enabled: root.controller.canGoUp
                    ToolTip.visible: hovered; ToolTip.text: "Up one folder"
                    onClicked: root.controller.goUp()
                    background: Rectangle { radius: 7; color: parent.hovered ? root.surfaceAlt : "transparent" }
                }
                Button {
                    objectName: "fileManagerHomeButton"
                    text: "Home"; icon.name: "home"
                    onClicked: root.controller.goHome()
                    background: Rectangle { radius: 7; color: parent.hovered ? root.surfaceAlt : "transparent" }
                }
                Rectangle { width: 1; Layout.fillHeight: true; Layout.topMargin: 12; Layout.bottomMargin: 12; color: root.line }
                Button {
                    objectName: "fileManagerNewFolderButton"
                    text: "New folder"; icon.name: "folder-new"
                    onClicked: { folderNameField.text = ""; createDialog.open() }
                    background: Rectangle { radius: 7; color: parent.hovered ? root.surfaceAlt : "transparent" }
                }
                Button {
                    objectName: "fileManagerRenameButton"
                    text: "Rename"; icon.name: "rename"; enabled: root.selectedEntryPath.length > 0
                    onClicked: { renameField.text = root.selectedEntryName; renameDialog.open() }
                    background: Rectangle { radius: 7; color: parent.hovered ? root.surfaceAlt : "transparent" }
                }
                Button {
                    objectName: "fileManagerCopyButton"
                    text: "Copy to"; icon.name: "copy"; enabled: root.selectedEntryPath.length > 0
                    onClicked: { root.selectedEntryForMove = false; destinationField.text = root.controller.currentPath; transferDialog.open() }
                    background: Rectangle { radius: 7; color: parent.hovered ? root.surfaceAlt : "transparent" }
                }
                Button {
                    objectName: "fileManagerMoveButton"
                    text: "Move to"; icon.name: "move"; enabled: root.selectedEntryPath.length > 0
                    onClicked: { root.selectedEntryForMove = true; destinationField.text = root.controller.currentPath; transferDialog.open() }
                    background: Rectangle { radius: 7; color: parent.hovered ? root.surfaceAlt : "transparent" }
                }
                Button {
                    objectName: "fileManagerDeleteButton"
                    text: "Delete"; icon.name: "delete"; enabled: root.selectedEntryPath.length > 0
                    onClicked: deleteDialog.open()
                    background: Rectangle { radius: 7; color: parent.hovered ? (root.lightAppearance ? "#fff0f0" : "#45282b") : "transparent" }
                }
                Item { Layout.fillWidth: true }
                Button {
                    objectName: "fileManagerViewToggle"
                    text: root.iconView ? "Details" : "Icons"; icon.name: root.iconView ? "list" : "grid"
                    onClicked: root.iconView = !root.iconView
                    background: Rectangle { radius: 7; color: parent.hovered ? root.surfaceAlt : "transparent" }
                }
                Button {
                    objectName: "fileManagerRefreshButton"
                    icon.name: "refresh"; text: ""; ToolTip.visible: hovered; ToolTip.text: "Refresh (F5)"
                    onClicked: root.controller.refresh()
                    background: Rectangle { radius: 7; color: parent.hovered ? root.surfaceAlt : "transparent" }
                }
            }
        }

        // Address and search, as in a desktop file manager.
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 54
            color: root.surfaceAlt
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 10
                Icon { name: "folder"; size: 23; color: root.accent }
                TextField {
                    id: pathField; objectName: "fileManagerPathField"
                    Layout.fillWidth: true; text: root.controller.currentPath
                    selectByMouse: true; placeholderText: "Enter a folder path"; onAccepted: root.controller.navigate(text)
                    background: Rectangle { radius: 7; color: root.surface; border.color: pathField.activeFocus ? root.accent : root.line }
                }
                TextField {
                    id: searchField; objectName: "fileManagerSearchField"
                    Layout.preferredWidth: 245; placeholderText: "Search this folder"; leftPadding: 34
                    onTextChanged: root.searchText = text
                    background: Rectangle { radius: 7; color: root.surface; border.color: searchField.activeFocus ? root.accent : root.line }
                    Icon { name: "search"; size: 18; color: root.muted; anchors.left: parent.left; anchors.leftMargin: 10; anchors.verticalCenter: parent.verticalCenter }
                }
            }
        }

        Connections {
            target: root.controller
            function onCurrentPathChanged() { pathField.text = root.controller.currentPath; root.selectedEntryPath = ""; root.selectedEntryName = "" }
            function onEntriesChanged() {
                if (root.selectedEntryPath.length && !root.controller.entries.some(function(e) { return e.path === root.selectedEntryPath })) {
                    root.selectedEntryPath = ""; root.selectedEntryName = ""
                }
            }
        }
        Shortcut { objectName: "fileManagerFocusPathShortcut"; sequence: "Ctrl+L"; enabled: root.visible; onActivated: { pathField.forceActiveFocus(); pathField.selectAll() } }
        Shortcut { objectName: "fileManagerRefreshShortcut"; sequence: "F5"; enabled: root.visible; onActivated: root.controller.refresh() }
        Shortcut { sequence: "Delete"; enabled: root.visible && root.selectedEntryPath.length > 0; onActivated: deleteDialog.open() }
        Shortcut { sequence: "Return"; enabled: root.visible && root.selectedEntryPath.length > 0; onActivated: root.openSelected() }

        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
            // Explorer navigation pane, not a global application toolbar.
            Rectangle {
                Layout.preferredWidth: 184; Layout.fillHeight: true; color: root.surfaceAlt; border.color: root.line
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 5
                    Label { text: "Quick access"; color: root.muted; font.pixelSize: 11; font.bold: true; leftPadding: 9; bottomPadding: 6 }
                    Button {
                        objectName: "fileManagerNavHome"; Layout.fillWidth: true; text: "Home"; icon.name: "home"; flat: true
                        onClicked: root.controller.goHome()
                        background: Rectangle { radius: 6; color: parent.hovered ? root.surface : "transparent" }
                    }
                    Button {
                        objectName: "fileManagerNavFilesystem"; Layout.fillWidth: true; text: "File system"; icon.name: "drive"; flat: true
                        onClicked: root.controller.navigate("/")
                        background: Rectangle { radius: 6; color: parent.hovered ? root.surface : "transparent" }
                    }
                    Rectangle { Layout.fillWidth: true; Layout.topMargin: 7; Layout.bottomMargin: 7; height: 1; color: root.line }
                    Label { text: "Current location"; color: root.muted; font.pixelSize: 11; font.bold: true; leftPadding: 9 }
                    Label {
                        Layout.fillWidth: true; Layout.leftMargin: 9; Layout.rightMargin: 7
                        text: root.controller.currentPath; color: root.fg; wrapMode: Text.WrapAnywhere; font.pixelSize: 11
                    }
                    Item { Layout.fillHeight: true }
                    Label { text: root.controller.entries.length + " items"; color: root.muted; font.pixelSize: 10; leftPadding: 9 }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 36; color: root.surface
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 14; anchors.rightMargin: 14; spacing: 8
                        Label { text: "Name"; color: root.muted; Layout.fillWidth: true }
                        Label { text: "Date modified"; color: root.muted; width: 150 }
                        Label { text: "Type"; color: root.muted; width: 100 }
                        Label { text: "Size"; color: root.muted; width: 82; horizontalAlignment: Text.AlignRight }
                    }
                }
                Rectangle { Layout.fillWidth: true; height: 1; color: root.line }
                ListView {
                    id: entriesView; objectName: "fileManagerEntriesView"
                    Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                    model: root.controller.entries
                    spacing: root.iconView ? 0 : 1
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                    delegate: Item {
                        id: entryDelegate
                        required property var modelData
                        required property int index
                        property bool selected: root.selectedEntryPath === modelData.path
                        property bool matches: root.filtered(modelData)
                        width: ListView.view.width
                        height: !matches ? 0 : (root.iconView ? 112 : 42)
                        visible: matches
                        Rectangle {
                            anchors.fill: parent; radius: root.iconView ? 8 : 4
                            color: entryDelegate.selected ? (root.lightAppearance ? "#d9eaff" : "#263c59") : (rowHover.hovered ? root.surfaceAlt : "transparent")
                            border.color: entryDelegate.selected ? root.accent : "transparent"
                        }
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 12; spacing: 10
                            visible: !root.iconView
                            Icon { name: modelData.isDir ? "folder" : "file"; size: 23; color: modelData.isDir ? (root.lightAppearance ? "#2475d7" : "#82baff") : root.muted }
                            Label { text: modelData.name; color: root.fg; elide: Text.ElideMiddle; Layout.fillWidth: true }
                            Label { text: modelData.modified; color: root.muted; font.pixelSize: 11; width: 150; elide: Text.ElideRight }
                            Label { text: modelData.isDir ? "File folder" : (modelData.name.lastIndexOf(".") > 0 ? modelData.name.substring(modelData.name.lastIndexOf(".")+1).toUpperCase() + " file" : "File"); color: root.muted; font.pixelSize: 11; width: 100; elide: Text.ElideRight }
                            Label { text: modelData.isDir ? "—" : modelData.sizeText; color: root.muted; font.pixelSize: 11; width: 82; horizontalAlignment: Text.AlignRight }
                        }
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 10; spacing: 6
                            visible: root.iconView
                            Icon { Layout.alignment: Qt.AlignHCenter; name: modelData.isDir ? "folder" : "file"; size: 34; color: modelData.isDir ? (root.lightAppearance ? "#2475d7" : "#82baff") : root.muted }
                            Label { Layout.fillWidth: true; text: modelData.name; color: root.fg; horizontalAlignment: Text.AlignHCenter; elide: Text.ElideMiddle; maximumLineCount: 2; wrapMode: Text.Wrap; font.pixelSize: 11 }
                        }
                        HoverHandler { id: rowHover }
                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onTapped: function(eventPoint, button) { root.choose(modelData); if (tapCount === 2) root.controller.openEntry(modelData.path) }
                        }
                        TapHandler {
                            acceptedButtons: Qt.RightButton
                            onTapped: { root.choose(modelData); entryContextMenu.popup() }
                        }
                        Menu {
                            id: entryContextMenu
                            MenuItem { text: "Open"; enabled: modelData.isDir || modelData.readable; onTriggered: root.controller.openEntry(modelData.path) }
                            MenuItem { text: "Rename"; onTriggered: { root.choose(modelData); renameField.text = modelData.name; renameDialog.open() } }
                            MenuItem { text: "Copy to…"; onTriggered: { root.choose(modelData); root.selectedEntryForMove = false; destinationField.text = root.controller.currentPath; transferDialog.open() } }
                            MenuItem { text: "Move to…"; onTriggered: { root.choose(modelData); root.selectedEntryForMove = true; destinationField.text = root.controller.currentPath; transferDialog.open() } }
                            MenuSeparator {}
                            MenuItem { text: "Delete"; onTriggered: { root.choose(modelData); deleteDialog.open() } }
                        }
                    }
                    Label {
                        anchors.centerIn: parent; visible: root.controller.entries.length === 0 || root.controller.entries.every(function(e) { return !root.filtered(e) })
                        text: root.searchText.length ? "No items match your search" : "This folder is empty"
                        color: root.muted; font.pixelSize: 14
                    }
                }
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 30; color: root.surfaceAlt; border.color: root.line
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                        Label { text: root.controller.status; color: root.muted; font.pixelSize: 10; Layout.fillWidth: true; elide: Text.ElideRight }
                        Label { text: root.controller.entries.length + " items"; color: root.muted; font.pixelSize: 10 }
                    }
                }
            }
        }
    }

    Dialog {
        id: transferDialog; width: 470; modal: true; title: root.selectedEntryForMove ? "Move item" : "Copy item"
        standardButtons: Dialog.Ok | Dialog.Cancel
        contentItem: ColumnLayout {
            width: 420; spacing: 10
            Label { text: (root.selectedEntryForMove ? "Move " : "Copy ") + root.selectedEntryName + " to:"; color: root.fg; wrapMode: Text.WordWrap }
            TextField { id: destinationField; Layout.fillWidth: true; placeholderText: "Destination folder path"; selectByMouse: true }
            Label { text: "Existing destinations will not be overwritten."; color: root.muted; wrapMode: Text.WordWrap }
        }
        onAccepted: {
            if (root.selectedEntryForMove) root.controller.moveEntry(root.selectedEntryPath, destinationField.text)
            else root.controller.copyEntry(root.selectedEntryPath, destinationField.text)
        }
    }
    Dialog {
        id: createDialog; modal: true; title: "New folder"; standardButtons: Dialog.Ok | Dialog.Cancel
        contentItem: TextField { id: folderNameField; placeholderText: "Folder name"; selectByMouse: true }
        onAccepted: root.controller.createFolder(folderNameField.text)
    }
    Dialog {
        id: renameDialog; modal: true; title: "Rename item"; standardButtons: Dialog.Ok | Dialog.Cancel
        contentItem: TextField { id: renameField; placeholderText: "New name"; selectByMouse: true }
        onAccepted: root.controller.renameEntry(root.selectedEntryPath, renameField.text)
    }
    Dialog {
        id: deleteDialog; modal: true; title: "Delete item?"; standardButtons: Dialog.Yes | Dialog.No; width: 430
        contentItem: ColumnLayout {
            width: 390; spacing: 8
            Label { text: "Are you sure you want to permanently delete this item?"; color: root.fg; wrapMode: Text.WordWrap }
            Label { text: root.selectedEntryName; color: root.muted; wrapMode: Text.WrapAnywhere }
            Label { text: root.selectedEntryIsDir ? "The folder and its contents will be removed." : "This file will be removed."; color: root.muted; wrapMode: Text.WordWrap }
        }
        onAccepted: root.controller.deleteEntry(root.selectedEntryPath, root.selectedEntryIsDir)
    }
}
