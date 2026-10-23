# Windows-Verbindung und erster begehbarer Test

Stand: 23.10.2026, Fortsetzung auf dem vorhandenen Projekt. Der Nutzer bestätigt **UE 5.8.0** auf seinem Windows-PC. Es wurde keine andere Engine-Version gewählt.

## Was tatsächlich erreichbar war

Das bestehende GitHub-Repository und sein Linux-Checkout sind erreichbar. In der aktuellen Cloud-Sitzung fehlen `list_toolsets`, `describe_toolset`, `call_tool` und `unreal_mcp_status`; die untersuchte Codex-Konfiguration enthält keinen Unreal-Server. Es gibt auf dieser Maschine keinen Unreal Editor. Port 8000 auf **diesem Linux-Host** nimmt keine Verbindung an. Diese letzte Prüfung sagt ausdrücklich nichts über den Windows-PC aus.

Es fand **kein** Windows-Dateizugriff, Engine-Start, UHT/UBT-Build, PIE-Spieltest oder Shader-/Renderingtest statt. Ein installierter Codex-Skill enthält Anweisungen; er belegt allein noch keine aktive Verbindung zum Editor. Ein Browser-/Cloud-Prozess erreicht über `127.0.0.1` seinen eigenen Rechner, nicht den PC des Nutzers.

Die folgenden Schritte sind deshalb auf dem Windows-PC auszuführen. Für Live-MCP Codex auf demselben Windows-Rechner wie Unreal starten oder eine bereits eingerichtete, ausdrücklich unterstützte Remote-Verbindung verwenden. Den MCP-Port nicht einfach öffentlich ins Internet freigeben.

## 1. Vorhandenen Checkout aktualisieren

Wenn das Projekt bereits auf dem PC liegt, **dessen tatsächlichen Ordner** verwenden, keinen zweiten Projekt-Neustart anlegen. Beispiel:

```powershell
cd "C:\Projects\NOSTALGIA-GAME"
git status --short
git pull --ff-only origin main
git lfs pull
```

Falls Git eigene Änderungen meldet, diese erhalten; kein `reset --hard` oder erzwungenes Überschreiben. Falls noch kein Checkout vorhanden ist:

```powershell
git clone https://github.com/hsynZz/NOSTALGIA-GAME.git C:\Projects\NOSTALGIA-GAME
cd C:\Projects\NOSTALGIA-GAME
git lfs install --local
git lfs pull
```

Die Projektdatei heißt danach `C:\Projects\NOSTALGIA-GAME\NOSTALGIA.uproject`. In allen folgenden Befehlen den Engine-Pfad ändern, falls der Launcher ein anderes Installationsverzeichnis verwendet.

## 2. Voraussetzungen prüfen

```powershell
.\Scripts\windows_preflight.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8"
```

Das Skript liest die echte Engine-Version, prüft Editor/Build-Werkzeuge, MSVC-C++-Installation, Windows-SDK-Header und die beiden MCP-Plugins. Es verändert weder `.uproject`, Client-Konfiguration noch Firewall. Ergebnis: `Artifacts\WindowsPreflight\report.json`.

`BLOCKED` bei C++ oder SDK: Visual Studio Installer öffnen, die Workload **Spieleentwicklung mit C++** und die von UE 5.8 unterstützten MSVC-/Windows-SDK-Komponenten installieren, Vorabcheck wiederholen. Das Skript erkennt deren Vorhandensein; die exakte Compilerkompatibilität bestätigt erst UnrealBuildTool. Ein bloßer Engine-Download ersetzt diese Werkzeuge nicht.

`CHECK` bei MCP ist für manuelles Spielen kein Build-Blocker. Es muss jedoch behoben sein, bevor Codex den Editor steuern kann. Falls PowerShell lokale Skripte wegen ihrer Execution Policy nicht startet: Skript zunächst lesen, dann beispielsweise `powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\Scripts\windows_preflight.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8"` verwenden. Das setzt die Richtlinie nur für diesen Prozess; keine dauerhafte globale Lockerung ist nötig.

