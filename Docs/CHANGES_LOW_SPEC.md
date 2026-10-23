# Fortsetzung: sparsamer begehbarer Blockout

23.10.2026; Ausgangspunkt: GitHub-Commit `5b94d21`. Bestehendes Projekt und alle vorhandenen C++-Systeme weiterverwendet. UE 5.8.0 vom Nutzer als installierte Version bestätigt. Kein Spielkonzeptwechsel, keine neuen Kapitel.

## Änderungen

- Renderer auf DX11/SM5 mit konventioneller Beleuchtung, ohne Lumen, virtuelle Schatten und Raytracing gestellt. Game-Defaults: 720p-Fenster, 75 % interne Auflösung, 30-FPS-Limit, niedrige Effekte und einfache Schatten. Editor-/Play-Start wendet das Profil explizit an, ohne vorhandene lokale Benutzereinstellungen zu löschen.
- Vorhandenes Kinderzimmer um ein tatsächliches Fensterloch mit unsichtbarer Kollisionssperre und sichtbaren Außenboden ergänzt.
- Wohnbereich zwischen Kinderzimmer und Vorgarten ergänzt: einfacher Sofablock, Tisch, durchgehender mittiger Laufweg, zweite benutzbare Tür. Beide Türen verwenden die vorhandene Tür-Bitmaske des Savegames.
- Sicheren Bereich auf den Wohnraum erweitert. Bezirksklassifizierung wird jetzt als echter gemeinsamer Core-Code von UE und nativen Tests benutzt.
- Frühes Loop-Ziel von X=600 (jetzt Haustür) nach X=950 auf die Straße verlegt. Finale-Route bleibt X=7200. Kein Teleport auf die neue Tür.
- Raum-/Laternenlichter ohne eigene Schatten und mit begrenzter Reichweite; eine einfache schattenlose Nachtaufhellung hält den Prüfpfad sichtbar. Sky-Capture einmal beim Aufbau; keine erneute Aufnahme beim Nachtwechsel. Tatsächliche Lichtqualität ist noch zu prüfen.
- Ruhende Türen führen keinen Kollisions-Overlap pro Frame mehr aus. Text-/Hausvarianten werden nur bei geändertem Zustand aktualisiert.
- `windows_preflight.ps1` ergänzt: lokale Version, Editor/Build, MSVC, SDK, MCP-Plugin-Vorhandensein; keine automatischen Installationen oder Änderungen an Integration/Firewall.
- Startskript um `Play`, `StartMcp` und begrenzte UBT-Parallelität erweitert. Standard zwei parallele Buildaktionen, auf Wunsch eine.
- Genaue lokale Zugriffs-/Build-/Spielanleitung ergänzt. Die MCP-Plugins wurden nicht als unbestätigte Pflichtabhängigkeiten ins Projekt geschrieben.

## Überprüfung und Grenze

`python3 Scripts/validate.py --sanitize`: **30 native C++-Tests und 9 Python-/Repositorytests bestanden**. Zwei neue native Tests prüfen die tatsächlich vom Unreal-Adapter verwendeten Bezirke an Spielorten und die Abgrenzung des Gartenereignisses. AddressSanitizer/UBSan fanden in diesen Läufen keinen Fehler.

Unreal-Werkzeuge sind dieser Cloud-Sitzung nicht zugänglich. Es wurde weder Windows noch Unreal kompiliert/ausgeführt. PowerShell ist hier ebenfalls nicht vorhanden; die Wrapper sind vorbereitet, aber unter Windows ungetestet. Vier UE-Automationstests, UHT/UBT, Material-/Map-Erzeugung, Bewegung/Kollision, Audio, Licht, Save-Dateien und Packaging bleiben offen. Es wurden keine tatsächlichen Engine-Compilerfehler beobachtet; daher wird auch keine Engine-Fehlerbehebung oder gemessene Performance behauptet.

Die bisherigen Kernsysteme sind weiterhin an den nativen Blockout angebunden. Das belegt Quellcode-Integration, noch keinen bestandenen Spieltest. [LOCAL_UNREAL_ACCESS.md](LOCAL_UNREAL_ACCESS.md) nennt die Aktionen, mit denen der fehlende lokale Nachweis hergestellt wird.
