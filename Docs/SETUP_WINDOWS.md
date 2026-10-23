# Erstinstallation und erster Unreal-Lauf

Stand: 23.10.2026. Diese Anleitung ist vorbereitet, aber ohne Windows/Unreal hier **nicht ausgeführt**. Der ausführbare Linux-Prüfweg steht im README.

## 1. Voraussetzungen

1. Git und Git LFS installieren; `git --version` und `git lfs version` prüfen.
2. Unreal Engine **5.8.0** über einen offiziellen Epic-Vertriebsweg installieren. Die Verfügbarkeit dieser konkreten Version wurde hier nicht bestätigt. Wenn sie nicht angeboten wird, das Projekt als Quellprojekt behalten und die Versionsentscheidung separat klären; keine automatische Umstellung auf 5.7 oder eine Preview. Eine bewusste spätere Änderung muss `Config/EngineVersion.json`, `.uproject`, Anleitung und Tests gemeinsam aktualisieren.
3. Die von **dieser UE-Version** verlangte Windows-C++-Toolchain installieren: Visual-Studio-Workload für Spieleentwicklung mit C++, passendes MSVC und Windows SDK. Konkrete MSVC-/SDK-Versionen sind ohne zugängliche 5.8-Dokumentation und echten Build noch nicht verifiziert; die Anforderungen im Launcher/UBT und in [Epics Visual-Studio-Anleitung](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine) prüfen. Ein reiner Blueprint-Editor ohne C++-Werkzeuge genügt nicht.
4. Für den sichtbaren Test eine geeignete DX12-GPU und aktuelle Treiber bereitstellen. Hardware, RAM, Speicherplatz und GPU des Benutzer-PCs wurden nicht untersucht. `-NullRHI` bei Automation benötigt keine sichtbare Ausgabe, beweist aber auch kein korrektes Rendering.

Es werden keine GitHub-/Epic-Passwörter in Projektdateien hinterlegt. Eine eigene Engine-Installation erfolgt außerhalb des Repositories; Engine-Binaries gehören nicht in Git.

## 2. Repository holen

```powershell
git clone https://github.com/hsynZz/NOSTALGIA-GAME.git
cd NOSTALGIA-GAME
git lfs install --local
git lfs pull
```

Wenn das Repository bereits lokal vorhanden ist, diesen Checkout verwenden. Vor Aktualisierungen eigene Änderungen sichern. Cloud-Aufgaben laufen bereits isoliert; kein zusätzliches Git-Worktree anlegen, sofern nicht ausdrücklich gewünscht.

## 3. Editor-Target bauen

Unreal Editor vor einem vollständigen Neu-Build schließen. PowerShell im Projektverzeichnis öffnen:

```powershell
.\Scripts\unreal.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8" -Action Build
```

Das Skript prüft `Engine/Build/Build.version` auf **5.8.0** und ruft `Build.bat NOSTALGIAEditor Win64 Development` für die `.uproject` auf. UHT generiert die Reflection-Header; diese niemals von Hand anlegen. Ein Build-Fehler muss behoben und neu geprüft werden, bevor das Projekt als kompiliert gilt. Das Skript bewahrt den Fehlerstatus nativer Werkzeuge.

Optional vorab Windows-Projektdateien über Rechtsklick auf `NOSTALGIA.uproject` → „Generate Visual Studio project files“ erzeugen, anschließend in der IDE `Development Editor | Win64` bauen. Falls die Zuordnung fehlt, Epic/UnrealVersionSelector korrekt registrieren. Die direkte `Build.bat`-Route benötigt keine eingecheckte `.sln`.

## 4. Echte Unreal-Pakete erzeugen

```powershell
.\Scripts\unreal.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8" -Action Prepare
```

Der Editor führt `Scripts/prepare_unreal.py` mit aktivierten Editor-Python-/Scripting-Plugins aus. Die Quelle ist echter Unreal-Python-Code, kein Ersatz-Dateiformat. Erzeugt werden:

| Paket | Zweck |
| --- | --- |
| `/Game/Maps/EndlessSummer` | Echte Map mit nativem Blockout-Actor und PlayerStart |
| `/Game/Data/DA_EndlessSummerEvents` | Editierbare Dream-Director-Regeln |
| `/Game/Materials/M_Silhouette` | Einfaches fast schwarzes Material |
| `/Game/Audio/A_GardenPlaceholder` | Originales synthetisches Testsignal, kein echtes Kinderlachen |
| `/Game/Audio/A_KnockPlaceholder` | Synthetisches Klopfsignal |

