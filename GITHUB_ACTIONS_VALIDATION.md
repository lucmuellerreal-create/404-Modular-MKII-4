# CI-Pruefprotokoll — 16.10.2026

## Bestanden

| Pruefung | Ergebnis |
| --- | --- |
| Workflow-YAML mit PyYAML 6.0.2 eingelesen | bestanden |
| GitHub-Actions-Schema/Expressions mit actionlint 1.7.7 | Exitcode 0, keine Diagnosen |
| PowerShell-AST beider neuer .ps1-Dateien | keine Parserfehler |
| CMakeLists.txt mit cmakelang 0.6.13 geparst | bestanden |
| CMake 3.31.6: `--list-presets` | VS-2022-x64-Preset erkannt |
| CMake 3.31.6: neue Manifest-Generierung isoliert konfiguriert/generiert | bestanden |
| JUCE-Tag 8.0.6 per `git ls-remote` | `51a8a6d7aeae7326956d747737ccf1575e61e209` bestaetigt |
| Checkout v4.2.2 / Upload v4.6.2 SHAs per `git ls-remote` | bestaetigt |
| Alle 24 vorhandenen C++-Quelldateien vor/nach CI-Aenderung gehasht | identisch |

Die Manifest-Pruefung verwendet **nur fuer den lokalen Test** ein CMake-Projekt
mit `LANGUAGES NONE` und importierten Test-Targets. Der neue `file(GENERATE)`-Block
wurde unveraendert aus der echten CMakeLists.txt uebernommen. Konfiguration und
Generierung wurden ausgefuehrt; die JSON-Targetpfade inklusive verschachtelter
JUCE-Generatorausdruecke ergaben das dokumentierte Release-VST3-Bundle und dessen
innere Windows-Binary. Dieser Test kompiliert keine C++-Dateien und ersetzt die
echte JUCE-Konfiguration auf dem GitHub-Runner nicht.

## Paketierungs- und Fehlerlog-Tests

`Tests/Test-CiPackaging.ps1` wurde lokal mit PowerShell 7.6.5 ausgefuehrt.
Die unveraenderte produktive `Build-Windows.ps1 -Phase Package` wird dabei in
isolierten Testverzeichnissen mit synthetischen PE-Headern aufgerufen.

| Testfall | Erwarteter und tatsaechlicher Exitcode |
| --- | --- |
| Bundle fehlt | 1 |
| Bundle vorhanden, Binary fehlt | 1 |
| Binary leer | 1 |
| x86 statt x64 | 1 |
| ARM64 statt x64 | 1 |
| Defekter PE-Header | 1 |
| AMD64-Header, Ressource und versteckte Datei | 0 |

Fuer jeden Fehlerfall wurde das erhaltene `Package-error.log` geprueft.
Beim Erfolgsfall wurden innere `.vst3`-Datei, Ressource und versteckte Datei im
kopierten Bundle nachgewiesen. Es sind **keine echten Plugin-Binaries** entstanden.
Diese Testdaten liegen nur unter dem lokalen Arbeitsordner und sind nicht Teil
des Projekt-ZIPs oder des installierbaren CI-Artefakts.

Wiederholbarer Aufruf im Projektordner mit PowerShell 7:

```powershell
./Tests/Test-CiPackaging.ps1 -ScratchDirectory (Join-Path $env:TEMP 'SPVinylLab-package-tests')
```

Der GitHub-Workflow fuehrt diese Tests ebenfalls vor dem Build aus.

## Grenzen / nicht ausgefuehrt

Portable CMake-/Pruefwerkzeuge wurden ausschliesslich im lokalen Arbeitsordner
bereitgestellt. Der erste Paketdownload scheiterte am gesperrten Standard-Temp-
Verzeichnis. Mit einem temporaren Verzeichnis im Arbeitsordner gelang der Download.
Der erste isolierte CMake-Probelauf verwendete einen nicht vorhandenen Ninja-Pfad;
nach Bereitstellung von Ninja im eigenen Werkzeugverzeichnis war der Test erfolgreich.
Beides betraf ausschliesslich die lokale Pruefumgebung; die Workflow-Dateien
verwenden das vorhandene CMake und MSVC des Windows-Runners.

Eine **echte** Konfiguration des vorhandenen Projekts wurde anschliessend versucht:

```powershell
cmake -S outputs/SPVinylLab -B work/actual-windows-configure -G "Visual Studio 17 2022" -A x64 -T v143
```

CMake beendet sich mit Exitcode 1 an `project(...)`:

> Generator Visual Studio 17 2022 could not find any instance of Visual Studio.

Es liegen weiterhin kein lokaler C++-Compilerlauf, keine echte VST3-Binary und
kein FL-Studio-Laufzeittest vor. Ebenso wurde kein GitHub-Actions-Lauf gestartet,
weil kein Repository/Remote vorhanden ist. Erst der erste Lauf im Zielrepository
kann Compiler-/Linkerfehler des bestehenden Codes nachweisen. DSP und GUI wurden
ohne konkrete Compilerdiagnose entsprechend der Anforderung nicht veraendert.

## Gepruefte primaere Quellen

- [JUCE 8.0.6: Target-Namen und Windows-Bundle-Ausgabe](https://github.com/juce-framework/JUCE/blob/8.0.6/extras/Build/CMake/JUCEUtils.cmake)
- [Checkout v4.2.2](https://github.com/actions/checkout/tree/v4.2.2)
- [Upload Artifact v4.6.2](https://github.com/actions/upload-artifact/tree/v4.6.2)
