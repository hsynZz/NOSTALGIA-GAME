# Scope und priorisiertes Backlog

**Status:** technischer Quellprototyp 0.1; keine bestätigte Spielbarkeit. Kreative Grundlage ist der unverändert archivierte Projektbrief. Keine weiteren Kapitelinhalte oder neue Lore implementiert.

Fortsetzung 23.10.2026: Nutzer bestätigt UE 5.8.0 auf Windows. Cloud-Zugriff auf den lokalen Editor bleibt blockiert. Sparsames Grafikprofil, Wohnbereich, zweite Tür und Fenster sind im bestehenden Code ergänzt; siehe [CHANGES_LOW_SPEC.md](CHANGES_LOW_SPEC.md) und [lokale Anleitung](LOCAL_UNREAL_ACCESS.md).

## Implementiert, Engine-Integration noch ungetestet

| ID | Lieferumfang | Abnahmekriterium |
| --- | --- | --- |
| P01 | UE-Projekt, Modul, Game-/Editor-Targets, Konfiguration, LFS | UHT und UBT bauen mit festgelegter 5.8.0; kein manueller Pseudocode-Ersatz |
| P02 | FP-Bewegung und Interaktion | Bewegung, Sprint, Sprung, Ducken, Sichtstrahl und verdeckte Objekte korrekt |
| P03 | Blockout, Zimmer-Tür, Haus-10-Innenraum | Zimmer verlassen, sechs Fassaden sehen, Innenraum betreten und zurückkehren |
| P04 | Uhr und Lichtzustand | Startdatum sichtbar, Tab zeigt Uhr, Erinnerungsänderung aktiviert Nacht/Laternen |
| P05 | Lokales Gartenereignis und Audioadapter | Ereignis nur im Garten, endet bei Nähe, kein Wiederholungsspam; Platzhalter klar erkennbar |
| P06 | WorldLoop und visuelle Änderung | Frühe Schleife kehrt orientiert zurück; nach Erinnerung neue Route; kein Trigger-Pingpong |
| P07 | Heimkehrer-Sichtung | Kontrollierte, kurze Silhouette; sichere Räume und Ruhezeit wirken |
| P08 | Mini-Quest und Abschluss | A/B/C-Reihenfolge erkennbar; H liefert Hinweise; Spielhaus setzt Complete |
| P09 | Save/Load | Wesentlicher Zustand und Position erhalten; korrupte/fremde Saves abgelehnt |
| P10 | Debug, native Tests, UE-Tests, CI | Offline-Prüfungen laufen; vier UE-Tests nach Engine-Setup, anschließend manuelle Abnahme |

Die reine Quest-/Director-/Zeitlogik aus P04–P10 ist bereits mit real kompilierten nativen Tests geprüft. Alle Aussagen über Bewegung, Rendering, Audioausgabe, Physik, Map-Import, UE-Speicherung und Windows-Auslieferung benötigen weiterhin Unreal.

## P0 — nächste drei Aufgaben

1. **Engine und Toolchain verifizieren:** UE 5.8.0 bereitstellen, `-Action All` ausführen, UHT-/UBT-/Python-/Automationsergebnisse archivieren. Fertig, wenn der Editor-Build, die Paket-Erzeugung und alle vier UE-Tests erfolgreich sind. Gegebenenfalls API-/Buildfehler in einer eigenen Iteration beheben.
2. **Ersten PIE-Lauf abnehmen:** [Testplan](TEST_PLAN.md) vollständig durchgehen, insbesondere Türen, Übergänge, Save/Load und Audio. Fertig mit tatsächlicher Aufnahme/Screenshots, protokollierten Ergebnissen und behobenen blockierenden Fehlern. Danach generierte `.umap`-/`.uasset`-Dateien über LFS committen.
3. **Windows-Prototyp paketieren:** UAT-Cook/Build ausführen und das Paket außerhalb des Editors testen. Fertig, wenn eine Person ohne Editorwissen den vollständigen Mini-Ablauf durchführen kann und Ergebnis/Hardware/Buildkennung dokumentiert sind.

## P1 — nach erfolgreicher technischer Abnahme

- Platzhalter durch eigene/lizenzierte Foto-, Zimmer-, Geburtstags-, Garten- und Audioassets ersetzen; Lizenzregister je konkretem Asset vervollständigen.
- Tatsächliches Kinderlachen, Umgebungsvarianten, Schritte und sparsame Atmung; räumliche Mischung und Occlusion hören.
- Tür-/Handanimationen, Kamera- und Übergangsgefühl, Controller-/Enhanced-Input-Anbindung, nutzbare Menüs.
- Vorhandenes Blockout-Fenster und Außenblick visuell abnehmen; glaubwürdiger Garten, bessere Silhouette; Bildsprache ohne permanente VHS-Verzerrung.
- Zeit-/Lichtinterpolation und Performance-Messung mit realer GPU; Cookgröße und Speicherverbrauch messen.

## P2 — späterer Kapitelumfang

- Kriechen, Klettern, volle Inspektionsmechanik und passende Animationen. Für MVP0 sind zunächst Gehen/Sprint/Sprung/Ducken implementiert.
- Vollständige Erinnerungsinszenierung: Fotoabweichung derzeit textlich, Erinnerung per Interaktion statt ausgearbeitetem Puzzle.
- Unmöglich langer Spielhaus-Übergang: 0.1 endet mit nachvollziehbarem Abschluss, lädt noch kein zweites Kapitel.
- Streaming mehrerer kuratierter Interiors, Ausbau Richtung 25–35 Häuser erst nach Performance- und Qualitätsnachweis.
- Heimkehrer mit Wahrnehmung, Navigation, Suche, Verfolgung, Fluchtmöglichkeiten und Recovery bei blockierten Wegen. 0.1 hat nur Sichtung/Rückzug ohne Schaden.
- Optional seltene Varianten/Easter Eggs; später weitere Kapitel. Multiplayer, Steam und PlayStation sind keine zugesicherten 0.1-Funktionen.

## Bewusste Vereinfachungen gegenüber dem Projektbrief

Grauer Code-Blockout statt hochwertiger Produktion; synthetisches Audio statt Stimmen; Textobjekt statt fertigem Geburtstagsfoto; feste Innenraumanker statt Streaming; keine Hände-/Kletteranimation; Abschlussbildschirm statt echter Weiterreise. Die A/B/C-Ursache-Wirkung-Kette ist bereits als kleine technische Version implementiert, die vollständige Kapitelinszenierung bleibt offen. Diese Einschränkungen sind kein spielbarer MVP-Nachweis.
