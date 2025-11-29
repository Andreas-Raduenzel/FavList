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

    onClosing: function(close) {
        if (trayAvailable) {
            close.accepted = false
            hide()
        } else {
            close.accepted = true
        }
    }

    // Shortcut für Einstellungen (z.B. Strg+,)
    Shortcut {
        sequence: StandardKey.Preferences
        onActivated: mainWindow.openSettings()
    }

    // 🔹 System-Palette vom aktuellen Theme holen
    SystemPalette {
        id: sysPalette
        colorGroup: SystemPalette.Active
    }

    // Overlay für visuelles Feedback beim Datei-Drag von außen
    Rectangle {
        id: dragOverlay
        anchors.fill: parent
        color: darkTheme ? "#40ffffff" : "#40000000" // halbtransparent
        visible: dropArea.containsDrag               // nur sichtbar, wenn gerade etwas „drüber“ ist
        z: 98
        border.width: 2
        border.color: darkTheme ? "white" : "black"

        Text {
            anchors.centerIn: parent
            text: "Datei hierher ziehen, um sie zur Liste hinzuzufügen"
            color: darkTheme ? "white" : "black"
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            width: parent.width * 0.8
        }
    }

    // DropArea über das ganze Fenster für neue Favoriten
    DropArea {
        id: dropArea
        anchors.fill: parent
        z: 99
        keys: ["text/uri-list"]    // Dateien vom Dateimanager

        onEntered: function(drag) {
            if (drag.hasUrls) {
                drag.acceptProposedAction();
            }
        }

        onDropped: function(drop) {
            if (!drop.hasUrls)
                return;

            for (let i = 0; i < drop.urls.length; ++i) {
                backend.addFavoriteFromUrl(drop.urls[i]);
            }
        }
    }

    // Einstellungsfenster
    Window {
        id: settingsWindow
        width: 320
        height: 200
        title: "Einstellungen – FavList"
        modality: Qt.ApplicationModal
        flags: Qt.Dialog | Qt.WindowCloseButtonHint
        visible: false

        color: sysPalette.window

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 8

            CheckBox {
                id: autostartCheck
                text: "Beim Systemstart automatisch starten"
                checked: autostartManager.isAutostartEnabled()

                onToggled: {
                    autostartManager.setAutostartEnabled(checked)
                    if (!checked) {
                        autostartManager.setStartOnlyTray(false)
                    }
                }
            }

            CheckBox {
                text: "Beim Start nur Tray-Icon anzeigen"
                visible: trayAvailable
                enabled: autostartCheck.checked
                checked: autostartManager.startOnlyTray()

                onToggled: autostartManager.setStartOnlyTray(checked)
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

    // Hauptinhalt der App
    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        anchors.margins: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            TextField {
                id: pathInput
                placeholderText: "Pfad eingeben oder Datei auf das Fenster ziehen..."
                Layout.fillWidth: true
                onAccepted: {
                    if (text.length > 0) {
                        backend.addFavorite(text)
                        text = ""
                    }
                }
            }

            ToolButton {
                text: "⚙"
                Accessible.name: "Einstellungen"
                onClicked: mainWindow.openSettings()
                ToolTip.visible: hovered
                ToolTip.text: "Einstellungen öffnen"
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

            // Delegate: ein Eintrag + DragHandler für Reordering
            delegate: Item {
                id: rowItem
                width: listView.width - (scroll.visible ? 25 : 0)
                height: 36

                required property int index
                required property string modelData

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.width: dragHandler.active ? 1 : 0
                    border.color: dragHandler.active
                                   ? (darkTheme ? "#8fb1ff" : "#3355ff")
                                   : "transparent"
                    radius: 4

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

                // Drag zum Verschieben, ohne Items optisch zu verschieben
                DragHandler {
                    id: dragHandler
                    acceptedButtons: Qt.LeftButton

                    onActiveChanged: {
                        if (!active) {
                            const center = Qt.point(rowItem.width / 2, rowItem.height / 2)
                            const posInView = listView.mapFromItem(rowItem, center.x, center.y)
                            const newIndex = listView.indexAt(posInView.x, posInView.y)

                            if (newIndex >= 0 && newIndex !== index) {
                                backend.moveFavorite(index, newIndex)
                            }
                        }
                    }
                }
            }
        }
    }
}
