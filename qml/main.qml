import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

ApplicationWindow {
    id: mainWindow
    visible: false
    width: 250
    height: 400
    title: "FavList"

    // Fensterverhalten je nach Umgebung:
    // - Ohne Tray  ODER unter GNOME/Ubuntu: normales Fenster
    // - Mit Tray und nicht GNOME: kleines Tool-Popup über der Leiste
    flags: (!trayAvailable || isGnomeLike)
           ? (Qt.Window
              | Qt.WindowMinimizeButtonHint
              | Qt.WindowMaximizeButtonHint
              | Qt.WindowCloseButtonHint)
           : (Qt.Tool
              | Qt.WindowCloseButtonHint
              | Qt.WindowStaysOnTopHint)

    // 🔹 Standardgröße merken
    property int defaultWidth: 250
    property int defaultHeight: 400

    // 🔹 Funktion, um jederzeit auf die Standardgröße zurückzuspringen
    function resetToDefaultSize() {
        width = defaultWidth
        height = defaultHeight
    }
    
    // 🔹 Funktion, um den aktuellen Pfad im Eingabefeld hinzuzufügen
    function addCurrentPath() {
        if (pathInput.text.length > 0) {
            backend.addFavorite(pathInput.text)
            pathInput.text = ""
        }
    }
    


    // Dark-/Light-Theme-Erkennung vom System
    property bool darkTheme: Qt.styleHints.colorScheme === Qt.Dark

    // Zentrales Theme-Objekt für alle Farben
    QtObject {
        id: theme

        property bool dark: darkTheme

        // Grundfarben
        property color window:        dark ? "#252525" : "#ffffff"
        property color text:          dark ? "#f0f0f0" : "#202020"
        property color textSecondary: dark ? "#c0c0c0" : "#404040"

        // Rahmen / Hervorhebungen
        property color borderStrong:  dark ? "#ffffff" : "#000000"

        // Drag-Overlay
        property color dragOverlay:   dark ? "#40ffffff" : "#40000000"

        // Liste / Hover / Auswahl
        property color itemHover:     dark ? "#404860" : "#d9e6ff"
        property color itemBorder:    dark ? "#8fb1ff" : "#3355ff"

        // Einfüge-Linie
        property color insertLine:    dark ? "#5b8dff" : "#3366ff"
    }

    // Fensterhintergrund nach Theme
    color: theme.window

    function toggleVisibility() {
        if (visible) {
            hide();
        } else {
            show();
            raise();
            requestActivate();
        }
    }

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

    Shortcut {
        sequence: StandardKey.Preferences
        onActivated: mainWindow.openSettings()
    }

    // SystemPalette entfernen – wir nutzen unser eigenes Theme
    // SystemPalette {
    //     id: sysPalette
    //     colorGroup: SystemPalette.Active
    // }

    Rectangle {
        id: dragOverlay
        anchors.fill: parent
        color: theme.dragOverlay
        visible: dropArea.containsDrag
        z: 98
        border.width: 2
        border.color: theme.borderStrong

        Text {
            anchors.centerIn: parent
            text: "Datei hierher ziehen, um sie zur Liste hinzuzufügen"
            color: theme.text
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            width: parent.width * 0.8
        }
    }

    DropArea {
        id: dropArea
        anchors.fill: parent
        z: 99
        keys: ["text/uri-list"]

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

    Window {
        id: settingsWindow
        width: 320
        height: 200
        title: "Einstellungen – FavList"
        modality: Qt.ApplicationModal
        flags: Qt.Dialog | Qt.WindowCloseButtonHint
        visible: false

        // statt sysPalette: eigenes Theme
        color: theme.window

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

            Item {
                Layout.fillHeight: true
            }

            Button {
                text: "Fenstergröße zurücksetzen"
                Layout.alignment: Qt.AlignLeft
                onClicked: {
                    mainWindow.resetToDefaultSize()
                }
            }

            Button {
                text: "Schließen"
                Layout.alignment: Qt.AlignRight
                onClicked: settingsWindow.close()
            }
        }
    }

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

                color: theme.text
                placeholderTextColor: theme.textSecondary

                background: Rectangle {
                    radius: 4
                    border.width: 1
                    border.color: theme.textSecondary
                    color: theme.window
                }

                onAccepted: {
                    mainWindow.addCurrentPath()
                }
            }

            // Kleiner Button direkt neben dem Textfeld
            Button {
                id: quickAddButton
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: 32
                Layout.preferredHeight: 28

                contentItem: Text {
                    // hier kannst du auch "+" statt "↵" nehmen
                    text: "↵"
                    color: theme.text
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 16
                }

                background: Rectangle {
                    radius: 4
                    border.width: 1
                    border.color: theme.textSecondary
                    color: quickAddButton.down
                        ? theme.itemBorder
                        : quickAddButton.hovered
                            ? theme.itemHover
                            : (theme.dark ? "#404040" : "#e0e0e0")
                }

                onClicked: mainWindow.addCurrentPath()
            }

            ToolButton {
                id: settingsButton
                Accessible.name: "Einstellungen"
                onClicked: mainWindow.openSettings()
                ToolTip.visible: hovered
                ToolTip.text: "Einstellungen öffnen"

                contentItem: Text {
                    text: "⚙"
                    color: theme.text
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 16
                }

                background: Rectangle {
                    implicitWidth: 24
                    implicitHeight: 24
                    radius: 4
                    color: settingsButton.down || settingsButton.hovered
                        ? theme.itemHover
                        : "transparent"
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

            // nur Scrollen per Mausrad/Scrollbar, nicht per Drag-Geste der Liste
            interactive: false

            // Einfüge-Info für Drag
            property int  dragInsertIndex: -1
            property bool dragging: false

            rightMargin: scroll.visible ? 25 : 0

            ScrollBar.vertical: ScrollBar {
                id: scroll
                policy: ScrollBar.AlwaysOn
                visible: listView.count > 0 && listView.contentHeight > listView.height
            }

            delegate: Item {
                id: rowItem
                width: listView.width - (scroll.visible ? 25 : 0)
                height: 36

                // index und modelData liefert ListView automatisch,
                // "required property" ist hier nicht nötig und kann sogar stören
                // required property int index
                // required property string modelData

                // für Drag-Erkennung
                property bool wasDrag: false
                property real pressY: 0

                Rectangle {
                    anchors.fill: parent

                    // Hover-/Press-Highlight
                    color: (rowMouse.containsMouse || rowMouse.pressed)
                           ? theme.itemHover
                           : "transparent"

                    border.width: rowMouse.wasDrag ? 1 : 0
                    border.color: rowMouse.wasDrag
                                   ? theme.itemBorder
                                   : "transparent"
                    radius: 4

                    // Einfüge-Linie oben im Ziel-Item
                    Rectangle {
                        id: insertLine
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        height: 2
                        color: theme.insertLine

                        // Linie nur, wenn wirklich Drag läuft und dieses Item Ziel ist
                        visible: listView.dragging && listView.dragInsertIndex === index
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.topMargin: 2
                        spacing: 8

                        Image {
                            source: backend.iconPathForFile(modelData)
                            width: 20
                            height: 20
                            fillMode: Image.PreserveAspectFit
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Text {
                            text: modelData.split("/").pop()
                            color: theme.text
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            text: "❌"
                            font.pixelSize: 16
                            color: theme.text
                            Layout.alignment: Qt.AlignVCenter

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor

                                onClicked: backend.removeFavorite(modelData)

                                // Klick fürs Löschen abfangen
                                onPressed: {
                                    mouse.accepted = true;
                                }
                            }
                        }
                    }

                    // gesamte Zeile (ohne ❌): Klick = öffnen, Drag = verschieben
                    MouseArea {
                        id: rowMouse
                        hoverEnabled: true
                        cursorShape: wasDrag ? Qt.SizeAllCursor : Qt.PointingHandCursor

                        anchors {
                            left: parent.left
                            top: parent.top
                            bottom: parent.bottom
                            right: parent.right
                            rightMargin: 30   // Platz fürs ❌
                        }

                        onPressed: {
                            rowItem.pressY = mouse.y
                            rowItem.wasDrag = false
                            listView.dragging = false
                            listView.dragInsertIndex = -1
                        }

                        onPositionChanged: {
                            // Nur reagieren, wenn linke Maustaste gedrückt ist
                            if (!(mouse.buttons & Qt.LeftButton))
                                return;

                            // ab kleiner Schwelle als Drag werten
                            if (!rowItem.wasDrag &&
                                Math.abs(mouse.y - rowItem.pressY) > 4) {
                                rowItem.wasDrag = true
                                listView.dragging = true
                            }

                            if (rowItem.wasDrag) {
                                var dy = mouse.y - rowItem.pressY
                                var rowStep = rowItem.height + listView.spacing

                                var deltaRows = Math.round(dy / rowStep)
                                var candidate = index + deltaRows

                                if (candidate < 0)
                                    candidate = 0
                                if (candidate > listView.count - 1)
                                    candidate = listView.count - 1

                                listView.dragInsertIndex = candidate
                            }
                        }

                        onReleased: {
                            if (rowItem.wasDrag) {
                                var targetIndex = listView.dragInsertIndex

                                // 🔹 Drag-Zustand zurücksetzen
                                rowItem.wasDrag = false
                                listView.dragging = false
                                listView.dragInsertIndex = -1

                                if (targetIndex < 0)
                                    targetIndex = index

                                if (targetIndex !== index) {
                                    backend.moveFavorite(index, targetIndex)
                                }

                            } else {
                                // 🔹 normaler Klick → Favorit öffnen
                                Qt.openUrlExternally(modelData)

                                // 🔹 Fenster automatisch schließen, aber nur wenn ein Tray existiert
                                if (trayAvailable) {
                                    mainWindow.hide()
                                }

                                // Zustand aufräumen
                                rowItem.wasDrag = false
                                listView.dragging = false
                                listView.dragInsertIndex = -1
                            }
                        }

                        onCanceled: {
                            rowItem.wasDrag = false
                            listView.dragging = false
                            listView.dragInsertIndex = -1
                        }

                        onExited: {
                            if (!(mouse.buttons & Qt.LeftButton)) {
                                rowItem.wasDrag = false
                                listView.dragging = false
                                listView.dragInsertIndex = -1
                            }
                        }
                    }
                }
            }
        }
    }
}
