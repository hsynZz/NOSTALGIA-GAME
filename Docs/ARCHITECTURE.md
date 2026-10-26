# Technische Architektur 0.1

## Verantwortlichkeiten

| System | Implementierung | Verantwortung |
| --- | --- | --- |
| Weltdefinition | `Core/NostalgiaWorldLayout` | Gemeinsame Anker, Bodenabdeckung, Bezirksgrenzen und zulässige Speicherregionen |
| Speicherprüfung | `Core/NostalgiaSnapshot` | Zustand und Position gemeinsam validieren; Kapsel gegen gespeicherte Türstellung prüfen |
| Engine-unabhängiger Zustand | `Public/Core/NostalgiaCore.h`, `Private/Core/NostalgiaCore.cpp` | Quest, Zeit, Schleifenfortschritt, Eventregeln, RNG, Validierung |
| Session / QuestState / TimeOfDay / SaveGame | `UNostalgiaStateSubsystem`, `UNostalgiaSaveGame` | Eine Instanz pro GameInstance; Adaption an UE, validierte Snapshot-Konvertierung und abgesicherte Wiederherstellung, UI-Texte |
| DreamDirector | Core `PlanEvent` / `CommitEvent`, `UNostalgiaEventCatalog` | Deterministische Auswahl, Verfügbarkeit, bestätigte Präsentation; editierbare Data Assets |
| Character | `ANostalgiaCharacter` | Bewegung, Kamera, Input, Sichtstrahl, Übergangsschutz |
| Interaction | `INostalgiaInteraction`, `ANostalgiaQuestObject`, `ANostalgiaDoor` | Einheitliche Prompts/Interaktionen, Questaktionen, Türbewegung |
| WorldLoop | `ANostalgiaWorldLoop` | Lokaler Trigger, sichere Zielplatzierung vor Fortschrittsänderung |
| HouseInterior | `ANostalgiaHouseInterior` | Kuratierte Ein-/Ausgangsanker, relative Blickrichtung |
| AudioDirector | `UNostalgiaAudioDirector` | Mono-3D-Audio, Reichweite, Occlusion, Ende bei Annäherung |
| EntityAI | Core `NostalgiaEncounter`, `ANostalgiaHomecomer` | Dormant → Observing / Approaching → Retreating → Cooldown; Wahrnehmung und begrenzte Bewegung |
| World / Licht | `ANostalgiaBlockout` | Erzeugt Szene, bestimmt Bezirke, taktet Session, spiegelt Zustand visuell |
| Start / HUD | `ANostalgiaGameMode`, `ANostalgiaHUD` | Pawn, PlayerStart, Ziele, Uhr, Hinweise, Diagnostik |

Der Core ist produktiver Code im Unreal-Modul, keine Testnachbildung. Der native Linux-Test kompiliert alle produktiven `.cpp` unter `Private/Core/`, einschließlich des Begegnungsmoduls. Die UE-Adapter sind ohne UHT/UBT nicht kompiliert. Core-Tests belegen daher ausschließlich dessen Verhalten.

Singleplayer, keine RPCs und keine zugesicherte Multiplayer-Unterstützung. Die zustandsändernde Logik liegt getrennt von der Präsentation, damit später ein Autoritätsmodell ergänzt werden kann. Aktuell bewusst einfache Input-Mappings in `DefaultInput.ini`; kein zusätzlicher Enhanced-Input-Asset-Generierungspfad.

## Kausale Escape-Kette

| Wert | Stage | Erlaubter Auslöser zur nächsten Stage |
| --- | --- | --- |
| 0 | Arrival | Foto im Kinderzimmer ansehen |
| 1 | PhotoFound | Foto mit fehlendem Haus im Garten vergleichen |
| 2 | DiscrepancySeen | Erinnerung am Geburtstagstisch wiederherstellen |
| 3 | BirthdayRestored | Straße erneut bis zum STOP-Übergang durchlaufen |
| 4 | RoadUnlocked | Dunkle Laterne in der Nacht untersuchen |
| 5 | DarkLampFound | Spielhaus betreten, langen Flur durchgehen und Endtür benutzen |
| 6 | Complete | Terminalzustand; kein nächstes Kapitel geladen |

