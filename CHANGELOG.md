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
