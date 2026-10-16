# GUI-Layout-Spezifikation (vor Implementierung)

Die drei PNG-Dateien wurden mit Pillow geoeffnet und vermessen. Ihre FL-Studio-
Titelleisten (y=0..21) gehoeren zum Host und werden nicht ins Plugin gezeichnet.
Alle folgenden Rechtecke sind logische JUCE-Pixel: `(x, y, Breite, Hoehe)`,
nach Abzug von 22 Pixeln am oberen Bildrand. Skalierung erfolgt gleichmaessig.

| Referenz | PNG | Plugin-Zeichenflaeche |
| --- | --- | --- |
| 4SX, 142938 | 667 x 539 | 667 x 517 |
| 3DR, 142946 | 688 x 491 | 688 x 469 |
| 40G, 142958 | 684 x 546 | 684 x 524 |

| Element | 4SX | 3DR | 40G |
| --- | --- | --- | --- |
| Gehaeuse | 8,2,657,513 | 18,15,655,442 | 6,1,674,519 |
| Oberes Bedienfeld | 92,95,505,113 | 38,65,616,359 (gesamtes schwarzes Feld) | 89,95,507,114 |
| Regler 1 | 124,100,104,104 | 133,112,104,104 | 122,100,104,104 |
| Regler 2 | 294,100,104,104 | 294,112,104,104 | 291,100,104,104 |
| Regler 3 | 462,100,104,104 | 455,112,104,104 | 460,100,104,104 |
| Unteres Bedienfeld | 91,265,506,214 | siehe schwarzes Feld | 89,265,507,215 |
| Display-Ring | 219,240,253,253 | keiner | 217,240,253,253 |
| LED-Display | 255,324,179,90 | 251,309,190,80 | 252,324,180,90 |
| Input-Fader (Hitbox) | 135,301,49,113 | 122,302,49,91 | 133,301,49,113 |
| Output-Fader (Hitbox) | 505,301,49,113 | 521,302,49,91 | 503,301,49,113 |
| Input-Meter | 209,325,6,88 | 200,321,6,72 | 208,325,6,88 |
| Output-Meter | 475,325,6,88 | 486,321,6,72 | 473,325,6,88 |
| Bypass | 490,444,81,25 | 548,432,91,20 | 488,444,81,25 |
| Peak-LED | 424,167,10,10 | 47,437,9,9 | 422,167,10,10 |
| Effect-LED | 131,449,10,10 | 47,437,9,9 | 129,449,10,10 |
| Modusumschalter (Ergaenzung) | 543,16,77,18 | 290,26,92,18 | 550,16,77,18 |

## Optische Details

- 4SX: anthrazitfarbenes Metall, orange `4SX`, weisses `VINYL`, dunkle
  gruenlichgraue Panels, schwarze konzentrische Knopfkoerper mit weisser Kappe.
  Beschriftungen RNG/NOZ/FLU; erster Regler steuert `tone`.
- 3DR: silberner Rahmen, flaches schwarzes Panel, rote Regler-Ticks und gelbliche
  Kappen mit Zeiger; erster Regler steuert `compression`. Compression / Vinyl Noise /
  Wow & Flutter, rote EFFECTS CONTROL-Linie, rechteckiges Digitaldisplay.
- 40G: helles Metall, dunkle Typografie, schwarzes Reglerpanel, CMP/NOZ/FLU,
  blauer radialer Displayeinsatz und dunkler Metallring.
- Fader: metallische Kappe, dunkler Schlitz, -24..+12 dB mit 0 dB bei 2/3 Hoehe.
  Werte erscheinen unterhalb; neue APVTS-Parameter pro Modus `input` und `output`.
- Digitaldisplay: drei Vektor-Sieben-Segment-Ziffern, fuehrende Nullen, 0..127.
- Keine Host-Titelleiste, Mauszeiger oder fremden Hersteller-/Autorenlogos kopiert.
  Stattdessen kleine SP VINYL LAB-Signatur.

## Interaktion und Skalierung

Modus ist derselbe automatisierbare `mode`-Parameter wie im DSP. Die GUI wechselt
das Seitenverhaeltnis passend zum Skin und erhaelt die Fensterbreite. Logische
Koordinaten bleiben konstant; JUCE skaliert den gemeinsamen Komponentenbaum.
Fensterbreite 80..200 Prozent der Referenzbreite, hostseitiges HiDPI zusaetzlich.
Regler erhalten 0..127-Wertzeilen; die zuletzt veraenderte sichtbare Reglerfunktion
bestimmt das grosse Display. Fader und Bypass haben Host-Automationsgesten.
Peak-LED zeigt gehaltene Spitzen; Pegelanzeigen erhalten zeitbasierten Abfall.

## Abgleich

Geometrischer Referenzabgleich: Positionen wie oben. Tatsaechliche JUCE-PNGs werden
vom Render-Ziel `SPVinylLabGuiCapture` erzeugt, sobald die C++-Toolchain vorhanden
ist. Ein visueller Laufzeit-Abgleich darf erst nach diesem Renderlauf als erfolgt
gelten. Status und erkennbare Grenzen stehen in GUI_VALIDATION.md.
