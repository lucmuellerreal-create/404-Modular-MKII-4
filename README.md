# SP Vinyl Lab

Windows-x64-VST3-Audioeffekt in C++20 mit JUCE **8.0.6**, CMake und MSVC v143.
Stereo hinein / Stereo heraus, kein MIDI, kein Synthesizer. Version 0.1.0.

**Windows-Build ohne lokale C++-Toolchain:** Der Workflow unter
`.github/workflows/build-windows.yml` baut dieses Projekt auf GitHub Actions.
Einrichtung, ZIP-Download und Installation in FL Studio: [GITHUB_ACTIONS.md](GITHUB_ACTIONS.md).
Lokale Workflow-Pruefung: [GITHUB_ACTIONS_VALIDATION.md](GITHUB_ACTIONS_VALIDATION.md).

## Funktionsumfang

Neue Instanzen starten mit aktivem **Bypass** (`enabled = false`): Samples werden
unveraendert durchgereicht, ohne Pegelanpassung oder hinzugefuegte Latenz.
Der Schalter aktiviert eine eigene, einfache Vinyl-Klangkette:
Wow/Flutter (gemeinsame modulierte Stereo-Delayline), Tiefpass, Stereo-gekoppelte
Kompression, Soft-Saturation und prozedurales Rauschen/Knistern, danach Dry/Wet.
Keine Roland-Originalalgorithmen, Samples oder garantierte Hardware-Emulation.
Saturation ist nicht oversampled und kann bei starkem Drive aliasen.
Die variable Delayline ist ein Effekt, keine feste, kompensierbare Plugin-Latenz.
Enable schaltet direkt; ein Umschalten im laufenden Signal kann klicken.

**4SX Vinyl**, **3DR Vinyl**, **40G Vinyl** haben eigene Startwerte und jeweils
acht unabhaengige Parameter einschliesslich Input-/Output-Gain. Die Algorithmen sind dieselben; die Modi sind
getrennte Klangbaenke mit unterschiedlich abgestimmten Anfangswerten.

## Parameter und Speicherung

26 stabile, automatisierbare APVTS-Parameter:

| ID | Bedeutung |
| --- | --- |
| `mode` | Choice: 0 = 4SX, 1 = 3DR, 2 = 40G |
| `enabled` | Vinyl-Verarbeitung aktiv; Vorgabe aus |
| `m0_noise` bis `m2_noise` | Rauschen/Knistern |
| `m0_wow` bis `m2_wow` | Wow/Flutter |
| `m0_compression` bis `m2_compression` | Kompressionsstaerke |
| `m0_tone` bis `m2_tone` | Hoehenbereich |
| `m0_drive` bis `m2_drive` | Saettigung |
| `m0_mix` bis `m2_mix` | Dry/Wet |
| `m0_input` bis `m2_input` | Input-Gain, -24..+12 dB |
| `m0_output` bis `m2_output` | Output-Gain, -24..+12 dB |

Die urspruenglichen sechs kontinuierlichen Werte je Bank sind intern auf 0..1
normiert; die drei sichtbaren Drehregler zeigen 0..127. Gain-Fader zeigen dB. FL Studio sieht alle drei Baenke gleichzeitig. Automation einer
inaktiven Bank bleibt moeglich, hoerbar wird sie bei Auswahl dieser Bank.
Ein Moduswechsel kopiert keine Parameter und ueberschreibt keine Automation.
Der Editor wechselt nur seine Attachments; Host-Moduswechsel werden ebenfalls
angezeigt. Eine laufende Mausgeste darf zuerst enden; danach wechselt die GUI ihre
Zuordnung. Der DSP folgt dem Host-Modus sofort.
Alle Werte einschliesslich Modus und Aktivierung werden als APVTS-XML im
Host-Zustandsblock gespeichert. Delay-/Filterpuffer sind bewusst kein Preset-Zustand.
Parameter-IDs und Plugin-Codes bei kuenftigen Updates nicht aendern.

## Drei Referenz-Skins

Die gesamte Oberflaeche wird mit JUCE Graphics/LookAndFeel gezeichnet, ohne
Rasterhintergruende oder externe Fonts. Kleine Auswahl oben: 4SX / 3DR / 40G.
Der Skin folgt dem gespeicherten und automatisierbaren DSP-Modus.

- **4SX**: dunkles Metall, orange/weisse Schrift, RNG / NOZ / FLU,
  dunkler kreisfoermiger Displayring.
- **3DR**: silberner Rahmen, schwarzes Panel, rote Skalenticks,
  Compression / Vinyl Noise / Wow & Flutter, rechteckiges Display.
- **40G**: helles Metall, CMP / NOZ / FLU, blauer Displayring.

Die drei Drehregler zeigen ihren Wert zusaetzlich unter dem Knopf. Das zentrale
Sieben-Segment-Display folgt dem zuletzt geaenderten Regler (auch Host-Automation)
und zeigt 000..127. Fader steuern reale, geglaettete Gains vor und nach der
Vinyl-Klangkette. Bypass deaktiviert die gesamte Klangkette einschliesslich Gains.
Eine leuchtende Bypass-Anzeige bedeutet unveraendertes Eingangssignal.
Die Effect-LED zeigt die Aktivierung, Peak wird 800 ms gehalten (4SX/40G).
Input-/Output-Pegel werden getrennt erfasst und mit 60-Hz-UI und zeitbasiertem
Abfall dargestellt. Die 3DR-Fuss-LED zeigt Effect wie in der Referenz.

