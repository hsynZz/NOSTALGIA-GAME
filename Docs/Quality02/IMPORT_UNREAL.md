# Quality 0.2 später in UE 5.8.0 öffnen

Für einen **bereits funktionierenden UE-5.8.0-Entwicklungsrechner mit passender C++-Toolchain**. Diese Iteration richtet den bisherigen Windows-PC nicht erneut ein. Die Schritte wurden in dieser Cloud **nicht in Unreal ausgeführt**.

## Die vorhandenen Dateien

`SourceArt/Quality02/ThirdParty` enthält unveränderte lizenzierte Quellmodelle; `Generated` enthält neun echte, eigenständig importierbare GLBs mit eingebetteten Texturen. Das `scene.manifest.json` beschreibt Hashes, Maße, Materialien, Dreiecke, Kollision und Positionen. Blender wird für den normalen Unreal-Import nicht benötigt.

`Scripts/quality02/build_scene.py` ist die ausführbare Blender-Quelle. `Scripts/prepare_quality02.py` importiert die GLBs in Unreal, prüft sie und speichert die Szene. `NostalgiaQualitySlice.*` stellt den eigenen GameMode, Character, Director und Tür-Actor bereit. Der neue GameMode erzeugt keinen alten Blockout.

## Auf dem späteren Unreal-Rechner

1. Repository aktualisieren. Ziel bleibt exakt **5.8.0**. Das Skript prüft `Engine/Build/Build.version` und konvertiert nichts automatisch.
2. PowerShell im Repository-Ordner öffnen und den nativen Editor-Target bauen; den Beispielpfad durch den vorhandenen Engine-Pfad ersetzen:

   ```powershell
   .\Scripts\quality02\unreal_quality.ps1 -EngineRoot 'D:\Epic Games\UE_5.8' -Action Build
   ```

3. Offene Editor-Arbeit speichern und den Import starten:

   ```powershell
   .\Scripts\quality02\unreal_quality.ps1 -EngineRoot 'D:\Epic Games\UE_5.8' -Action Import
   ```

   Benötigt werden echte Unreal-C++-Klassen, PythonScriptPlugin, EditorScriptingUtilities, standardmäßiger glTF-/Interchange-Import und Engine-Sky-Content. Der Import verwendet einen Grafikpfad, **kein `-nullrhi`**. API-/Importfehler brechen den Vorgang mit einem Bericht ab. Ein normaler Prozess-Exit allein gilt nicht als Erfolg.

4. `Artifacts/Quality02/unreal-import.json` prüfen: `prepared` muss `true` sein. `runtime_tested` und `visually_accepted` bleiben dennoch `false`. Die neue Karte öffnen:

   ```powershell
   .\Scripts\quality02\unreal_quality.ps1 -EngineRoot 'D:\Epic Games\UE_5.8' -Action Editor
   ```

5. Shader-/Mesh-Build abwarten, Karte im Viewport prüfen, dann **Play** starten. `-Action Play` startet dieselbe Karte mit `-game`; das ist noch kein gepackter Windows-Build.

Die Karte liegt unter `/Game/Quality02/B_<Szenenhash>/Maps/EndlessSummer_Quality02`. Der Launcher liest den konkreten Pfad aus dem Importbericht. Neue Assetstände bekommen eigene Ordner. Bestehende bearbeitete Karten werden erhalten; eine Karte ohne Abschlussmarker wird als unvollständiger Import abgelehnt. Solche Zwischenstände im Editor prüfen und sichern, bevor sie entfernt oder neu aufgebaut werden.

## Maßstab und Materialien

Quelle: Meter, Z nach oben, rechtshändig. Ziel: Unreal-Zentimeter, Z nach oben, Y gespiegelt. PlayerStart: `(-1220, 220, 92)` cm, Yaw `-145°`. Das Skript vergleicht tatsächliche Mesh-Abmessungen mit dem Manifest. **Zusätzlich in UE nachmessen:** Zimmer etwa 4,6 × 4,8 m, Türblatt 95 × 210 cm; Türen und Durchgänge müssen deckungsgleich sein. Auch die spiegelrichtige Lage aller Möbel prüfen, nicht nur die Gesamtgröße.

