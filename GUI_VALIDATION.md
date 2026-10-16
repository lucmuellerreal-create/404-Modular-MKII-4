# GUI-Validierung — 16.10.2026

## Ausgefuehrt

- Original-PNGs eingelesen: 667x539, 688x491, 684x546 Pixel.
- Alle drei angehaengten Bilder visuell analysiert; Gehaeuse, Panels,
  Regler, Fader, Anzeigen und Beschriftungspositionen in GUI_LAYOUT.md festgehalten.
- Geometrie in `Source/GUI/SkinDesign.cpp` mit der Spezifikation abgeglichen.
- Beim Quelltextabgleich korrigiert: rechter Schraubenabstand des 40G-Skins;
  vergroesserte 3DR-Regler-Zeichenflaeche, damit die aussenliegenden roten Ticks
  nicht an der Komponentengrenze abgeschnitten werden.
- Farbmasken der Originalbilder lokal ausgewertet: helle Reglerkappen liegen
  beim 4SX/40G um y=150, beim 3DR um y=164 (ohne Host-Leiste). Die anfangs um
  neun Pixel zu tiefe 3DR-Reglerposition wurde korrigiert; Textzeilen bleiben
  auf den gemessenen Referenzhoehen.
- Benutzerdefinierte Bypass-Anbindung mit `juce::ParameterAttachment` gegen
  JUCE 8.0.6 API abgeglichen. GUI-Zustand ist das Inverse von `enabled`.
- Fuer alle sichtbaren Regler und Fader sind APVTS-Attachments vorhanden.
- CMake-Preset-JSON, Python-Syntax und alle lokalen C++-Include-Pfade geprueft:
  12 Header und 12 Implementierungsdateien, keine fehlenden lokalen Includes.
- Python-Vergleichsskript syntaktisch geprueft und Hilfeausgabe aufgerufen.
- Release-Skript erneut gestartet: Exitcode 1 wegen fehlendem CMake.
- Erneut weder MSVC/Clang/GCC im PATH noch Windows-SDK am Standardpfad gefunden.

## Nicht ausgefuehrt

**Keine kompilierten GUI-Screenshots, kein visueller Vergleich einer laufenden
JUCE-GUI mit den Vorlagen, kein erfolgreicher Release-Build und kein FL-Studio-Test.**
Ohne CMake, MSVC und Windows-SDK lassen sich diese Schritte hier nicht ausfuehren.
Es werden keine Mockups als angebliche Plugin-Screenshots ausgegeben.

## Vorbereitete Pruefung

`Build-Release.ps1` baut jetzt auch `SPVinylLabGuiCapture`. CTest prueft damit:

- Regler -> APVTS und Host-Parameter -> GUI;
- Display-Endpunkte 000 / 127 und Zwischenwert 032;
- Input/Output-Fader bei -24 / 0 / +12 dB;
- Bypass-Polaritaet und Host-Rueckmeldung;
- wiederhergestellten Modus/Skin und Output-Gain;
- PNG-Export aller Skins in Referenzgroesse, 2x-Aufloesung und bei 150% GUI-Groesse.

Die Exporte werden aus dem echten `SPVinylLabEditor` per
`createComponentSnapshot` erzeugt, nicht aus einer separaten Nachzeichnung.
Die Parameterwerte entsprechen fuer diese Aufnahmen den Referenzbildern;
Meter werden mit einem kurzen deterministischen Audiosignal versorgt.
Die Tests rufen die Slider-/Button-Callbacks auf. Native Mausbewegung,
Monitor-DPI-Wechsel und FL-Studio-Host-Resize bleiben manuelle Pruefschritte.

Nach erfolgreichem Build liegen die PNGs in `build/GuiCaptures`. Vergleich:

```powershell
python -m pip install Pillow
python Tests/Compare-References.py --references "C:/Users/lucmu/Pictures/Screenshots" --captures build/GuiCaptures --output build/VisualComparison
```

Das Skript erzeugt drei nebeneinanderliegende Ansichten: beschnittene Referenz,
JUCE-Aufnahme und Pixeldifferenz. Es bricht bei fehlenden echten Aufnahmen ab.
Pixelfehlerwerte dienen nur als Diagnose, nicht als automatischer Qualitaetsnachweis.

## Bekannte optische Grenzen / offene Sichtpruefung

Die Vorlagen verwenden eine andere Schrift; diese Umsetzung verwendet eine
fette System-Sans-Schrift mit JUCE-Vektorrasterung. Logoformen und Laufweiten sind
eine Annaeherung. Fremde Marken-/Autorensignaturen sind durch SP VINYL LAB ersetzt.
Die kleine Moduswahl ist eine bewusste Ergaenzung. Metallstruktur ist prozedural,
keine Fototextur. Ringreflexe, Knopfschatten, Schriftbreiten und Abstaende muessen
nach dem ersten echten Renderlauf visuell feinjustiert werden.