Ziehen oder Mausrad fuer Regler/Fader; Doppelklick stellt den jeweiligen
Parameter-Startwert wieder her (Gain: 0 dB). Tooltips und Tastaturfokus sind
vorhanden. Das Fenster skaliert vektoriell zwischen 80 und 200 Prozent mit
festem Skin-Seitenverhaeltnis, zusaetzlich zu JUCE/Windows-HiDPI.

Die uebrigen DSP-Parameter bleiben in FL Studios Host-Parameterliste erreichbar;
zusatzliche sichtbare Regler wuerden von den Drei-Regler-Referenzen abweichen.
Bestehende 20 IDs und ihre Reihenfolge bleiben erhalten; sechs Gain-Parameter
sind am Ende angehaengt. Gain-Vorgabe ist 0 dB.
Version-1-Zustaende ohne Gain-Parameter werden beim Laden auf 0 dB ergaenzt.

Koordinaten und Bildanalyse: `GUI_LAYOUT.md`. Status der Laufzeit-Sichtpruefung:
`GUI_VALIDATION.md`. CTest startet auch den echten JUCE-Capture-Test und legt
bei Erfolg neun PNG-Dateien unter `build/GuiCaptures` ab.

## Voraussetzungen und Release-Build

1. Windows 10/11 x64; Visual Studio 2022 oder Build Tools mit **Desktopentwicklung
   mit C++**, MSVC v143 und Windows 10/11 SDK installieren.
2. CMake >= 3.22 und Git installieren und in `PATH` bereitstellen. Einmaliger
   Netzwerkzugriff auf GitHub fuer JUCE erforderlich.
3. Im Projektordner PowerShell oeffnen:

```powershell
.\Build-Release.ps1
```

Alternativ:

```powershell
cmake --preset vs2022-x64
cmake --build --preset release --parallel
ctest --preset release
```

JUCE wird per festem Release-Tag `8.0.6` bezogen, nicht von `master` oder `develop`.
Fuer einen Offline-Build einen lokalen unveraenderten JUCE-8.0.6-Quellbaum nutzen:

```powershell
cmake --preset vs2022-x64 -DJUCE_SOURCE_DIR=C:/SDK/JUCE-8.0.6
cmake --build --preset release --parallel
ctest --preset release
```

`build/SPVinylLab.sln` laesst sich in Visual Studio oeffnen (Release, x64).
MSVC-Runtime wird statisch mit `/MT` eingebunden (`/MTd` in Debug); WebBrowser
und CURL sind abgeschaltet. Windows-System-DLLs bleiben erforderlich. JUCE
und das enthaltene VST3-SDK werden aus den Quellen gebaut; keine separate
JUCE-, VC-Redistributable- oder WebView2-Installation beim Anwender vorgesehen.
Die JUCE-Lizenzbedingungen sind vor einer Veroeffentlichung zu pruefen.

## VST3 und FL Studio

Ergebnis ist das **gesamte Bundle-Verzeichnis**:

`build/SPVinylLab_artefacts/Release/VST3/SP Vinyl Lab.vst3`

Den ganzen Ordner nach `C:\Program Files\Common Files\VST3\` kopieren
(gegebenenfalls Administratorrechte). Kein automatisches Kopieren beim Build.
In FL Studio den Plugin Manager oeffnen, einen Plugin-Scan ausfuehren und
**SP Vinyl Lab** in einem Mixer-Effektslot laden.

Herstellercode: `Spvl`, Plugin-Code: `Spv1`, Bundle-ID: `com.spvinyllab.effect`.
Dies sind gueltige projektspezifische Codes; eine weltweite Registrierung oder
Kollisionsfreiheit gegen fremde Plugins wird nicht behauptet.

## Validierung

`SPVinylLabTests` prueft Stereo-Konfiguration, Mono-Ablehnung, bitgenauen
Default-Passthrough bei 44.1/48/96/192 kHz und verschiedenen Blockgroessen,
State-Roundtrip aller 26 Parameter, ungueltige Zustandsdaten, endliche DSP-Ausgabe
in allen Modi, trockenen Mix, Gain-Verarbeitung und Hard-Bypass trotz Gain-Werten. Tests verwenden explizite Fehlerpruefungen,
die auch im Release-Build aktiv bleiben.

Noch manuell in FL Studio pruefen: Plugin-Scan, Audio, Modusautomation,
separate Werte aller Baenke, Projekt speichern/schliessen/neu laden,
Editor neu oeffnen, Offline-Rendering, wechselnde Blockgroessen und mehrere
Instanzen. Optional zusaetzlich Steinberg VST3 Validator/pluginval einsetzen.
Diese Host-Pruefungen sind durch die Prozessortests nicht ersetzt.

Der aktuelle Ausfuehrungsstatus steht in `BUILD_REPORT.md`; Build-Skript-Ausgaben
werden in `BuildLogs/Release.log` abgelegt.

## Quellen

- [JUCE 8.0.6 CMake API](https://github.com/juce-framework/JUCE/blob/8.0.6/docs/CMake%20API.md)
- [JUCE 8.0.6 Quellstand](https://github.com/juce-framework/JUCE/tree/8.0.6)
