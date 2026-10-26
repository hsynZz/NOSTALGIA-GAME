# Assetnachweis für Quality 0.2

Alle hier übernommenen Dateien sind kostenlos. Keine Marketplace-Käufe, keine Übernahme fremder Spielassets, keine unklaren Sketchfab-Downloads. Zugriff am 26.10.2026. Der Lizenznachweis bezieht sich auf die **konkreten Dateien**, nicht auf eine pauschale Lizenz für das gesamte Quellrepository.

Bezugsrepository: [KhronosGroup/glTF-Sample-Assets, Commit 8f492390ddc7a2ae221a1369ef560589cf8c70fb](https://github.com/KhronosGroup/glTF-Sample-Assets/tree/8f492390ddc7a2ae221a1369ef560589cf8c70fb). Die mitgelieferten Original-READMEs dokumentieren unter „Legal“ Urheber und Lizenz. Original-GLBs bleiben bytegleich, alle Dateihashes und direkten Downloadquellen stehen in [sources.lock.json](../../SourceArt/Quality02/sources.lock.json).

| Asset / Quelle | Urheber und Lizenz | Verwendung / Modifikationen |
|---|---|---|
| [SheenChair](../../SourceArt/Quality02/ThirdParty/SheenChair/README.md) | © 2020 Wayfair LLC; Eric Chadwick; **CC0 1.0** | Wohnsessel; auf 76 cm Breite skaliert. Holzmaterial für eigenes Mobiliar/Fußboden wiederverwendet; stuhlspezifische AO für diese Verwendung entfernt. |
| [BoomBox](../../SourceArt/Quality02/ThirdParty/BoomBox/README.md) | Microsoft, 2017; **CC0 1.0** | Tragbares Radio auf dem Schreibtisch; skaliert und platziert. Keine Audiodatei im Modell. |
| [ToyCar](../../SourceArt/Quality02/ThirdParty/ToyCar/README.md) | Guido Odendahl; Eric Chadwick, 2020; **CC0 1.0** | Spielzeugauto; Demonstrationsstoff unter dem Auto entfernt; Kameras entfernt; skaliert. |
| [SheenWoodLeatherSofa](../../SourceArt/Quality02/ThirdParty/SheenWoodLeatherSofa/README.md) | © 2024 Darmstadt Graphics Group GmbH; Eric Chadwick, Verbesserungen: **CC BY 4.0**. Original von Fran Calvente / Poly Haven, 2021: **CC0 1.0**. | Wohnsofa; zusammengeführt, auf 2,25 m Breite skaliert und platziert; Material-/Texturkonvertierung durch Blender. **Die bearbeitete Fassung wird mit CC-BY-Nachweis weitergegeben.** |
| [DiffuseTransmissionPlant](../../SourceArt/Quality02/ThirdParty/DiffuseTransmissionPlant/README.md) | © 2024 Darmstadt Graphics Group GmbH; Eric Chadwick, Material/Animation: **CC BY 4.0**. Original Rico Cilliers / Poly Haven, 2021: **CC0 1.0**. | Zimmerpflanze; Demonstrations-Glühwürmchen, Animationen, Lichter und Kameras entfernt; zusammengeführt, skaliert und platziert. **CC-BY-Nachweis bleibt erhalten.** |

CC0 erlaubt kommerzielle Nutzung und Bearbeitung ohne Attributionspflicht. Wir führen die Urheber trotzdem auf. CC BY 4.0 erlaubt kommerzielle Nutzung, Bearbeitung und Weitergabe bei Namensnennung, Lizenzlink und Kennzeichnung von Änderungen. Die genannten Creditzeilen müssen auch in künftigen Veröffentlichungen mit den bearbeiteten Sofa-/Pflanzenassets erhalten bleiben. Es wird keine Unterstützung durch die Urheber behauptet.

Lizenztexte: [CC0 1.0](../../SourceArt/Quality02/Licenses/CC0-1.0.html), [CC BY 4.0](../../SourceArt/Quality02/Licenses/CC-BY-4.0.html). Offizielle Links: <https://creativecommons.org/publicdomain/zero/1.0/> und <https://creativecommons.org/licenses/by/4.0/>. Die lokalen HTML-Lizenztexte stammen aus dem offiziellen Repository [creativecommons/creativecommons.org](https://github.com/creativecommons/creativecommons.org/tree/main/docroot/legalcode).

## Eigene Arbeiten und Export

Raumhülle, Bett, Tisch, CRT-Gehäuse, Tastatur, Regale/Bücher, Kalender, Tür, Fenster, Vorhänge, Gartenmöbel, Zaun, Blatt-/Astgeometrie und Vista-Fassaden werden aus `Scripts/quality02/build_scene.py` erstellt. Es wurden keine fremden Poster, Markenlogos, Kinderfotos oder Figuren kopiert. Stoff-Basecolor/Normalmaps sind eigene deterministische Webmuster. Die eigenen Arbeiten haben keine gesonderte Open-Source-Freigabe; die genannten Drittasset-Lizenzen bleiben davon unberührt.

Die abgeleiteten GLBs kombinieren eigene Geometrie und die oben genannten Materialien/Modelle. **Die Gesamtdateien werden daher nicht pauschal als CC0 ausgegeben.** Einheiten, Materialslots, Größen, Dreiecke und Änderungen liegen im Szenenmanifest. Nulltangenten an degenerierten UV-Stellen werden im Export durch einen zur Normalen orthogonalen Einheitsvektor ersetzt; gültige Tangenten und Geometrie bleiben erhalten.

Die verwendeten Wayfair-Modelle zeigen die von den Autoren beschriebenen Produkte beziehungsweise Referenzen. Die Assetlizenz ist kein historischer Nachweis, dass jedes Design 2004 angeboten wurde. Die konkrete historische Ausstattung muss vor Art-Abnahme geprüft werden. Produkt-/Referenzfotos aus den verlinkten READMEs wurden **nicht** als Produktionsbilder heruntergeladen.

Poly Haven und ambientCG konnten direkt nicht aufgerufen werden (Cloud-Proxy HTTP 403). Die Pflanze und das Sofa stammen aus der ausdrücklich lizenzierten Khronos-Weiterbearbeitung der Poly-Haven-Originale. Nicht erreichbare Bibliotheken werden nicht als recherchierte Einzelassets oder fertige Downloads ausgegeben.

## Renderwerkzeuge

Blender 4.3.2 ist in der Cloud vorhanden. Optional wurde [Intel Open Image Denoise 2.3.3](https://github.com/RenderKit/oidn/releases/tag/v2.3.3) als freies Renderwerkzeug in `/tmp` verwendet; es wird nicht mit dem Spiel verteilt. Die Pipeline nutzt dessen üblichen Raytracing-Denoiser auf tatsächlich gerenderten linearen EXR-Pixeln. Rohbilder bleiben nachprüfbar. Keine Bildgenerierungsdienste wurden für die Nachweise genutzt.
