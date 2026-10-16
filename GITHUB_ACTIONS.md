# Windows-x64-Build mit GitHub Actions

Der Workflow baut das vorhandene Plugin mit `windows-2022`, Visual Studio 2022,
MSVC v143, CMake und Release/x64. Es wird kein neues Plugin erzeugt oder kopiert.
Lokal werden keine Visual-Studio-Werkzeuge benoetigt.

## 1. Dateien ins bestehende Repository uebernehmen

Der **Inhalt** dieses SPVinylLab-Ordners muss am Repository-Stamm liegen:

```text
Repository/
  .github/
    workflows/build-windows.yml
    scripts/Build-Windows.ps1
  CMakeLists.txt
  CMakePresets.json
  Source/
  Tests/
  Assets/
```

Beim Uebernehmen der Aenderungen auch `.github/scripts/Build-Windows.ps1`,
`Tests/Test-CiPackaging.ps1` und die aktualisierte `CMakeLists.txt` mitnehmen.
Der Workflow allein ohne diese Begleitdateien reicht nicht aus.
Nicht bloss das Projekt-ZIP als einzelne Datei ins Repository laden.
Die lokale Verpackung `outputs/SPVinylLab` ist kein Repository-Pfad des Workflows.
Wenn dein bestehendes Repository das Plugin in einem Unterordner enthaelt, muessen
Workflow-Aufrufe und Artefaktpfade zuerst auf diese konkrete Struktur angepasst werden.
Diese Version setzt absichtlich den oben dokumentierten Repository-Stamm voraus.

Die Datei `.github/workflows/build-windows.yml` committen und pushen. Der Workflow
startet bei jedem Push und Pull Request. Zusaetzlich ist `workflow_dispatch`
aktiviert. Fuer die manuelle Auswahl **Actions > Windows x64 VST3 Release >
Run workflow** muss die Workflow-Datei auf dem Standardbranch vorhanden sein.
Falls Actions im Repository deaktiviert ist, dort zuerst aktivieren.

## 2. Was gebaut und geprueft wird

| Eigenschaft | Tatsaechlicher Projektwert |
| --- | --- |
| Shared-Code-Target | `SPVinylLab` |
| Plugin-Target | `SPVinylLab_VST3` |
| Test-Targets | `SPVinylLabTests`, `SPVinylLabGuiCapture` |
| Generator / Plattform | `Visual Studio 17 2022` / `x64` |
| Toolset / Konfiguration | `v143` / `Release` |
| JUCE-Bezug | bestehendes CMake-FetchContent, Tag `8.0.6` |
| Gepruefter JUCE-Commit | `51a8a6d7aeae7326956d747737ccf1575e61e209` |
| VST3-Bundle | `build/SPVinylLab_artefacts/Release/VST3/SP Vinyl Lab.vst3/` |
| Windows-Binary im Bundle | `Contents/x86_64-win/SP Vinyl Lab.vst3` |

Die Pfade wurden aus dem vorhandenen Target/Product-Namen und JUCE 8.0.6s
CMake-Implementierung abgeleitet. CMake schreibt die tatsaechlich ausgewerteten
Target-Pfade in `build/ci/Release/vst3-paths.json`; die Paketierung liest diese
Datei. Falls das Target nicht existiert, stoppt schon die Konfiguration.

Checkout erfolgt rekursiv fuer Submodule. Aktuell ist JUCE kein Submodule,
sondern wird durch die vorhandene CMake-Konfiguration geladen. Ein leerer
`JUCE_SOURCE_DIR` erzwingt im frischen CI-Build diesen Weg. Nach der Konfiguration
wird der exakte Git-Commit geprueft; ein verschobener Release-Tag wird abgelehnt.
Die Checkout-/Upload-Actions sind ebenfalls auf gepruefte Commit-SHAs festgelegt.
`windows-2022` ist ein aktualisiertes Runner-Image: Der JUCE-Quellstand ist
festgelegt, byteidentische Ergebnisse ueber alle kuenftigen Runner-Updates
werden damit nicht zugesichert. Werkzeugversionen stehen in den Diagnose-Logs.

Der Build umfasst die vorhandenen DSP- und GUI-Dateien unveraendert. Die GUI
zeichnet alle Assets prozedural mit JUCE Graphics. Externe PNGs, Fonts, Samples
oder die urspruenglichen Referenzbilder sind zur Kompilierung nicht erforderlich.
`Assets/README.md` ist vorhanden. Preflight prueft alle benoetigten Quell-/Testdateien
und protokolliert deren SHA256-Hashes.

CTest fuehrt Prozessor- und GUI-Bindungstests aus; der GUI-Test erzeugt neun echte
JUCE-PNGs. Ein fehlgeschlagener Test verhindert das installierbare Plugin-Artefakt.
Die GUI-Captures sind im separaten Diagnose-Artefakt enthalten.
Ein erfolgreicher CI-Lauf ersetzt keinen Test in FL Studio.

