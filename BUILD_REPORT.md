# Build-Protokoll — 16.10.2026

## Ergebnis

Quellprojekt und Build-Dateien erstellt. **Kein erfolgreicher Compilerlauf,
keine gebaute VST3-Binaerdatei und keine ausgefuehrten C++-Tests.**
Die Kompilierbarkeit ist deshalb noch nicht durch einen Build bestaetigt.

## Ausgefuehrte Umgebungspruefung

- PowerShell: `Get-Command cmake,git,cl -ErrorAction SilentlyContinue`: keine Treffer.
- `where.exe git` und `where.exe cmake`: nicht gefunden.
- `C:\Program Files\CMake\bin\cmake.exe`: nicht vorhanden.
- `C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe`: nicht vorhanden.
- `C:\Program Files\Microsoft Visual Studio\2022`: kein vorhandener Installationsordner gefunden.

Eine Installation an anderen benutzerdefinierten Orten ist damit nicht ausgeschlossen.

## Release-Versuch

Ausgefuehrt aus dem uebergeordneten Arbeitsordner:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\outputs\SPVinylLab\Build-Release.ps1
```

Exitcode **1**. Der Build wurde bei der Werkzeugpruefung angehalten:

> CMake fehlt im PATH. Visual Studio 2022 mit Desktopentwicklung mit C++, MSVC v143, Windows SDK und CMake installieren oder dessen bin-Verzeichnis zum PATH hinzufuegen.

Vollstaendige Ausgabe: `BuildLogs/Release.log`.
Dies ist ein Umgebungsfehler vor der CMake-Konfiguration, kein C++-Compilerfehler.
Es liegen keine weiteren Compiler- oder Linkerdiagnosen vor.

## Korrektur / noch auszufuehren

1. VS 2022 C++-Toolchain, Windows SDK, CMake >= 3.22 und Git bereitstellen.
2. `Build-Release.ps1` erneut ausfuehren. Es konfiguriert VS 2022 x64,
   baut Release und startet die Regressionstests; jeder Schritt prueft den Exitcode.
3. Etwaige Compiler-/Linkerfehler anhand von `BuildLogs/Release.log` beheben.
4. VST3-Validierung und die FL-Studio-Pruefschritte aus README durchfuehren.

Werkzeuge wurden nicht systemweit installiert. Der fehlende Compiler kann nicht
durch Quellcodeaenderungen behoben werden; die notwendige lokale Einrichtung ist
in README beschrieben. JUCE wird bei der ersten Konfiguration geladen, daher
erfolgte hier auch kein lokaler Abgleich gegen JUCE-Header durch einen Compiler.

## Statische Pruefung

- CMake-Presets als JSON eingelesen.
- Angeforderte .h/.cpp-Dateien sowie DSP-/GUI-Paare vorhanden.
- Projektkonfiguration gegen die offizielle JUCE-8.0.6-CMake-Dokumentation geprueft.
- Keine Audio-Thread-Allokation im eigenen processBlock; Ringpuffer werden in
  prepareToPlay angelegt. State-Serialisierung und GUI-Attachments liegen ausserhalb
  der Sample-Verarbeitung.

Diese Quelltextpruefung ersetzt weder Compiler noch Host-Test.

## Zweiter Release-Versuch nach GUI-Erweiterung

Nach Umsetzung der drei Skins wurde derselbe Release-Befehl erneut ausgefuehrt.
Ergebnis erneut Exitcode 1: CMake fehlt im PATH; der Compiler wurde nicht
gestartet. Auch Windows-SDK und C++-Compiler wurden bei der erneuten Pruefung
nicht gefunden. Das aktuelle Transkript steht in BuildLogs/Release.log.
Ein mitgeliefertes Git wurde inzwischen in den gebuendelten Workspace-Werkzeugen
gefunden; das behebt den fehlenden CMake/MSVC/SDK-Unterbau nicht.

Zusaetzlich vorbereitet: GUI-Bindungstests, echte JUCE-Screenshot-Exporte und
Referenzvergleich per Python. Ausfuehrungsgrenzen siehe GUI_VALIDATION.md.

## GitHub-Actions-Erweiterung

Ein Windows-2022-/VS-2022-x64-Release-Workflow ist jetzt enthalten. YAML,
PowerShell, CMake-Grammatik, isolierte CMake-Pfadgenerierung sowie sieben
Paketierungsfaelle wurden lokal erfolgreich geprueft. Portable CMake-Werkzeuge
stehen fuer diese Pruefung im Arbeitsordner bereit; MSVC fehlt weiterhin.
Ein echter CMake-Konfigurationsversuch stoppt deshalb jetzt explizit mit
"could not find any instance of Visual Studio". Alle Dateien unter Source/
sind fuer die CI-Erweiterung unveraendert geblieben.

Es gibt noch keinen ausgefuehrten GitHub-Lauf und kein fertiges VST3-Artefakt.
Vollstaendige Einrichtung: GITHUB_ACTIONS.md; Pruefprotokoll:
GITHUB_ACTIONS_VALIDATION.md.