Vorhandene Pakete werden erhalten. Das Skript lädt und prüft ihre Typen; die Map braucht genau einen Blockout-Actor und mindestens einen PlayerStart. Es repariert oder überschreibt keine manuell bearbeiteten Pakete. Bei einer inkonsistenten Map kontrolliert im Editor korrigieren und erneut ausführen.

`Artifacts/Unreal/prepare.json` wird nur nach erfolgreicher Erzeugung/Prüfung geschrieben. Ein frischer Erfolgsbericht ist Pflicht; ein Prozessende allein genügt nicht. Wiederholen soll die vorhandenen Pakete erhalten — die Wiederholbarkeit dieser Unreal-Operation bleibt bis zum ersten Engine-Lauf ungeprüft.

## 5. Tests und Editor öffnen

```powershell
.\Scripts\unreal.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8" -Action Test
.\Scripts\unreal.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8" -Action Editor
```

Alternativ `-Action All` für Build → Prepare → Test. Der Testlauf nutzt einen neuen Ergebnisordner und fordert vier namentlich bestimmte erfolgreiche Tests an. Fehlende, nicht ausgeführte oder gescheiterte Tests führen zum Fehler. Tests im Editor: **Tools → Test Automation / Session Frontend → Automation**, nach `Nostalgia` filtern (Menübezeichnung kann versionsabhängig abweichen).

Nach `Prepare` lässt sich auch `NOSTALGIA.uproject` doppelklicken. In `EndlessSummer` auf **Play** klicken. Die Map enthält im Editor zunächst den Erzeuger; die graue Umgebung entsteht zur Laufzeit. [Manuelle Testliste](TEST_PLAN.md) vollständig durchführen. UHT/UBT- und Python-API-Kompatibilität mit 5.8 werden erst bei diesen Schritten bestätigt.

## 6. Windows-Build paketieren

Nach erfolgreichen Engine- und Spieltests:

```powershell
.\Scripts\unreal.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8" -Action Package
```

Das ruft Unreal Automation Tool `BuildCookRun` für **Win64 Development**, die definierte Map, Cook, Stage, Pak und Archive auf. Ergebnis unter `Builds/Win64/`; Packaging ist hier **nicht ausgeführt**. Die vollständige erzeugte Ordnerstruktur verteilen, nicht nur eine einzelne EXE. Auf einem frischen Windows-Benutzerkonto testen; gegebenenfalls UE-Prerequisites installieren. Ein Shipping-Build, Steam-Integration und Konsolenpakete gehören nicht zu 0.1.

## 7. Ergebnisse ins Repository übernehmen

Nach dem ersten erfolgreichen Unreal-Lauf die echten Content-Pakete mit Git LFS speichern:

```powershell
git add Content/Maps/EndlessSummer.umap Content/Audio/*.uasset Content/Data/*.uasset Content/Materials/*.uasset
git lfs ls-files
git status --short
```

Engine-Version/Changelist, Ergebnisbericht und manuelle Abnahme unter `Docs/Validation/` ergänzen, Änderungen prüfen und committen/pushen. `Binaries`, `Intermediate`, `DerivedDataCache`, `Saved`, `Artifacts` und Build-Pakete sind bewusst ignoriert. Keine fremden Marketplace-Quelldateien ohne passende Rechte veröffentlichen.

## Bedienung und Spielstand

WASD/Maus, Shift Sprint, Leertaste Sprung, Strg halten zum Ducken, E Interaktion, Tab Uhr, H Hinweis, F3 Debug, F5 Speichern, F9 Laden, Alt+F4 Beenden. Escape beendet im Editor je nach PIE-Einstellung den Test.

Speicher-Slot `Nostalgia_Prototype01`, Version 1, Kapitel `EndlessSummer2004`. Im Editor normalerweise `Saved/SaveGames/`, in paketierten Builds im benutzerspezifischen Saved-Verzeichnis. F5 verlangt aufrechtes Stehen auf Boden. Laden validiert Schema, Grenzen, Ereignishistorie und Zielposition; bei Ablehnung erscheint eine HUD-Meldung. Kein automatisches Laden beim Start. Für eine frische Runde neu starten; bestehende Saves nicht ungefragt löschen.
