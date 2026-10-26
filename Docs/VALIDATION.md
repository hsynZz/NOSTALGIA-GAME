# Validierungsstand Prototype 0.1

Stand: 26.10.2026. **Quellprototyp implementiert; Unreal-Build, Windows-Paket und Spielbarkeit unverifiziert.**

## Umgebung und überprüfte Ergebnisse

- Der vorhandene Checkout wurde weiterverwendet. Diese Iteration baut auf `4088dc6` auf; das Projekt wurde nicht neu begonnen.
- Linux x86_64, GCC 14.2.0, Python 3, Git LFS 3.6.1 verfügbar. Keine Unreal-Binaries auf dem Pfad bzw. in den untersuchten Installationsverzeichnissen, keine UE-bezogenen Umgebungsvariablen. Kein nachgewiesener GPU-Zugang, keine Windows-Toolchain und keine PowerShell-Laufzeit.
- GitHub-Zugriff über die vorhandene Git-Konfiguration funktionierte lesend. Der Veröffentlichungsstand wird abschließend anhand des Remote-Commits geprüft.
- Ziel **UE 5.8.0** ist explizit gepinnt und inzwischen vom Nutzer als auf Windows installiert bestätigt. Die aktuelle Cloud-Sitzung bietet keine Unreal-MCP-Werkzeuge; die untersuchte Konfiguration enthält keinen Unreal-Server. Kein Zugriff auf den Windows-Editor. API-/Build-Kompatibilität bleibt unbestätigt.

Ausgeführter Befehl im Repository:

```sh
python3 Scripts/validate.py --sanitize
```

| Prüfung | Ergebnis | Aussagegrenze |
| --- | --- | --- |
| Gemeinsame produktive C++-Logik, GCC C++20, `-Wall -Wextra -Werror -Wpedantic` | 62 Tests bestanden | Keine Unreal-Header oder Engine-Adapter kompiliert |
| AddressSanitizer + UndefinedBehaviorSanitizer | Kein Befund in den ausgeführten Tests | Keine Garantie für ungetestete Pfade oder UE-Objektlebenszyklen |
| Python-/Repository-/Audiogenerator-Prüfungen | 9 Tests bestanden | Statische Konsistenz und echter WAV-Generator, keine Unreal-Python-API-Ausführung |
| UHT / UBT / Linken des UE-Moduls | UNRUN | Engine und Windows-Toolchain fehlen |
| `prepare_unreal.py` in UE | UNRUN | Echte `.umap`-/`.uasset`-Erzeugung noch offen |
| Vier vorbereitete Unreal-Automationstests | UNRUN | Nicht als bestanden zählen |
| PowerShell-Wrapper unter Windows | UNRUN | Keine PowerShell auf dieser Maschine |
| PIE, Soundausgabe, Physik, Rendern, UI und Datenträger-Saves | UNRUN | Editor/GPU fehlen |
| Windows Cook/Package und EXE-Smoke-Test | UNRUN | Keine ausführbare Auslieferung vorhanden |
| GitHub-Actions-Lauf | Nicht aus lokalem Test abgeleitet | Workflow eingecheckt; Status separat auf GitHub prüfen |

Native Prüfungen umfassen nun zusätzlich Plan/Bestätigung, fehlgeschlagene Präsentation, veraltete Bestätigungen, Wiederholbarkeit bei Fehlern, Sicht-/Entfernungsbedingungen, begrenzte Annäherung und zeitliche Recovery des Heimkehrers. Außerdem prüfen sie die vollständige Quest-Kette, frühe/verbotene Aktionen, wiederholte Schleifen, Debounce, Nachtbedingung, Abschlusszustand, Uhrgrenzen, ungültige Werte, lokale Events, sichere Bereiche, Bedrohung, Ruhezeiten, Seed-Replay, wiederhergestellte Historie, kaputte Zustände und Eventregel-Grenzen. Ein Snapshot-Test prüft Core-Zustandskopien; tatsächliche Unreal-Serialisierung hat zusätzlich einen eigenen, noch nicht ausgeführten UE-Test.