Die Erinnerung setzt die Uhr frühestens auf **20:04**, zeigt das fehlende Haus als Platzhalter, ordnet den vierten Stuhl am Geburtstagstisch ein und ändert die nächste Schleifenroute. Die mit J abrufbare schematische Fotoansicht zeigt die passende Anordnung. Die sichtbare Änderung nach einer frühen Schleife ist die zusätzliche Hausnummer `13 1/2`. Keine zufälligen Schlüssel und keine voneinander unabhängigen Sammelaufgaben. Verfrühte Interaktionen verändern den Zustand nicht und liefern Hinweise.

Zeit: festes Datum 17.08.2004, Start 15:00, eine Spielminute pro realer Sekunde, Halt bei 23:59. Dies ist eine beschleunigte Testuhr. Tick-Deltas werden im UE-Adapter auf 0,25 Sekunden begrenzt, um Debug-Pausen nicht als Stunden zu zählen. Nach Abschluss hält der Director die Zeit an. Nächte: ab 20:00 bzw. vor 06:00. Tageslichtwechsel derzeit diskret, kein fertiges atmosphärisches Beleuchtungssystem.

## Dream Director

Bezirke: `0 Bedroom`, `1 Street`, `2 Garden`, `3 Interior`, `4 Finale`. Phasen: `0 Any`, `1 Day`, `2 Night`. Stage-Werte siehe oben. Aktuelle Bewegungsrate und Aufenthaltszeit kommen aus der Figur; Entdeckungen sind über Questfortschritt repräsentiert.

| ID | Ort / Voraussetzung | Limit / Ruhe |
| --- | --- | --- |
| `ES04.Garden.Laughter` | Garten, Arrival bis BirthdayRestored, mindestens 1 s Aufenthalt | Einmal; Audio endet bei <300 cm Nähe oder Verlassen |
| `ES04.Entity.Homecomer` | Straße, ab BirthdayRestored, mindestens 2 s Aufenthalt, Asset-/Sicht-/Bildschirm-/Entfernungsprüfung | Einmal; bis 9 s Beobachtung/Annäherung + maximal 2 s Rückzug |
| `ES04.Interior.Knock` | Hausinnenraum, nachts, ab PhotoFound, mindestens 4 s, Geschwindigkeit ≤200 cm/s | Einmal; mindestens 45 s nach anderem Schreckereignis |

Alle Regeln prüfen Präsentationsverfügbarkeit, sicheren Bereich, bestehende Bedrohung, Phase, Questfenster, Aufenthalt, Geschwindigkeit, Häufigkeit und Cooldown. Bedroom und Finale sind sichere Bereiche. Kritische Ereignisse haben Vorrang vor optionalen; sie werden bei gültigem Kontext zuverlässig gewählt und nicht durch eine Zufallswahrscheinlichkeit verloren. Auch kritische Schreckereignisse halten Ruhezeiten ein. Bei gleichrangigen Kandidaten verwendet der Director gespeichertes xorshift32-RNG mit Startseed 2004. Die Standardregeln brauchen wegen getrennter Orte meist keinen Zufallsentscheid.

`PlanEvent` arbeitet auf einer Zustandskopie. Der UE-Adapter startet genau die ausgewählte Präsentation synchron im selben Tick und bestätigt sie mit `AcknowledgeEvent`. `CommitEvent` prüft den unveränderten Ausgangszustand und schreibt erst dann Historie, RNG und Ruhezeit. Fehlende Assets oder eine nicht sichtbare Figur bleiben aus der Auswahl ausgeschlossen; ein fehlgeschlagener Start verbraucht keinen Event. Doppelte oder durch Zustandsänderung veraltete Bestätigungen werden verworfen. `SelectEvent` bleibt als sofort bestätigende Komfortfunktion für rein logische Aufrufer erhalten; der Runtime-Adapter verwendet sie nicht.

