# Validierungsstand Prototype 0.1

Stand: 23.10.2026. **Quellprototyp implementiert; Unreal-Build, Windows-Paket und Spielbarkeit unverifiziert.**

## Umgebung und überprüfte Ergebnisse

- Repository war ohne Commits und Dateien; vorhandener Checkout wurde verwendet. Kein Worktree angelegt.
- Linux x86_64, GCC 14.2.0, Python 3, Git LFS 3.6.1 verfügbar. Keine Unreal-Binaries auf dem Pfad bzw. in den untersuchten Installationsverzeichnissen, keine UE-bezogenen Umgebungsvariablen. Kein nachgewiesener GPU-Zugang, keine Windows-Toolchain und keine PowerShell-Laufzeit.
- GitHub-Zugriff über die vorhandene Git-Konfiguration funktionierte lesend. Der Veröffentlichungsstand wird abschließend anhand des Remote-Commits geprüft.
- Ziel **UE 5.8.0** ist explizit gepinnt. Offizielle Release-Notes konnten nicht abgerufen werden (HTTP 403); Verfügbarkeit, Stabilitätsstatus und API-Kompatibilität werden nicht behauptet.

Ausgeführter Befehl im Repository:

```sh
python3 Scripts/validate.py --sanitize
```

| Prüfung | Ergebnis | Aussagegrenze |
| --- | --- | --- |
| Gemeinsame produktive C++-Logik, GCC C++20, `-Wall -Wextra -Werror -Wpedantic` | 28 Tests bestanden | Keine Unreal-Header oder Engine-Adapter kompiliert |
| AddressSanitizer + UndefinedBehaviorSanitizer | Kein Befund in den ausgeführten Tests | Keine Garantie für ungetestete Pfade oder UE-Objektlebenszyklen |
| Python-/Repository-/Audiogenerator-Prüfungen | 9 Tests bestanden | Statische Konsistenz und echter WAV-Generator, keine Unreal-Python-API-Ausführung |
| UHT / UBT / Linken des UE-Moduls | UNRUN | Engine und Windows-Toolchain fehlen |
| `prepare_unreal.py` in UE | UNRUN | Echte `.umap`-/`.uasset`-Erzeugung noch offen |
| Vier vorbereitete Unreal-Automationstests | UNRUN | Nicht als bestanden zählen |
| PowerShell-Wrapper unter Windows | UNRUN | Keine PowerShell auf dieser Maschine |
| PIE, Soundausgabe, Physik, Rendern, UI und Datenträger-Saves | UNRUN | Editor/GPU fehlen |
| Windows Cook/Package und EXE-Smoke-Test | UNRUN | Keine ausführbare Auslieferung vorhanden |
| GitHub-Actions-Lauf | Nicht aus lokalem Test abgeleitet | Workflow eingecheckt; Status separat auf GitHub prüfen |

Native Prüfungen umfassen die vollständige Quest-Kette, frühe/verbotene Aktionen, wiederholte Schleifen, Debounce, Nachtbedingung, Abschlusszustand, Uhrgrenzen, ungültige Werte, lokale Events, sichere Bereiche, Bedrohung, Ruhezeiten, Seed-Replay, wiederhergestellte Historie, kaputte Zustände und Eventregel-Grenzen. Ein Snapshot-Test prüft Core-Zustandskopien; tatsächliche Unreal-Serialisierung hat zusätzlich einen eigenen, noch nicht ausgeführten UE-Test.

Der Runner erzeugt frische Logs und JSON unter `Artifacts/Validation/`. Ein geprüfter Beleg wird unter `Docs/Validation/` archiviert. Keine Testzahl stammt aus einer Simulation oder einem Null-Test-Lauf.

## Offene Fehler und Risiken

| ID | Art | Status / nächste Prüfung |
| --- | --- | --- |
| ENV-01 | Externe Voraussetzung | UE 5.8.0 + dazu passende Windows-Werkzeuge installieren; erster UHT/UBT-Lauf steht aus |
| ENV-02 | Versionsnachweis | Offizielle 5.8.0-Verfügbarkeit/Release-Status prüfen; bei Abweichung bewusst neu entscheiden |
| INT-01 | Ungeprüfte Integration | C++-/Python-API-Kompatibilität, Reflection, Material- und Map-Erzeugung in echter 5.8.0 prüfen |
| INT-02 | Ungeprüfte Interaktion | Türblatt, Kollisionsprüfung, Portalpositionen und Loop-Richtung können erst in PIE abgenommen werden |
| INT-03 | Ungeprüfte Darstellung | Belichtung/Nachtlesbarkeit, Textpositionen, Silhouettensichtung und Audio-Occlusion prüfen |
| INT-04 | Ungeprüfte Persistenz | Datenträger-Save, Laden bei Hindernissen und Übergängen sowie Neustart testen |
| INT-05 | Ungeprüfter Build | Cook dynamisch geladener Assets, Editor-Plugin-Ausschluss und Windows-Start prüfen |
| SCOPE-01 | Bekannte Einschränkung | Nur Blockout und Testklänge; keine fertigen Foto-/Stimmen-/Animationsassets |
| SCOPE-02 | Bekannte Einschränkung | Keine Verfolgung, kein Streaming, kein Kriechen/Klettern, keine weiteren Kapitel |

Es gibt aktuell keine fehlgeschlagenen **ausgeführten Offline-Tests**. Ausbleibende Engine-Tests sind weiterhin offen und werden nicht zu Erfolgen umgedeutet. Die Liste enthält mögliche Integrationsprobleme, keine als beobachtet ausgegebenen Laufzeitfehler.