Vor dem Upload muss die VST3-Binary existieren, nicht leer sein und einen
AMD64-PE32+-DLL-Header enthalten. Fehlendes Bundle, fehlende/defekte Binary,
x86- oder ARM64-Binary brechen den Workflow ab. Die Paketpruefung wird zusaetzlich
mit synthetischen Testdaten getestet. Diese Testdateien werden nie als Plugin
hochgeladen. Das echte Bundle wird einschliesslich versteckter Dateien und
`Contents/Resources` nach `dist/SP Vinyl Lab.vst3` kopiert; alle kopierten Dateien
werden per SHA256 gegen das Build-Ergebnis geprueft.

## 3. ZIP-Artefakt herunterladen

1. Bei GitHub anmelden und das Repository oeffnen.
2. **Actions > Windows x64 VST3 Release** waehlen.
3. Einen erfolgreichen Lauf fuer deinen gewuenschten Commit oeffnen.
4. Unter **Artifacts** auf **SP-Vinyl-Lab-Windows-x64-VST3** klicken.
5. Die heruntergeladene ZIP-Datei vollstaendig entpacken.

GitHub erstellt das ZIP aus `dist/`. Darin liegt direkt der ganze Ordner
`SP Vinyl Lab.vst3` mit seiner `Contents`-Struktur. Es gibt kein zweites,
selbst erzeugtes ZIP im Artefakt. Das Plugin-Artefakt bleibt 30 Tage erhalten,
sofern Repository-Richtlinien diesen Zeitraum nicht weiter beschraenken.
Zum Download sind Anmeldung und Lesezugriff auf das Repository erforderlich.
Siehe [GitHub: Artefakte herunterladen](https://docs.github.com/en/actions/how-tos/manage-workflow-runs/download-workflow-artifacts?tool=webui).

## 4. In FL Studio installieren

1. FL Studio schliessen.
2. Den **gesamten Ordner** `SP Vinyl Lab.vst3` nach
   `C:\Program Files\Common Files\VST3\` kopieren; Windows kann Administratorrechte
   verlangen. Die innere `.vst3`-Binary nicht einzeln herausnehmen.
3. Diese Struktur muss vorhanden sein:

```text
C:\Program Files\Common Files\VST3\SP Vinyl Lab.vst3\
  Contents\
    x86_64-win\
      SP Vinyl Lab.vst3
    Resources\
      moduleinfo.json
```

4. FL Studio 64-Bit starten und **Options > Manage plugins** oeffnen.
5. **Verify plugins** aktivieren und **Find installed plugins** ausfuehren.
6. **SP Vinyl Lab** als Effekt in einem Mixer-Effektslot laden.
7. Audio, Bypass, alle drei Modi und das Speichern/erneute Laden eines Projekts pruefen.

Das Standardverzeichnis und das Scan-Verfahren entsprechen der
[FL-Studio-Installationsanleitung](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_externalplugins.htm).

## 5. Fehlerlogs herunterladen

Bei einem roten Lauf dasselbe Actions-Ergebnis oeffnen und das Artefakt
**SP-Vinyl-Lab-Windows-x64-Diagnostics** herunterladen. Es wird auch bei Fehlern
hochgeladen (`always()`), solange der Runner den Upload noch ausfuehren kann.
Enthalten sind alle bis zum Fehler vorhandenen Diagnose-Dateien:

- PowerShell-Transkripte und phasenbezogene Fehlerdateien;
- CMake-Konfigurationsausgabe und Cache/Configure-Logs;
- Compiler-/Linker-Ausgabe in `build.log` und MSBuild-Binaerlog `msbuild.binlog`;
- CTest-Ausgabe und temporare Testlogs;
- Werkzeugversionen, JUCE-Commit, Quell-/Bundle-Hashes und Target-Pfadmanifest;
- vorhandene GUI-Captures und Protokolle der Paketpruefungs-Tests.

Diagnosen bleiben 14 Tage erhalten. Ein Runner-Abbruch oder ausgefallener
Artefaktdienst kann auch einen `always()`-Upload verhindern.
Fuer konkrete Compilerfehler ist `build.log` der erste Anlaufpunkt.

## Aenderungen und bisherige Pruefung

- Neu: `.github/workflows/build-windows.yml`.
- Neu: `.github/scripts/Build-Windows.ps1` mit Preflight, Configure, Build, Test und Package.
- Neu: `Tests/Test-CiPackaging.ps1` fuer sieben Paketierungs-Faelle.
- CMake ergaenzt: Target-Existenzpruefung und generiertes JSON-Pfadmanifest.
- `.gitignore` ergaenzt: `ci-logs/` und `dist/`.
- README/Build-Bericht und diese Anleitung aktualisiert.
- **Keine Aenderung an den 24 Dateien unter Source/**; SHA256-Abgleich bestanden.

Lokale YAML-, PowerShell-, CMake- und Paketierungspruefungen sind in
`GITHUB_ACTIONS_VALIDATION.md` dokumentiert. Es wurde noch kein Workflow in einem
GitHub-Repository gestartet: In diesem Arbeitsordner ist kein Git-Repository und
kein Remote eingerichtet. Die Dateien sind fuer dein bestehendes Repository
vorbereitet; ein erfolgreicher Online-Build ist noch nicht nachgewiesen.
