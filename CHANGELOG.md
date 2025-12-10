# Changelog – FavList-App

## [2025-11-29] – Initialer Commit: FavList-App

### Status

Die erste funktionsfähige Version der **FavList-App** ist stabil und vollständig lauffähig.

### Umgesetzte Funktionen

* **Icons integriert:** Alle Favoriten werden mit Icons dargestellt.
* **Pfeilbasiertes Verschieben:** Einträge können über Aufwärts-/Abwärts-Pfeile zuverlässig verschoben werden.
* **Sichtbares App-Icon:** Das Icon der App erscheint korrekt in der Taskleiste – aktuell ein Stern.
* **Stabile Grundfunktionen:** Die gesamte Basislogik (Laden, Speichern, Anzeigen der Favoriten) läuft sauber und fehlerfrei.

### Geplante nächste Schritte

* **Drag & Drop Unterstützung:** Statt Pfeiltasten soll eine Drag-and-Drop-Sortierung eingeführt werden.
* **Autostart der App:** Die App soll wie X-Pad automatisch in der Taskleiste starten, sobald der PC hochfährt.

### [2025-12-04] –Verbesserungen am Einstellungsfenster + Reset-Funktion für Fenstergröße

- Reset-Funktion für die Fenstergröße implementiert (resetToDefaultSize())
  → stellt das Fenster jederzeit auf die Standardmaße 250×400 zurück.
- Button „Fenstergröße zurücksetzen“ ins Einstellungsfenster integriert.
- Ursprüngliche Versuche zur optischen Hervorhebung des Buttons
  (custom background, eigenes Rectangle, RowLayout) wieder verworfen,
  um maximale Kompatibilität mit Qt-Styles (Breeze, Mint-Y etc.) zu
  gewährleisten.
- Buttons im Settings-Fenster bewusst schlicht gelassen, analog zu
  KDE/Plasmoid-Designrichtlinien.
- Experiment mit Qt.FramelessWindowHint und eigenem Popup-Hintergrund
  (runde Ecken außen) durchgeführt, aber verworfen – Rückkehr zur
  normalen Fensterdekoration, da diese ein konsistentes Verhalten unter
  KDE gewährleistet und die Bedienbarkeit verbessert.
- Popup-Verhalten über dem Panel bleibt unverändert funktional.

Ergebnis:
Stabilere, optisch konsistente Einstellungen; sauberer Reset-Button;
keine Konflikte mit Breeze-Dekorationen; keine Frameless-Seiteneffekte.
Das Fenster behält nun die KDE-typische Erscheinung und arbeitet dennoch
als Tray-Popup mit allen bisherigen Vorteilen.
### [2025-12-09] – UI: Theme-System überarbeitet, Dark/Light Fix, Add-Button optimiert

UI & Theme-Verbesserungen

- Zentrales Theme-Objekt eingeführt:
  - Einheitliche Farben für Dark/Light Mode
  - Keine Abhängigkeit mehr von SystemPalette
  - Bessere Lesbarkeit unter KDE, GNOME und Ubuntu

- Dark/Light-Mode:
  - Farbe wird korrekt aus Qt.styleHints.colorScheme übernommen
  - Theme steuert nun Fensterhintergrund, Textfarben, Hover-Farben usw.
  - GNOME/Ubuntu zeigen jetzt lesbare Inhalte im Dunkelmodus

- Add-Button überarbeitet:
  - Großer "Hinzufügen"-Button entfernt
  - Kleiner Button ("↵" bzw. "+") wurde neben dem Textfeld ergänzt
  - Enter-Taste im Textfeld fügt nun ebenfalls hinzu
  - Neue Funktion addCurrentPath() eingeführt

- Settings-Button (Zahnrad):
  - Icon-Farbe wird nun aus Theme übernommen
  - Bleibt im Dark-Mode sichtbar, auch ohne Hover

- Placeholder-Text im TextField:
  - erhält nun theme.textSecondary, damit im Dark-Mode sichtbar

- Button-Styling:
  - Hintergrundfarben, Ränder und Hover-Effekte vereinheitlicht
  - Optik wirkt kompakter und moderner

- Entfernte Elemente:
  - Alter Button "Hinzufügen" unterhalb der Liste wurde komplett entfernt

- Diverse Aufräumarbeiten:
  - SystemPalette entfernt
  - Doppeltes Farb-Handling aus QML bereinigt
  - required property in ListView Delegate entfernt
  
  
### [2025-12-10] Tray-Menü verbessert: Öffnen/Schließen-Toggle und Einstellungen gefixt

- Tray-Menü-Eintrag "Öffnen" toggelt jetzt das Hauptfenster über die QML-Funktion
  toggleVisibility() statt nur stumpf showOrActivateMainWindow() aufzurufen.
- Text des Tray-Menü-Eintrags wird dynamisch zwischen "Öffnen" und "Schließen"
  umgeschaltet, abhängig von window->visible (via visibleChanged-Signal).
- "Einstellungen..." im Tray-Menü ruft wieder korrekt die QML-Funktion
  openSettings() auf und öffnet das Einstellungsfenster.
- Verhalten ist jetzt konsistent mit dem Klick auf das Tray-Icon:
- Fenster sichtbar → Schließen
- Fenster verborgen → Öffnen