Der separate Encounter-Core wertet Wahrnehmungsdaten aus, ohne UE zu kennen. Der Actor liefert Distanz, Blickrichtung, Sichtstrahl, Bildschirmprojektion und Kollisionsfeedback. Maximal 150 cm Annäherung, mindestens 450 cm Abstand, ein festes Zeitlimit und sichere Bereiche begrenzen die erste Begegnung. Rückzug ist eine kurze Bewegung vom Spieler weg mit Sweep; blockiert er, endet die Darstellung trotzdem. Keine Navigation oder Suche um Ecken. Details und Grenzen stehen in [CHANGES_CLOUD_GAMEPLAY.md](CHANGES_CLOUD_GAMEPLAY.md).

Kataloge werden vollständig validiert: genau drei eindeutige, gültige IDs. Fehlerhafte Assets verändern die aktiven Regeln nicht; validierte C++-Defaults bleiben aktiv, mit Log-Fehler. Konfiguration später in `/Game/Data/DA_EndlessSummerEvents`; neue Event-IDs benötigen eine explizite Code-/Save-Schema-Erweiterung, nicht bloß weitere Arrayzeilen.

## Übergänge und Blockout

Einheiten: Zentimeter. Raumstart `(-1200,0,100)`, Zimmerausgang bei X=-200, Wohnbereich bis zur Haustür X=600. Der Wohnbereich gehört zum sicheren Zuhause. Straße entlang +X, Garten um `(2200,1100,0)`, Haus-10-Eingang bei `(1200,-1130,120)`, größerer Innenraum um `(-1000,3200,0)`. STOP bei X=5700 führt zunächst nach X=950 außerhalb der Haustür, nach Erinnerungsänderung nach X=7200. Finale mit dunkler Laterne X=8500 und Spielhaus X=8900. Dessen 22 m langer Innenflur liegt um `(8000,3000,0)`, Einstieg `(7150,3000,96)`, Rückweg zur Laterne `(8550,450,96)`. Ein Rückportal im Finale führt nach `(5300,0,96)` in die Straße. Alle Zielpositionen liegen innerhalb der bestehenden Save-Grenzen.

Der Straßenübergang behält seitliche Position und Blickrichtung. Erst erfolgreiche kollisionsgeprüfte Platzierung schreibt den Fortschritt. Der Loop prüft während einer blockierten Überlappung mit 10 Hz erneut und beendet seinen Tick nach Verlassen oder erfolgreichem Übergang. Drei Sekunden Übergangsschutz verhindern Trigger-Kaskaden; auch der Core speichert seine Schleifen-Abklingzeit. Bei einem Fall unter Z=-500 Rückkehr zum Zimmer mit erhaltenem Fortschritt. Die Rückkehr setzt die Fallgeschwindigkeit zurück.

Hausinnenräume sind feste separate Geometrie mit expliziten Toren, keine zufälligen Teleports. Es gibt noch kein Level-Streaming, keine nahtlose Portalansicht und keine Übergangsanimation. Physische Straßen-/Türkollision, Orientierung, Raumgefühl und Rückweg müssen in PIE abgenommen werden. Die Wohnhausfassaden sind Blockouts; Haus 10 und das finale Spielhaus besitzen separate Innenräume. Randbegrenzungen verhindern das ebenerdige Umgehen der Route. Statische Geometrie ist ausdrücklich Static; der bewegte Erinnerungsstuhl ist Movable und ohne Kollision, damit sein Zustandswechsel keine Figur einklemmt.

Grafikprofil für den ersten schwachen PC: DX11/SM5, TAA, keine Lumen-GI/-Reflexionen, kein Raytracing und keine virtuellen Schatten. 1280×720, 75 % interne Auflösung und 30-FPS-Limit. Punktlichter ohne eigene Schatten; eine schattenlose Richtungsaufhellung unterstützt den Nachtpfad. Sky-Capture nur beim Aufbau. Keine Performance-Messung vorhanden. Details im [Änderungsprotokoll](CHANGES_LOW_SPEC.md).

## Gemeinsame Weltdefinition

`Core/NostalgiaWorldLayout.h` definiert den Figurenradius, die Standhöhe, Start-/Loop-/Portalziele, die Nachbarschaftsbodenplatte und XY-Grenzen der kuratierten Regionen. Die Runtime baut aus diesen Werten; Speicherprüfung und native Tests verwenden dieselben Definitionen. Der GameMode exportiert die Spawn-Position als reflektierte statische Methode an das Editor-Python-Skript. Das reduziert voneinander abweichende Startpunkte. Editor-Reflection und Python-Aufruf sind noch ungetestet.