## 3. Bestehendes C++-Projekt bauen und Karte erzeugen

Unreal schließen, um Änderungen an C++-/Reflection-Klassen vollständig neu zu bauen. Im selben Projektordner:

```powershell
.\Scripts\unreal.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8" -Action All
```

`All` führt nacheinander **Build → Prepare → vier Unreal-Automationstests** aus. Bei einem Fehler stoppt das Skript. Auf einem sehr knappen Rechner `-MaxParallelActions 1` ergänzen; der Standard begrenzt UBT auf zwei parallele Aktionen. Den ersten Shader-Build abwarten; weder Editor- noch Compilerprozesse wahllos beenden.

Die Karte und `.uasset`-Pakete entstehen erst bei diesem echten Unreal-Lauf. Bestehende Pakete werden erhalten; die C++-Blockout-Erweiterung wird beim nächsten Play automatisch aufgebaut. Die Fenster-/Raumgeometrie steht deshalb nicht schon als einzelne Actor-Sammlung im Editor-Outliner.

Bei Fehlern sammeln: vollständige erste Compilerfehlermeldung, `Artifacts\Unreal\prepare.log` bzw. den aktuellen Automation-Ordner und `Artifacts\WindowsPreflight\report.json`. Keine Passwörter oder Tokens eintragen. Fehlende Plugins oder eine fehlgeschlagene Klasse nicht durch erfundene `.uasset`-Dateien ersetzen.

## 4. Mit sparsamen Einstellungen spielen

```powershell
.\Scripts\unreal.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8" -Action Play
```

Dies startet die vorhandene Karte über die installierte Editor-Engine im `-game`-Modus, **noch kein eigenständiges Windows-Paket**. Alternativ für PIE:

```powershell
.\Scripts\unreal.ps1 -EngineRoot "C:\Program Files\Epic Games\UE_5.8" -Action Editor
```

Im Editor auf **Play** klicken und in das Spielfenster klicken. Der Startweg ist: Kinderzimmer → Foto auf dem Tisch mit E ansehen → Zimmertür mit E öffnen → Wohnbereich → Haustür mit E öffnen → Vorgarten → Straße. Danach Garten rechts der Straße, Fotovergleich, Geburtstagstisch, STOP-Schleife, dunkle Laterne und Spielhaus. `H` zeigt Hinweise; `F3` zeigt Debugzustände. Steuerung und ausführliche Abnahme stehen in [TEST_PLAN.md](TEST_PLAN.md).

Das Profil nutzt **DX11/SM5, 1280×720 Fenster, 75 % interne Auflösung, höchstens 30 FPS**, einfache Schatten, kein Lumen, keine virtuellen Schatten und kein Raytracing. Raum-/Laternenlichter werfen keine eigenen Schatten. Die Startbefehle setzen das Qualitätsprofil auch dann für den Prozess, wenn alte Benutzereinstellungen vorhanden sind. Das sind Kostenbegrenzungen, keine nachgewiesene FPS-Garantie auf dem PC. Bei Bedarf `r.ScreenPercentage 60` in der UE-Konsole setzen; das Spielkonzept bleibt gleich.

## 5. Live-Unreal-Verbindung für Codex herstellen

Erst sicherstellen, dass genau **dieses NOSTALGIA-Projekt** im Editor geöffnet ist. Änderungen speichern.

1. **Edit → Plugins** öffnen. `ModelContextProtocol` und `AllToolsets` müssen installiert und für dieses Projekt aktiviert sein. `AllToolsets` stellt die eigentlichen Werkzeuge bereit. Falls sie fehlen, die zu dieser Integration gehörenden Engine-Plugins für **5.8.0** installieren. Das Codex-Skillpaket allein installiert keinen laufenden Unreal-Server.
2. Die Aktivierung schreibt Einträge in `NOSTALGIA.uproject`. Vorhandene Einträge erhalten; für das Editor-Target können diese zwei Einträge innerhalb des bestehenden `Plugins`-Arrays verwendet werden:

   ```json
   { "Name": "ModelContextProtocol", "Enabled": true, "TargetAllowList": ["Editor"] },
   { "Name": "AllToolsets", "Enabled": true, "TargetAllowList": ["Editor"] }
   ```

   Das ist ein JSON-Ausschnitt, keine komplette Projektdatei. Hier wurden diese Abhängigkeiten nicht blind im Git-Stand aktiviert, weil ihre lokale Installation nicht zugänglich ist.
