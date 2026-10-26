# Auslieferung und echte Prüfungen – Browser 0.1

Stand: 26.10.2026. Grundlage: bestehendes Repository `2ef77a3`. Das Unreal-Projekt und seine Konfigurationen wurden nicht verändert. In dieser Umgebung sind weiterhin keine Unreal-Binaries oder Live-Editorwerkzeuge verfügbar. Es wurde kein Unreal-Build erzeugt oder als getestet ausgegeben.

## Sofort nutzbare Lieferung

[NOSTALGIA-Browser-0.1.zip herunterladen](https://github.com/hsynZz/NOSTALGIA-GAME/raw/refs/heads/main/BrowserPrototype/Delivery/NOSTALGIA-Browser-0.1.zip), entpacken, `index.html` im aktuellen Edge/Chrome öffnen und **Aufwachen** wählen. Die etwa 147 KB große ZIP enthält das vollständige Spiel als eigenständige HTML-Datei, Anleitung, Lizenz und Build-Prüfsumme. Es werden keine Modelle, Klänge, Bibliotheken oder Schriftarten aus dem Netz nachgeladen. Ein WebGL2-fähiger Browser wird benötigt.

Eine GitHub-Pages-Veröffentlichung ist mit [.github/workflows/browser-pages.yml](../.github/workflows/browser-pages.yml) vorbereitet. Die Cloud konnte die Pages-Einstellungen wegen gesperrtem API-Zugriff nicht lesen/ändern; die Pages-Adresse konnte wegen des Netzwerkproxys nicht überprüft werden. **Kein bestätigter Live-Link.** Die ZIP ist die unmittelbar gelieferte Testversion. Die konkreten einmaligen Pages-Einstellungen stehen in [README.md](README.md#github-pages).

## Implementierter Spielumfang

- First-Person mit Maus und Tastatur; Gehen, Sprint, Sprung und Ducken; feste Wand-/Möbelkollision und Türöffnung.
- Start im eingerichteten Kinderzimmer 2004, Wohnbereich, Vorgarten, sechs Nachbarhäuser und ein separater größerer Hausinnenraum mit Rückweg.
- Warme Sommerfarben, prozeduraler Himmel und Wolken, Datum/Uhr und ein Nachtwechsel mit Laternen. Sparprofil begrenzt die interne Auflösung auf höchstens 1280×720 und den Renderer auf 30 Bilder/s; keine gemessene Hardware-FPS-Garantie.
- Originale schematische Fotoansicht; drei Stühle am Tisch und ein versetzter vierter Stuhl als Erinnerungshinweis. Geburtstag stellt die Anordnung und das fehlende Haus wieder her.
- Räumliches synthetisches „Ha-ha“-Gartengeräusch: nur im Garten, einmalig, Ende beim Annähern oder Verlassen. Nacht-Klopfen im übergroßen Haus nach Ruhezeit.
- Frühe Straßenschleife mit zusätzlicher Hausnummer; nach Geburtstag neue Route. Sichtgeprüfter schwarzer Heimkehrer, begrenzte Annäherung beim Wegsehen und zeitlicher Rückzug.
- Dunkle Laterne → freigegebenes Spielhaus → begehbarer viel zu langer Innenflur → Abschlussanzeige. Kein zweites Kapitel enthalten.
- Browser-Spielstand mit Ereignishistorie, Uhr, Türen, Fortschritt und Blickrichtung; Speichern auf Möbeln oder im künftigen Türblatt wird ohne Überschreiben eines gültigen Saves abgelehnt; Pause, Hinweise, Grafik-/Tonumschaltung und Touch-Steuerung.

## Wirklich ausgeführte Prüfungen

| Prüfung | Ergebnis |
| --- | --- |
| `npm test` | **13/13** Node-Tests für Quest, Director, Speicherdaten und echte Bewegungs-/Kollisionslogik bestanden |
| C++/JavaScript-Parität | Vier echte produktive C++-Core-Dateien mit GCC kompiliert; **41 vollständige Zustandszeilen** einschließlich RNG, Quest und bestätigter Ereignisse identisch |
| `npm run build` | Selbstständiges HTML und ZIP erzeugt; ZIP-Integrität und HTML-Prüfsumme gegen exakt getestete Datei geprüft |
| `npm run test:browser` | **20/20** Prüfgruppen in echtem Chromium 154, WebGL2 mit SwiftShader bestanden; keine JavaScript-Seitenfehler |
| Audioausgabegraph | WebAudio per Startklick freigegeben; tatsächliche Analyser-Ausgabe größer null, 44,1 kHz. Kein menschlicher Hörtest behauptet |
| Offline-Ausführung | Exakte HTML-Auslieferungsbytes bei abgeschaltetem Browsernetz gestartet; keine externen Asset-Anfragen |
| Mobile Eingaben | Touch-Joystick, Ziehen zum Umsehen, Ducken, E-Interaktion und Fotooberfläche in Chromium-Mobilsimulation geprüft |
| Bestehende Regression | **62/62** native C++- und **9/9** Python-Tests mit ASan/UBSan bestanden |

Die 20 Browser-Prüfgruppen umfassen tatsächliche Tastatureingaben, Wand-/Türkollision, Raycast-Interaktionen, beide Hausportale, Audiostart/-abbruch, beide Schleifenvarianten, Geburtstag/Nacht, Heimkehrer/Annäherung, Ruhezeit vor Klopfen, Lampensperre, Flur/Abschluss, Speichern und Wiederherstellen nach Neuladen. Zwischen längeren Abschnitten verwendet der Test ausschließlich im Testmodus verfügbare Positionshilfen. **Kein ununterbrochener menschlicher Spieldurchlauf behauptet.** Die eigentlichen Interaktionen laufen über die gerenderte Spielwelt und ihre Sichtstrahlen, nicht durch direktes Setzen der Quest-Stufen.

Die Cloud-Browserrichtlinie blockiert `file://`-Navigation. Daher wurde das offline ausgeführte HTML über `about:blank` gestartet; der direkte Windows-Doppelklick wurde hier nicht getestet. Das Format verwendet bewusst ein klassisches eingebettetes Script ohne Module, Fetch oder Serverabhängigkeit. Ein echter Windows-/iPhone-/Safari-Gerätetest bleibt offen.

Maschinenlesbare Belege: [Browser-Ergebnis](Validation/browser-results.json), [Unit-Log](Validation/unit-tests.log), [Native-Ergebnis](Validation/native-results.json), [Liefermanifest und Hashes](Validation/delivery-manifest.json). Reale Prüfscreenshots werden beim erneuten Browser-Test unter `test-results/` erzeugt; es wurden keine generierten Bilder als Gameplay-Belege verwendet.

## Bewusste Unterschiede zum Unreal-Projekt

Der Browser verwendet einen ausdrücklich getrennten JavaScript-Port. Direkte C++-Ausführung per WebAssembly wurde für die installierungsfreie, serverlose Ein-Datei-Lieferung nicht eingerichtet. Quest-Stufen, Datum, Uhrgeschwindigkeit, Event-IDs, Regeln, Seed und Auswahl-/Bestätigungsprinzip entsprechen dem geprüften Core. Die Paritätstests belegen die ausgeführten Fälle, keine vollständige Gleichheit aller möglichen Eingaben.

Unreal-Zentimeter werden in Browser-Meter mit Y als Höhe und Z als zweiter Bodenachse übertragen. Physik ist vereinfacht: vertikale Figur gegen feste rechteckige Flächen, ebener Grund, vereinfachte Tür-Kollision statt Unreal-Capsule-Sweeps. Offene Türblätter blockieren nicht dynamisch. Die Heimkehrer-Darstellung ist ein eigener begrenzter Browser-Ablauf, kein Port der vollständigen Unreal-KI. Räumliches Browseraudio nutzt Abstand/Panorama; echte Wand-Occlusion fehlt. Browser-Saves nutzen einen eigenen lokalen Speicherplatz und sind nicht mit Unreal-Saves austauschbar; private Browsereinstellungen können Speicherung verhindern.

Die Gartenstimme ist ein eigener synthetischer Testklang, keine Aufnahme von Kindern. Hochwertige Modelle, fotorealistische Grafik, Hände/Animationen, echte Stimmen, vollständige Verfolgung, weitere Kapitel, Steam-/Konsolenbuild und der erste tatsächliche Unreal-Lauf fehlen weiterhin. Die Browserversion bleibt der spielbare Konzepttest neben dem Unreal-Hauptprojekt.
