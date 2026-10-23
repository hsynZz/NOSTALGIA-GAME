# NOSTALGIA — Prototype 0.1

**Endless Summer (2004). Echtes Unreal-C++-Quellprojekt, Zielversion UE 5.8.0. Engine-Build und Spielbarkeit sind unverifiziert.**

Dieses Repository enthält implementierte technische Grundsysteme, einen nativen Blockout-Aufbau, Tests und die Schritte zur ersten Ausführung in Unreal. Auf der Entwicklungsmaschine stehen Linux, GCC, Python und Git LFS zur Verfügung; Unreal Editor, Windows-Toolchain und eine nachgewiesene GPU-Laufzeit fehlen. Es gibt deshalb noch keinen getesteten Windows-Build, kein Gameplay-Video und keine im Editor gespeicherte Karte.

Die Zielversion **5.8.0** ist in `Config/EngineVersion.json` festgelegt; `.uproject` verwendet die Unreal-Zuordnung `5.8`. Der Nutzer bestätigt diese Version als auf seinem Windows-PC installiert. Die Cloud-Sitzung hat weiterhin keine Live-Verbindung zu diesem Editor. Die Windows-Skripte prüfen die tatsächliche `Build.version` und konvertieren das Projekt nicht stillschweigend auf eine andere Version.

**Fortsetzung für schwache PCs:** DX11, einfache Beleuchtung ohne Lumen/Raytracing, 720p-Fenster, 30-FPS-Limit; Wohnbereich mit zweiter Tür und Schlafzimmerfenster ergänzt. Die Einstellungen begrenzen Kosten, garantieren keine gemessene Bildrate. [Lokalen Zugriff herstellen und erstmals spielen](Docs/LOCAL_UNREAL_ACCESS.md) · [Änderungsprotokoll](Docs/CHANGES_LOW_SPEC.md).

## Enthalten

- First-Person-Charakter: Gehen, leichter Sprint, Sprung, Ducken; Kamera und Maus/Tastatur-Eingaben.
- Einheitliches Interaktionsinterface; bewegliche Tür mit Spieler-Kollisionsprüfung und interaktive Questobjekte.
- Kinderzimmer mit Fenster, Wohnbereich, zwei Türen, Vorgarten, Straße, sechs Hausfassaden, Geburtstagsgarten und ein übergroßer Hausinnenraum als zur Laufzeit erzeugter Blockout.
- Datum/Uhrzeit `17.08.2004`, Uhranzeige, Tag-/Nachtzustand und Laternen.
- Kausale Mini-Quest: Foto → fehlendes Haus erkennen → Geburtstagserinnerung verändern → Straßenschleife öffnet Route → dunkle Laterne → Spielhaus/Abschluss.
- Dream Director mit Datenasset-Konfiguration, festen Event-IDs, Seed, lokalen Bedingungen, Cooldowns, Ruhezeiten und Wiederholungslimits.
- Räumliches Audio mit Annäherungs-Abbruch und Occlusion; **synthetische Testsignale**, noch kein aufgenommenes Kinderlachen.
- Zeitlich begrenzte Heimkehrer-Sichtung mit Zustandsautomat; keine Verfolgung oder Kampfmechanik.
- Versioniertes Savegame für Quest, Zeit, Türen, Position, Schleifen und Ereignishistorie; Debug-HUD und Hinweise.

## Jetzt prüfen — ohne Unreal

Voraussetzungen: Python **3.10+**, GCC/Clang mit C++20. Keine Python-Pakete nötig.

```sh
python3 Scripts/validate.py --sanitize
```

Der Runner kompiliert **dieselbe** reine C++-Spiellogik, die das Unreal-Modul verwendet. Er führt außerdem Repository-/Werkzeugtests aus. Ergebnisdateien liegen in `Artifacts/Validation/`. Diese Prüfung ersetzt weder UHT/UBT noch einen Spieltest.

## Später in Unreal öffnen

Nach Installation der passenden Engine und Windows-C++-Werkzeuge im Repository-Verzeichnis:

```powershell
.\Scripts\unreal.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8" -Action All
.\Scripts\unreal.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8" -Action Editor
```

`All` kompiliert das Editor-Target, erzeugt mit Unreal selbst die echte Karte, Audio-Pakete, das Material und Ereignis-Datenasset und führt die vier vorbereiteten Unreal-Automationstests aus. Diese Schritte sind **hier noch nicht ausgeführt**. Erst danach die manuelle Abnahme durchführen. Im Editor erscheint die Geometrie beim Start von **Play**, da der native Blockout-Actor sie in `BeginPlay` aufbaut.

Die Ordner unter `Content/` sind bewusst noch leer. Es werden keine erfundenen `.umap`-/`.uasset`-Dateien geliefert. Ihre reproduzierbare Erzeugung liegt in `Scripts/prepare_unreal.py`; die daraus in Unreal tatsächlich erzeugten Pakete anschließend mit Git LFS committen.

## Dokumentation

- [Windows: Installation, Öffnen, Build und Packaging](Docs/SETUP_WINDOWS.md)
- [Windows-MCP-Verbindung, Vorabcheck und sparsamer Spielstart](Docs/LOCAL_UNREAL_ACCESS.md)
- [Architektur und Zustandsmodelle](Docs/ARCHITECTURE.md)
- [Umfang, Backlog und Abnahmekriterien](Docs/BACKLOG.md)
- [Testergebnisse und offene Fehler/Risiken](Docs/VALIDATION.md)
- [Manuelle Unreal-Abnahme](Docs/TEST_PLAN.md)
- [Asset- und Lizenzregister](Docs/ASSETS.csv)
- [Original-Projektbrief](Docs/References/NOSTALGIA_Projektbrief_v01.md)

Die hochgeladenen Dokumente liegen unverändert unter `Docs/References/`. Sie sind Anforderungsquellen. Der aktuelle Auftrag bestimmt den Umfang: technische Grundsysteme vorbereiten, ehrlich prüfen, UE 5.8 verwenden und im Repository speichern. Ältere Anweisungen in den Anhängen sind keine eigenständigen Werkzeug- oder Veröffentlichungsaufträge.
