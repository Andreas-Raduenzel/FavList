import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 250
    height: 400
    title: "FavList"
    

    // Dark-/Light-Theme-Erkennung
    property bool darkTheme: Qt.styleHints.colorScheme === Qt.Dark

    // Wird vom Tray (oder später) genutzt, um Fenster ein-/auszublenden
    function toggleVisibility() {
        if (visible) {
            hide();
        } else {
            show();
            raise();
            requestActivate();
        }
    }

    // Beim Klick auf das X nicht wirklich beenden, sondern nur ins Tray
    onClosing: {
        close.accepted = false      // verhindert echtes Schließen
        hide()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        anchors.margins: 10

        TextField {
            id: pathInput
            placeholderText: "Pfad zur Datei oder zum Ordner eingeben..."
            Layout.fillWidth: true
            onAccepted: {
                if (text.length > 0) {
                    backend.addFavorite(text)
                    text = ""
                }
            }
        }

        Button {
            text: "Hinzufügen"
            onClicked: {
                if (pathInput.text.length > 0) {
                    backend.addFavorite(pathInput.text)
                    pathInput.text = ""
                }
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4
            model: backend.favorites
            clip: true

            rightMargin: scroll.visible ? 25 : 0

            ScrollBar.vertical: ScrollBar {
                id: scroll
                policy: ScrollBar.AlwaysOn
                visible: listView.count > 0 && listView.contentHeight > listView.height
            }

            delegate: Rectangle {
                width: parent.width - (scroll.visible ? 25 : 0)
                height: 36
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    spacing: 8

                    // Hoch-Pfeil
                    Label {
                        text: "⬆"
                        font.pixelSize: 14
                        verticalAlignment: Text.AlignVCenter
                        enabled: index > 0
                        opacity: enabled ? 1.0 : 0.3

                        MouseArea {
                            anchors.fill: parent
                            enabled: parent.enabled
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.moveFavorite(index, index - 1)
                        }
                    }

                    // Runter-Pfeil
                    Label {
                        text: "⬇"
                        font.pixelSize: 14
                        verticalAlignment: Text.AlignVCenter
                        enabled: index < listView.count - 1
                        opacity: enabled ? 1.0 : 0.3

                        MouseArea {
                            anchors.fill: parent
                            enabled: parent.enabled
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.moveFavorite(index, index + 1)
                        }
                    }

                    Image {
                        source: backend.iconPathForFile(modelData)
                        width: 20
                        height: 20
                        fillMode: Image.PreserveAspectFit
                        Layout.alignment: Qt.AlignVCenter
                    }

                    Text {
                        text: modelData.split("/").pop()
                        color: darkTheme ? "white" : "black"
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Qt.openUrlExternally(modelData)
                        }
                    }

                    Label {
                        text: "❌"
                        font.pixelSize: 16
                        color: darkTheme ? "white" : "black"
                        verticalAlignment: Text.AlignVCenter
                        padding: 4

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: backend.removeFavorite(modelData)
                        }
                    }
                }
            }
        }
    }
}
