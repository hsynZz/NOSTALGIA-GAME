# NOSTALGIA — Endless Summer (2004)

**Aktueller Schwerpunkt: [Quality Overhaul 0.2](Docs/Quality02/README.md).** Echte lizenzierte 3D-Assets, die erste eingerichtete Raumszene, PBR-Materialien, Blender-Rendernachweise und ein konkreter UE-5.8.0-Import-/Aufbauprozess liegen vor. Die Unreal-Szene wurde hier noch nicht importiert, kompiliert oder gespielt. [Nachweise und Grenzen](Docs/Quality02/README.md) · [Assetlizenzen](Docs/Quality02/ASSET_LICENSES.md) · [Späterer Unreal-Import](Docs/Quality02/IMPORT_UNREAL.md).

Unreal ist das Hauptprojekt. Die folgende Browserversion bleibt ein technischer Konzepttest von 0.1; ihre Grafik entspricht nicht dem Qualitätsziel.

**Jetzt ausprobieren:** [Spielbare Browser-Testversion herunterladen (ZIP)](https://github.com/hsynZz/NOSTALGIA-GAME/raw/refs/heads/main/BrowserPrototype/Delivery/NOSTALGIA-Browser-0.1.zip). Entpacken und `index.html` mit Edge/Chrome öffnen; kein Unreal, kein lokaler Server und keine zusätzliche Installation erforderlich. Aktueller Browser mit WebGL2 vorausgesetzt.

**Ohne ZIP:** [Einzelne HTML-Datei auf GitHub](https://github.com/hsynZz/NOSTALGIA-GAME/blob/main/BrowserPrototype/Delivery/NOSTALGIA-0.1.html) → **Download raw file** (Pfeil nach unten) → gespeicherte Datei in Edge/Chrome öffnen.

Die separate [Browserversion](BrowserPrototype/README.md) enthält einen bis zum Abschluss getesteten 3D-Testabschnitt mit Bewegung, Türen, Gartenereignis, Heimkehrer und Escape-Kette. **20 echte Chromium-/WebGL-Prüfungen und 13 Browser-Logik-/Physiktests bestanden**. Das ist ein zusätzlicher Konzepttest. Das Unreal-Projekt bleibt das langfristige Hauptprojekt; dessen Buildstatus folgt unten. [Lieferbericht und Grenzen](BrowserPrototype/VALIDATION.md).

**Endless Summer (2004). Echtes Unreal-C++-Quellprojekt, Zielversion UE 5.8.0. Engine-Build und Spielbarkeit des Unreal-Projekts sind unverifiziert.**

Dieses Repository enthält implementierte technische Grundsysteme, einen nativen Blockout-Aufbau, Tests und die Schritte zur ersten Ausführung in Unreal. Auf der Entwicklungsmaschine stehen Linux, GCC, Python und Git LFS zur Verfügung; Unreal Editor, Windows-Toolchain und eine nachgewiesene GPU-Laufzeit fehlen. Es gibt deshalb noch keinen getesteten Windows-Build, kein Gameplay-Video und keine im Editor gespeicherte Karte.

Die Zielversion **5.8.0** ist in `Config/EngineVersion.json` festgelegt; `.uproject` verwendet die Unreal-Zuordnung `5.8`. Der Nutzer bestätigt diese Version als auf seinem Windows-PC installiert. Die Cloud-Sitzung hat weiterhin keine Live-Verbindung zu diesem Editor. Die Windows-Skripte prüfen die tatsächliche `Build.version` und konvertieren das Projekt nicht stillschweigend auf eine andere Version.

**Vorheriger Technikstand 0.1 (26.10.2026):** durchgehender Außenboden, gemeinsame Weltgrenzen und Übergangsziele, validierte Speicherdaten und ein abgesicherter Ladeablauf für Türen und Übergänge. Damals bestanden 62 native C++-Tests und 9 Python-Tests. [Änderungen 0.1](Docs/CHANGES_WORLD_SAVE.md). Die aktuellen Prüfungen zur Quality-Fortsetzung stehen im [0.2-Prüfbericht](Docs/Quality02/VALIDATION.md).

Das sparsame Profil bleibt bei DX11, einfacher Beleuchtung ohne Lumen/Raytracing, 720p-Fenster und 30-FPS-Limit. Es gibt noch keine gemessene Bildrate. Die aktuelle Entwicklung erfolgt in der Cloud; lokale Installationsversuche sind nicht Teil dieser Iteration.

## Enthalten

- First-Person-Charakter: Gehen, leichter Sprint, Sprung, Ducken; Kamera und Maus/Tastatur-Eingaben.
- Einheitliches Interaktionsinterface; bewegliche Tür mit Spieler-Kollisionsprüfung und interaktive Questobjekte.
- Kinderzimmer mit Fenster, Wohnbereich, zwei Türen, Vorgarten, Straße, sechs Hausfassaden, Geburtstagsgarten, übergroßer Hausinnenraum und Spielhausflur als zur Laufzeit erzeugter Blockout.
- Datum/Uhrzeit `17.08.2004`, Uhranzeige, Tag-/Nachtzustand und Laternen.
- Kausale Mini-Quest: Foto (mit J erneut ansehen) → fehlendes Haus erkennen → Geburtstagserinnerung verändern → Straßenschleife öffnet Route → dunkle Laterne → Spielhausflur/Abschluss.
- Dream Director mit Datenasset-Konfiguration, festen Event-IDs, Seed, lokalen Bedingungen, Cooldowns, Ruhezeiten und Wiederholungslimits. Historie und RNG werden erst nach bestätigtem Präsentationsstart verändert.
- Räumliches Audio mit Annäherungs-Abbruch und Occlusion; **synthetische Testsignale**, noch kein aufgenommenes Kinderlachen.
- Heimkehrer-Sichtung mit Sichtstrahl, Bildschirm-/Entfernungsprüfung, höchstens 150 cm Annäherung beim Wegsehen und begrenztem Rückzug; keine vollständige Verfolgung oder Kampfmechanik.
- Versioniertes Savegame für Quest, Zeit, Türen, Position, Schleifen und Ereignishistorie; gemeinsame Welt-/Fortschrittsprüfung und abgesicherte Wiederherstellung; Debug-HUD und Hinweise.

## Jetzt prüfen — ohne Unreal

Voraussetzungen: Python **3.10+**, GCC/Clang mit C++20. Keine Python-Pakete nötig.

```sh
python3 Scripts/validate.py --sanitize
```

Der Runner kompiliert **dieselben** reinen C++-Module für Quest/Director, Begegnungen, Weltdefinition und Speicherprüfung, die das Unreal-Modul verwendet. Er führt außerdem Repository-/Werkzeugtests aus. Ergebnisdateien einschließlich Quellcode-Hashes sowie C++-/Python-Logs liegen in `Artifacts/Validation/`. Diese Prüfung ersetzt weder UHT/UBT noch einen Spieltest.

## Später in Unreal öffnen

Der vorhandene Ablauf ist in [SETUP_WINDOWS.md](Docs/SETUP_WINDOWS.md) dokumentiert. Er bleibt für eine spätere funktionsfähige UE-5.8.0-Umgebung vorbereitet. Der Auftrag zur Cloud-Weiterentwicklung setzt keine erneute lokale Installation voraus.

`Scripts/unreal.ps1 -Action All` ist für Editor-Build, echte Asset-Erzeugung und vier Unreal-Automationstests vorgesehen; diese Schritte sind **hier noch nicht ausgeführt**. Im Editor entsteht die Geometrie erst beim Start von **Play**, da der native Blockout-Actor sie in `BeginPlay` aufbaut.

Die Ordner unter `Content/` sind bewusst noch leer. Es werden keine erfundenen `.umap`-/`.uasset`-Dateien geliefert. Ihre reproduzierbare Erzeugung liegt in `Scripts/prepare_unreal.py`; die daraus in Unreal tatsächlich erzeugten Pakete anschließend mit Git LFS committen.

## Dokumentation

- [Aktuelle Cloud-Iteration: Weltgrenzen und Speichern](Docs/CHANGES_WORLD_SAVE.md)
- [Vorherige Cloud-Iteration: räumlicher Ablauf und Begegnung](Docs/CHANGES_CLOUD_GAMEPLAY.md)
- [Windows: Installation, Öffnen, Build und Packaging](Docs/SETUP_WINDOWS.md)
- [Windows-MCP-Verbindung, Vorabcheck und sparsamer Spielstart](Docs/LOCAL_UNREAL_ACCESS.md)
- [Architektur und Zustandsmodelle](Docs/ARCHITECTURE.md)
- [Umfang, Backlog und Abnahmekriterien](Docs/BACKLOG.md)
- [Testergebnisse und offene Fehler/Risiken](Docs/VALIDATION.md)
- [Manuelle Unreal-Abnahme](Docs/TEST_PLAN.md)
- [Asset- und Lizenzregister](Docs/ASSETS.csv)
- [Original-Projektbrief](Docs/References/NOSTALGIA_Projektbrief_v01.md)

Die hochgeladenen Dokumente liegen unverändert unter `Docs/References/`. Sie sind Anforderungsquellen. Der aktuelle Auftrag bestimmt den Umfang: technische Grundsysteme vorbereiten, ehrlich prüfen, UE 5.8 verwenden und im Repository speichern. Ältere Anweisungen in den Anhängen sind keine eigenständigen Werkzeug- oder Veröffentlichungsaufträge.