3. Editor nach Plugin-Aktivierung neu starten. In **Output Log** unten in der Befehlszeile `ModelContextProtocol.StartServer` ausführen. Eine Erfolgsmeldung abwarten. Mit aktivierten Plugins ist auch `-Action Editor -StartMcp` im Startskript möglich.
4. Codex **lokal auf diesem Windows-PC** mit dem Projektordner als Arbeitsverzeichnis öffnen. Wenn das installierte Plugin bereits eine Verbindung zu `http://127.0.0.1:8000/mcp` bereitstellt, diese verwenden. Keine zweite parallele Verbindung anlegen.
5. Nur wenn keine Verbindung konfiguriert ist: In Unreal `ModelContextProtocol.GenerateClientConfig Codex` ausführen. Bei einer Launcher-Installation liegt die erzeugte `.codex\config.toml` neben `.uproject`. Eine bestehende Datei wird nicht überschrieben; darin andere Einstellungen erhalten. Alternativ in der vorhandenen Konfiguration genau einen Eintrag setzen:

   ```toml
   [mcp_servers.unreal-mcp]
   url = "http://127.0.0.1:8000/mcp"
   ```

   Bei einem Engine-Source-Build liegt der Konfigurationsort am Engine-Workspace statt zwingend neben dem Projekt.
6. Codex-Verbindung neu laden bzw. lokalen Codex-Client neu starten. Verifikation: `list_toolsets` muss erfolgreich antworten; anschließend ein **lesender Editor-Aufruf** muss den aktuellen Projekt-/Levelzustand liefern. Erst dann ist tatsächlicher Zugriff bestätigt.

Für dauerhaftes Auto-Start optional in der **lokalen, ignorierten** Datei `Saved\Config\WindowsEditor\EditorPerProjectUserSettings.ini` ergänzen:

```ini
[/Script/ModelContextProtocolEngine.ModelContextProtocolSettings]
bAutoStartServer=True
```

Falls bereits `unreal-mcp-proxy` eingerichtet ist: diesen erhalten, keinen zweiten direkten Server hinzufügen. Proxy-Status oder sichtbare Werkzeuge allein belegen keine aktuelle Erreichbarkeit. Unreal starten, nötigenfalls `ModelContextProtocol.RefreshTools` ausführen und Codex neu verbinden; mit einem lesenden Editor-Aufruf nachprüfen. Aktive Proxys nicht ungefragt beenden.

## 6. Abnahme und Rückgabe

Die vier Automationstests und die vollständige [manuelle Testliste](TEST_PLAN.md) abarbeiten. Beide Türen, sicheren Wohnbereich, freie Loop-Zielposition, Nachtlesbarkeit, örtliches Audio, Sichtung und Save/Load besonders prüfen. Anschließend echte `.umap`-/`.uasset`-Dateien über LFS committen. Erst nach einem tatsächlichen Durchlauf darf dieser Stand „funktionierende, begehbare Version“ heißen.

Für den nächsten lokalen Codex-Auftrag reicht: **„Nutze das vorhandene NOSTALGIA-Projekt. Prüfe zuerst die Live-Verbindung mit einem lesenden Unreal-Aufruf. Lies Docs/LOCAL_UNREAL_ACCESS.md und Docs/CHANGES_LOW_SPEC.md. Baue das Projekt, behebe echte Buildfehler, führe Automation und PIE-Abnahme aus; erhalte Konzept und sparsames Grafikprofil.“**