Der Runner erzeugt frische Logs und JSON unter `Artifacts/Validation/`. Belege unter `Docs/Validation/`: `offline-2026-10-23.json` gehört zum ursprünglichen Stand mit 28 Core-Tests; `offline-low-spec-2026-10-23.json` dokumentiert die Fortsetzung mit 30 Core-Tests und geänderten Quelldatei-Hashes. Die zwei zusätzlichen Tests prüfen echte gemeinsame Bezirkslogik für Wohnbereich, Loop-Ziel und Ereignisorte. Der Bericht `offline-cloud-gameplay-2026-10-23.json` belegt den vorherigen Stand mit 47 Core- und 9 Python-Tests. Der aktuelle Bericht `offline-world-save-2026-10-26.json` und die zugehörigen Logs belegen 62 Core- und 9 Python-Tests; 15 neue Tests betreffen Weltgrenzen, Bodenabdeckung, Snapshots und Türgeometrie. Alle Tests führen echte Funktionen aus; Engine-Adapter und räumliche Wahrnehmung wurden dabei nicht ausgeführt.

## Offene Fehler und Risiken

| ID | Art | Status / nächste Prüfung |
| --- | --- | --- |
| ENV-01 | Externe Voraussetzung | UE 5.8.0 laut Nutzer installiert; lokale C++-Einrichtung problematisch. Keine weiteren Installationsversuche im aktuellen Auftrag. Erster UHT/UBT-Lauf bleibt für geeignete Umgebung offen |
| ENV-02 | Verbindungsblocker | Keine Live-Unreal-Werkzeuge in dieser Sitzung. Cloud-Codearbeit wird fortgesetzt; lokaler Editorzugriff bleibt unbestätigt |
| INT-01 | Ungeprüfte Integration | C++-/Python-API-Kompatibilität, Reflection, Material- und Map-Erzeugung in echter 5.8.0 prüfen |
| INT-02 | Ungeprüfte Interaktion | Türblatt, Kollisionsprüfung, Portalpositionen und Loop-Richtung können erst in PIE abgenommen werden |
| INT-03 | Ungeprüfte Darstellung | Belichtung, Fotoansicht, neue Flure, Bildschirmprojektion, Silhouettensichtung, Audioausgabe und Occlusion prüfen |
| INT-04 | Ungeprüfte Persistenz | Datenträger-Save, Laden bei aktueller/gespeicherter Türstellung, Restore-Sperre, Fehler-Rollback und Neustart testen |
| INT-05 | Ungeprüfter Build | Cook dynamisch geladener Assets, Editor-Plugin-Ausschluss und Windows-Start prüfen |
| SCOPE-01 | Bekannte Einschränkung | Nur Blockout und Testklänge; keine fertigen Foto-/Stimmen-/Animationsassets |
| SCOPE-02 | Bekannte Einschränkung | Begrenzte Annäherung statt vollständiger Verfolgung; kein Streaming, kein Kriechen/Klettern, keine weiteren Kapitel |

Es gibt aktuell keine fehlgeschlagenen **ausgeführten Offline-Tests**. Ausbleibende Engine-Tests sind weiterhin offen und werden nicht zu Erfolgen umgedeutet. Die Liste enthält mögliche Integrationsprobleme, keine als beobachtet ausgegebenen Laufzeitfehler.

## In dieser Iteration behobene Quellcodeprobleme

- Director verbrauchte Ereignisse vor erfolgreicher Präsentation; jetzt separates Planen/Bestätigen und Verfügbarkeitsfilter, nativ getestet.
- Heimkehrer erschien ohne Sicht-/Entfernungsprüfung; Sensoradapter ergänzt, Core-Grenzen nativ getestet. Tatsächliche Sichtstrahlen und Capsule-Sweeps bleiben ungetestet.
- Garten-FadeOut konnte jeden Frame erneut aufgerufen werden; jetzt einmalige Ausblendung und Freigabe beendeter Audio-Komponenten. UE-Laufzeitprüfung offen.
- Loop-Überlappung während der Übergangssperre ging verloren; jetzt begrenzter Retry-Tick, UE-Überlappungsprüfung offen.
- Spielhaus führte unmittelbar zu Complete; jetzt eigener Flur mit Endtür, Rückweg und Quest-Sperre, räumliche Abnahme offen.

Änderungen und genaue Grenzen: [CHANGES_CLOUD_GAMEPLAY.md](CHANGES_CLOUD_GAMEPLAY.md).

Die aktuelle Fortsetzung behebt Bodenlücken neben/hinter dem Wohnhaus, ungebundene Ladebereiche, unterschiedliche Türstellungen beim Laden sowie Übergangs-Callbacks während der Wiederherstellung. Core-Prüfungen bestanden; UE-Adapter und erweiterter Unreal-Serialisierungstest bleiben UNRUN. Einzelheiten: [CHANGES_WORLD_SAVE.md](CHANGES_WORLD_SAVE.md).
