import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property var controller
    property color foreground: "#f4f6fb"
    property color muted: "#aab4c4"
    property color panel: "#191f29"
    property color borderColor: "#2b3340"
    property bool lightMode: false

    readonly property color cardText: foreground
    readonly property color cardMuted: muted
    readonly property color cardBackground: panel
    readonly property color cardBorder: borderColor

    Flickable {
        anchors.fill: parent
        contentWidth: width
        contentHeight: contentColumn.implicitHeight + 16
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: contentColumn
            width: parent.width
            spacing: 14

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 180
                radius: 16
                color: page.lightMode ? "#e7edff" : "#172541"
                border.color: page.lightMode ? "#c4d3f7" : "#314c7c"
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 22
                    spacing: 20
                    Rectangle {
                        width: 70; height: 70; radius: 20
                        color: "#3978d4"
                        Label { anchors.centerIn: parent; text: "◉"; font.pixelSize: 38; color: "white" }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        Label { text: "AI Screen Share"; color: page.cardText; font.pixelSize: 23; font.bold: true }
                        Label {
                            Layout.fillWidth: true
                            text: "Create a private screen session and connect it to an AI client through The End MCP."
                            color: page.cardMuted; wrapMode: Text.WordWrap
                        }
                        Label { text: "SCREEN CAPTURE STARTS ONLY AFTER YOU APPROVE IT IN THE HOST BROWSER."; color: page.cardText; font.pixelSize: 10; font.bold: true; wrapMode: Text.WordWrap }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 132
                radius: 13; color: page.cardBackground; border.color: page.cardBorder
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 18; spacing: 8
                    Label { text: "1 · Create MCP session"; color: page.cardText; font.pixelSize: 17; font.bold: true }
                    Label {
                        Layout.fillWidth: true
                        text: "Create MCP asks the existing Screen Share service for a new session with separate host and viewer credentials. It does not start capturing your screen by itself."
                        color: page.cardMuted; wrapMode: Text.WordWrap
                    }
                    Button {
                        objectName: "createMcpButton"
                        text: controller.busy ? "Creating session…" : (controller.ready ? "Create another MCP session" : "Create MCP")
                        enabled: !controller.busy
                        onClicked: controller.createMcp()
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: controller.ready ? 224 : 100
                radius: 13; color: page.cardBackground; border.color: page.cardBorder
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 18; spacing: 8
                    Label { text: "2 · Connect this computer"; color: page.cardText; font.pixelSize: 17; font.bold: true }
                    Label { text: controller.status; color: page.cardMuted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    RowLayout {
                        visible: controller.ready
                        Layout.fillWidth: true; spacing: 8
                        Button { text: "Open host page"; onClicked: controller.openHostLink() }
                        Button { text: "Copy host link"; onClicked: controller.copyText(controller.hostUrl) }
                        Button { text: "End session"; enabled: !controller.busy; onClicked: controller.endSession() }
                    }
                    TextField {
                        visible: controller.ready
                        Layout.fillWidth: true; readOnly: true; selectByMouse: true
                        text: controller.hostUrl; placeholderText: "Host link appears after session creation"
                    }
                    Label { visible: controller.ready; text: "Session expiry (UTC): " + controller.expiresAt; color: page.cardMuted; font.pixelSize: 10 }
                    Label {
                        visible: controller.ready; Layout.fillWidth: true
                        text: "Open the host page on the computer whose screen you want to share. Press Share screen in the browser and choose the screen/window/tab yourself."
                        color: page.cardMuted; wrapMode: Text.WordWrap
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: controller.ready ? 250 : 100
                radius: 13; color: page.cardBackground; border.color: page.cardBorder
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 18; spacing: 8
                    Label { text: "3 · Connect AI through MCP"; color: page.cardText; font.pixelSize: 17; font.bold: true }
                    Label {
                        Layout.fillWidth: true
                        text: "Add the endpoint to an AI client that supports remote MCP, complete its sign-in flow, then provide the session details. The snapshot tool only exposes the screen after browser sharing has been approved."
                        color: page.cardMuted; wrapMode: Text.WordWrap
                    }
                    TextField { Layout.fillWidth: true; readOnly: true; selectByMouse: true; text: controller.mcpEndpoint }
                    TextField {
                        visible: controller.ready
                        Layout.fillWidth: true; readOnly: true; selectByMouse: true
                        text: controller.viewerUrl; placeholderText: "Private viewer link appears after session creation"
                    }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 8
                        Button { text: "Open MCP endpoint"; onClicked: controller.openMcpEndpoint() }
                        Button { text: "Copy MCP endpoint"; onClicked: controller.copyText(controller.mcpEndpoint) }
                        Button { visible: controller.ready; text: "Copy viewer link"; onClicked: controller.copyText(controller.viewerUrl) }
                        Button { visible: controller.ready; text: "Copy AI connection details"; onClicked: controller.copyText(controller.connectionInstructions()) }
                    }
                    Label {
                        visible: controller.ready; Layout.fillWidth: true
                        text: "The connection details contain a private viewer token. Share them only with the AI client you trust; creating a session does not configure an AI client automatically."
                        color: page.cardMuted; wrapMode: Text.WordWrap; font.pixelSize: 11
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 100
                radius: 13; color: page.lightMode ? "#fff4df" : "#332819"; border.color: page.lightMode ? "#e8ce9b" : "#70522c"
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 16; spacing: 5
                    Label { text: "Privacy and safety"; color: page.cardText; font.bold: true }
                    Label {
                        Layout.fillWidth: true
                        text: "Only share a screen you own or are authorized to share. The host browser displays the system's screen-picker prompt; stop sharing from the browser when finished. Session links and viewer tokens are credentials."
                        color: page.cardMuted; wrapMode: Text.WordWrap; font.pixelSize: 11
                    }
                }
            }
        }
    }
}