Die Nachbarschaft hat durchgehenden Boden von X=-1900 bis 6100 und Y=-2350 bis 2350; frühere Lücken neben/hinter dem Wohnhaus sind im Quellcode geschlossen. Die analytischen Regionen ersetzen keine echte Kollisions- oder Bodenprüfung.

## Speicherung

Ein versionierter `USaveGame` speichert Stage, Minute, vergangene Zeit, RNG-Zustand, Schleifenzähler, Tür-Bitmaske, Abklingzeiten, Eventzähler/-zeitpunkte und Spielerposition/-blick. Die Konvertierung `UNostalgiaSaveGame::FromSnapshot` / `ToSnapshot` verwendet einen engine-unabhängigen `SaveSnapshot` und ändert Ein-/Ausgabedaten erst nach vollständiger Validierung. Geprüft werden Schema/Kapitel, endliche Werte, erreichbare Regionen, Kapselabstände, Fortschrittsbedingungen und plausible Uhr-/Schleifenzustände. Zielpositionen innerhalb des erweiterten Loop-Triggers sind ausgeschlossen. Finale setzt RoadUnlocked voraus, der Spielhausflur DarkLampFound.

Während des Ladens sperrt die Figur Übergänge; der WorldLoop erzeugt dabei keinen Retry. Vorab prüft eine vertikale Kapsel gegen die gespeicherte Türgeometrie. Aktuelle Tür-Kollision wird vorübergehend ausgesetzt, sonstige Weltkollision bleibt für `TeleportTo` aktiv. Auch eine durch Unreal angepasste Zielposition wird erneut geprüft. Erst danach werden Zustand und gespeicherte Türwinkel übernommen. Scope-Guards stellen bei Fehlern die vorherige Spielerposition und Kollision wieder her, bevor Übergänge freigegeben werden. Das ist im Quellcode implementiert, die tatsächlichen UE-Callbacks und der Rollback bleiben ungetestet.

Die Kapsel-/Türprüfung gilt für die generierten nicht skalierten Türen mit den festgelegten Scharnierstellungen. Angepasste spätere Türassets benötigen entsprechende Geometriedaten. Speichern im künftigen Türblatt wird abgelehnt, damit ein Save während einer Türanimation nicht unladbar wird. Ein beliebiger manipuliert gespeicherter Punkt innerhalb einer Region ist damit noch kein nachgewiesen freier Platz mit tragfähigem Boden.

Weltvarianten, Lampen und Türziele werden aus dem wiederhergestellten Zustand abgeleitet. Laufendes Audio und Heimkehrer-Präsentation werden beim Laden beendet; bereits konsumierte Ereignisse werden nicht wiederholt. Augenblicklicher Türwinkel, Audio-Abspielposition und Sichtungsrestzeit werden nicht gespeichert. Beim Laden springt die Tür direkt auf ihren gespeicherten Zielwinkel; sie schwenkt nicht nachträglich durch die Figur. Ein Save während eines Events lädt dessen konsumierten Fortschritt, ohne den kurzen Effekt erneut zu starten. Ein Slot, synchrones Speichern; keine Autosave-/Backup-/Migrationsgarantie über Version 1 hinaus.

## Asset-Erzeugung

`prepare_unreal.py` ist ausschließlich im Editor auszuführen. Die echte Map referenziert native Klassen; die eigentliche Blockout-Geometrie entsteht in `BeginPlay`. Quell-WAVs werden deterministisch unter `Artifacts/Audio/` erzeugt und als SoundWave importiert. Python-/Editor-Plugins sind nur für das Editor-Target aktiviert. Packaged Runtime verwendet ausschließlich die erzeugten Pakete und C++.

`DirectoriesToAlwaysCook` nimmt die dynamisch geladenen Audio-/Data-Pakete mit. Das Silhouettenmaterial ist ebenfalls explizit im Cook-Verzeichnis eingetragen. Die Engine-Grundformen sind harte CDO-Referenzen. Der tatsächliche Cook-/Package-Erfolg bleibt bis zum UAT-Lauf offen.
