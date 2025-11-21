import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 250
    height: 400
    title: "FavList"

    // Dark-/Light-Theme-Erkennung
    property bool darkTheme: Qt.styleHints.colorScheme === Qt.Dark

    // Wird vom Tray genutzt, um Fenster ein-/auszublenden
    function toggleVisibility() {
        if (visible) {
            hide();
        } else {
            show();
            raise();
            requestActivate();
        }
    }

    // Wird aus C++ (Tray) aufgerufen
    function openSettings() {
        settingsWindow.show();
        settingsWindow.raise();
        settingsWindow.requestActivate();
    }

    onClosing: {
        close.accepted = false      // verhindert echtes Schließen
        hide()
    }

    // 🔹 System-Palette vom aktuellen Theme holen
    SystemPalette {
        id: sysPalette
        colorGroup: SystemPalette.Active
    }

    // 🔹 Eigenes Einstellungsfenster mit Systemfarben
    Window {
        id: settingsWindow
        width: 320
        height: 180
        title: "Einstellungen – FavList"
        modality: Qt.ApplicationModal
        flags: Qt.Dialog | Qt.WindowCloseButtonHint
        visible: false

        // Hintergrund an System-Theme koppeln
        color: sysPalette.window

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 8

            CheckBox {
                id: autostartCheck
                text: "Beim Systemstart starten"
                checked: autostartManager.isAutostartEnabled()
                onToggled: autostartManager.setAutostartEnabled(checked)
            }

            Label {
                text: "(Weitere Optionen folgen …)"
                opacity: 0.6
                color: sysPalette.windowText
            }

            Item {
                Layout.fillHeight: true
            }

            Button {
                text: "Schließen"
                Layout.alignment: Qt.AlignRight
                onClicked: settingsWindow.close()
            }
        }
    }

    // 🔹 Hauptinhalt der App (dein ursprüngliches Layout)
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
            Layout.fillWidth: true
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
