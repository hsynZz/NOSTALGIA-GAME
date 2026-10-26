# Nachweise und offene Abnahme · Quality 0.2

Diese Cloud-Prüfungen ersetzen keinen Unreal-Build oder Unreal-Spieltest. Stand: 26.10.2026.

| Prüfung | Tatsächlich ausgeführt | Grenze |
|---|---|---|
| Unreal-Zugriff | Werkzeugkatalog, konfigurierte Verbindung, ausführbare Programme und Prozesse geprüft; keine nutzbare Unreal-Laufzeit/Verbindung gefunden | Kein Zugriff auf lokalen Windows-Editor; keine Aussage, ob UE dort inzwischen funktioniert |
| Native Produktionsteile | 67 C++-Tests mit GCC, AddressSanitizer und UndefinedBehaviorSanitizer | Reiner Core, keine UHT-/UBT-Kompilierung der Unreal-Adapter |
| Repository/Assets | 12 Python-Tests: Engine-Pin, Skriptparser, Quell-/Assetintegrität, echte GLB-Struktur, Tangentenreparatur u. a. | Keine Ausführung der Unreal-Python-API |
| Browser-Regression | 13 vorhandene Node-Tests einschließlich 41 Zustandsvergleichen mit dem realen C++-Core | Browsergrafik/Steuerung wurde in dieser Iteration nicht verändert; kein neuer menschlicher Spieltest |
| glTF-Format | Khronos-Validator über alle neun ausgelieferten GLBs; keine Fehler und keine Warnungen im geprüften Endstand | Renderer können Materialerweiterungen dennoch unterschiedlich darstellen |
| Mesh-Rundlauf und Weg | Alle neun GLBs tatsächlich erneut in Blender importiert; begehbarer Prüfweg mit Boden- und Körperstrahlen, geöffnete Türmeshes eingeschlossen | Konservative diskrete Strahlprüfung, keine vollständige Capsule-Sweep- oder Unreal-Physiksimulation |
| Sichtprüfung | Drei echte Cycles-CPU-Renderings; Licht, Dachgeometrie, Einrichtung und Außenansicht geprüft und nachgebessert | Kein Echtzeit-FPS-Nachweis, kein Unreal-Lumen-/Wolkenbild, keine fotorealistische Endabnahme |

Die **finalen maschinenlesbaren Ergebnisse und Dateihashes** liegen unter [Validation](Validation): `offline-results.json`, `core.log`, `python.log`, `browser-unit-tests.log`, `gltf-validation.json`, `walk-route.json`, `blender-build.json` und `delivery.json`.

## Änderungen aufgrund tatsächlicher Prüfungen

- Der erste Cycles-Lauf scheiterte am im Cloud-Blender fehlenden OpenImageDenoise-Build. Die fertige Pipeline nutzt optional das eigenständige freie OIDN-Werkzeug; rohe Cycles-Bilder bleiben erhalten. Ohne dieses Werkzeug läuft die Quelle weiterhin ohne Entrauschung.
- Die erste Innenansicht war zu dunkel. Fensterverglasung erhielt transparente direkte Schatten, indirektes Tageslicht wurde angepasst und Außen-/Innenkameras bekommen dokumentierte unterschiedliche Belichtung.
- Sichtbare Dachüberschneidungen wurden korrigiert, Dachneigung und Giebel ausgearbeitet. Die Hauswände an Türen/Fenstern verwenden verbundene Flächen statt überlappender Kästen, um dunkle Stoßfugen zu vermeiden.
- Der glTF-Validator fand nach dem Tangentenexport Nullvektoren an degenerierten UV-Stellen. Die Exportkorrektur erhält gültige Tangenten und ersetzt nur undefinierte durch einen zur Normalen orthogonalen Einheitsvektor. Das wird separat gegen Geometrie-/Handedness-Veränderungen und wiederholte Anwendung getestet.
- Der Mesh-Wegtest deckte eine Quaternion-/Euler-Verwechslung beim Einstellen der importierten Prüftüren auf. Der Test und das Asset-Staging setzen den Rotationsmodus jetzt ausdrücklich. Türen werden im Geometrietest tatsächlich offen positioniert.

## Was noch offen ist

1. **Unreal-Import und -Build:** Der ausführbare Importer ist geschrieben, aber nicht in UE 5.8.0 ausgeführt. Engine-API-, Interchange- oder Shaderfehler können erst dort sichtbar werden. Es sind noch keine echten Quality-`.uasset`-/`.umap`-Dateien in der Cloud entstanden.
2. **Darstellung in Unreal:** Achsen, Normalmaps, Sheen, Blattmaterial, Glas, Belichtung, Lumen und volumetrische Wolken müssen real geprüft werden. Die Blender-Bilder garantieren keine pixelgleiche Darstellung.
3. **Spielgefühl:** Bewegung, Kamerakomfort, Tür-Kollisionsbogen und HUD sind im nativen Adapter implementiert, aber noch nicht in PIE beurteilt. Animierte Hände, greifbare Gegenstände, eine hochwertige Fotoinspektion und Radiointeraktion fehlen.
4. **Audio:** Eine natürliche, freigegebene Kinderlachen-Aufnahme fehlt. Die neue Szene verwendet deshalb kein synthetisches Ersatzlachen. Natürliches Raumklangbild, Schritte und Umweltaufnahmen fehlen; keine Hörabnahme wurde behauptet.
5. **Art-Qualität:** Der Innenraum ist eine konkrete erste Fassung. Garten, Bodenübergänge, Baum-/Grasqualität, Fassade, Dachmaterial, Alterung, kuratierte persönliche Details und die historische Stimmigkeit jedes Fremdmodells müssen weiter ausgearbeitet werden. Diese Objekte gelten nicht als endgültig abgenommen.
6. **Auslieferung:** Kein Quality-0.2-Windows-Build, keine FPS-/GPU-Speichermessung, kein eigener Quality-Spielstand. Die vorhandene 0.1-Escape-Kette wird nicht als bereits in der Quality-Karte spielbar ausgegeben. Die erste Szene hat einen Sicherheitsrücksprung bei einem Sturz, aber noch keine fertig gestaltete Weltbegrenzung für freie Langzeiterkundung.

Der überprüfbare Lieferstand ist eine eingerichtete 3D-Quellszene mit echten lizenzierten Assetdateien, Materialdaten, überprüften Exporten, nativen Adaptern und konkretem Unreal-Aufbauprozess. **Der hochwertige, in Unreal spielbare Vertical Slice ist noch nicht fertig.**