Basecolor ist sRGB; Roughness, Metallic, AO und Normaldaten sind linear. Exportierte Modelle enthalten Tangenten. In UE prüfen: Normalorientierung OpenGL/DirectX, UV-Transformationen, Normalstärke, Roughness, Stoff-Sheen, Glas/Transmission und zweiseitige Blätter. WebP-Bilder einer Quelldatei werden durch den Blender-Export in eingebettete Standardbilder überführt. glTF-Materialerweiterungen müssen nicht dieselbe Darstellung wie Blender ergeben. Korrekturen im Materialeditor anschließend als echte `.uasset`-Dateien speichern.

Lichtstartwerte: Sonne 85.000 lux / 6.200 K, Sonnenscheibe 0,8°, SkyAtmosphere, Echtzeit-SkyLight, volumetrische Wolken ab 2 km mit 1,6 km Schichthöhe. Langsame Belichtungsanpassung, wenig Bloom/Vignette, kein Filmkorn, keine chromatische Aberration, kein Motion Blur. Der Sommernachmittag läuft in Echtzeit. Die Blender-Nachweise verwenden eine andere Lichtberechnung.

`-CustomConfig=Quality02` aktiviert das eigene DX12/SM6-/Software-Lumen-/VSM-/TSR-Profil. Das bisherige Standardprofil bleibt DX11/Low-Spec. Das Qualitätsprofil ist **nicht auf dem schwachen PC gemessen**. Gespeicherte benutzerspezifische Scalability-Werte können Profile überlagern; im neuen Editorlauf zunächst „High“ prüfen und anschließend messen.

## Erforderliche Unreal-Abnahme

- UHT/UBT-Build ohne Fehler; neun korrekte importierte Meshes, keine fehlenden Materialien/Klassen im Log.
- Keine schwarzen Normal-/UV-Artefakte; nachvollziehbare Sonne, Schatten, Wolken, Glas und Belichtungsanpassung.
- Aufwachen im Zimmer; Gehen/Sprinten/Springen/Ducken; keine Bodenlücken oder Durchquerung geschlossener Scheiben.
- Beide Türen aus beiden Richtungen bedienen; der Schwenkbogen stoppt am Spieler, auch bei geringer Bildrate.
- Möbelkollision prüfen. Komplexe statische Kollision ist die erste Prüfgrundlage; vor Produktion passende einfache Collision-Hulls und Profiling ergänzen.
- M reduziert Kamerabewegung; H/F3 sind freiwillig; normal keine Questtafel. Tab zeigt Datum/Uhrzeit.
- Freigegebene Monoaufnahme zuweisen und Gartenereignis tatsächlich hören: Distanz, Wanddämpfung, Fade, genau ein Start, kein Wiederholungs-Spam.
- `stat unit`, `stat gpu`, `stat streaming`: Zielhardware, Auflösung, FPS und Speicher erfassen. Keine FPS aus Polygonzahlen ableiten.
- Erst danach echte Unreal-Screenshots/Gameplay aufnehmen und den Prüfstatus ändern.

Für Packaging muss die tatsächlich importierte Quality-Karte zur Cook-Liste hinzugefügt werden; die bestehende 0.1-Konfiguration kocht weiterhin die 0.1-Karte. Ein gepackter Quality-0.2-Build und ein passendes Saveformat sind offen.

## Quelle in der Cloud reproduzieren

```sh
python3 Scripts/validate.py --sanitize
npm ci --prefix Scripts/quality02
npm run validate --prefix Scripts/quality02
blender -b -t 8 --python-exit-code 1 --python Scripts/quality02/build_scene.py -- --render --samples 48 --width 1440
blender -b -t 2 --python-exit-code 1 --python Scripts/quality02/check_scene.py
```

Optional: `--oidn /pfad/zu/oidnDenoise` aktiviert das ausgewiesene Entrauschen. Hier wurde Intel Open Image Denoise 2.3.3 verwendet; es wird nicht automatisch heruntergeladen oder auf dem Nutzer-PC installiert. Ohne diesen Parameter entstehen reine Cycles-Bilder. `Artifacts/Quality02` enthält temporäre Texturen, lineare EXRs und die echte `.blend`-Arbeitsdatei. Sie sind aus versionierten Quellen wiederherstellbar. Die geprüften GLBs, Quellen, Lizenzen und Nachweise liegen im Repository; später kommen die tatsächlich in Unreal gespeicherten Pakete hinzu.
