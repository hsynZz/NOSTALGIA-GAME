# Technische Architektur 0.1

## Verantwortlichkeiten

| System | Implementierung | Verantwortung |
| --- | --- | --- |
| Engine-unabhängiger Zustand | `Public/Core/NostalgiaCore.h`, `Private/Core/NostalgiaCore.cpp` | Quest, Zeit, Schleifenfortschritt, Eventregeln, RNG, Validierung |
| Session / QuestState / TimeOfDay / SaveGame | `UNostalgiaStateSubsystem`, `UNostalgiaSaveGame` | Eine Instanz pro GameInstance; Adaption an UE, Speichern/Laden, UI-Texte |
| DreamDirector | Core `SelectEvent`, `UNostalgiaEventCatalog` | Deterministische Auswahl; editierbare echte Data Assets |
| Character | `ANostalgiaCharacter` | Bewegung, Kamera, Input, Sichtstrahl, Übergangsschutz |
| Interaction | `INostalgiaInteraction`, `ANostalgiaQuestObject`, `ANostalgiaDoor` | Einheitliche Prompts/Interaktionen, Questaktionen, Türbewegung |
| WorldLoop | `ANostalgiaWorldLoop` | Lokaler Trigger, sichere Zielplatzierung vor Fortschrittsänderung |
| HouseInterior | `ANostalgiaHouseInterior` | Kuratierte Ein-/Ausgangsanker, relative Blickrichtung |
| AudioDirector | `UNostalgiaAudioDirector` | Mono-3D-Audio, Reichweite, Occlusion, Ende bei Annäherung |
| EntityAI | `ANostalgiaHomecomer` | Dormant → Observing → Retreating → Cooldown; erste Sichtung |
| World / Licht | `ANostalgiaBlockout` | Erzeugt Szene, bestimmt Bezirke, taktet Session, spiegelt Zustand visuell |
| Start / HUD | `ANostalgiaGameMode`, `ANostalgiaHUD` | Pawn, PlayerStart, Ziele, Uhr, Hinweise, Diagnostik |

Der Core ist produktiver Code im Unreal-Modul, keine Testnachbildung. Der native Linux-Test kompiliert genau diese `.cpp`. Die UE-Adapter sind ohne UHT/UBT nicht kompiliert. Core-Tests belegen daher ausschließlich dessen Verhalten.

Singleplayer, keine RPCs und keine zugesicherte Multiplayer-Unterstützung. Die zustandsändernde Logik liegt getrennt von der Präsentation, damit später ein Autoritätsmodell ergänzt werden kann. Aktuell bewusst einfache Input-Mappings in `DefaultInput.ini`; kein zusätzlicher Enhanced-Input-Asset-Generierungspfad.

## Kausale Escape-Kette

| Wert | Stage | Erlaubter Auslöser zur nächsten Stage |
| --- | --- | --- |
| 0 | Arrival | Foto im Kinderzimmer ansehen |
| 1 | PhotoFound | Foto mit fehlendem Haus im Garten vergleichen |
| 2 | DiscrepancySeen | Erinnerung am Geburtstagstisch wiederherstellen |
| 3 | BirthdayRestored | Straße erneut bis zum STOP-Übergang durchlaufen |
| 4 | RoadUnlocked | Dunkle Laterne in der Nacht untersuchen |
| 5 | DarkLampFound | Spielhaus benutzen |
| 6 | Complete | Terminalzustand; kein nächstes Kapitel geladen |

Die Erinnerung setzt die Uhr frühestens auf **20:04**, zeigt das fehlende Haus als Platzhalter und ändert die nächste Schleifenroute. Die sichtbare Änderung nach einer frühen Schleife ist die zusätzliche Hausnummer `13 1/2`. Keine zufälligen Schlüssel und keine voneinander unabhängigen Sammelaufgaben. Verfrühte Interaktionen verändern den Zustand nicht und liefern Hinweise.

Zeit: festes Datum 17.08.2004, Start 15:00, eine Spielminute pro realer Sekunde, Halt bei 23:59. Dies ist eine beschleunigte Testuhr. Tick-Deltas werden im UE-Adapter auf 0,25 Sekunden begrenzt, um Debug-Pausen nicht als Stunden zu zählen. Nach Abschluss hält der Director die Zeit an. Nächte: ab 20:00 bzw. vor 06:00. Tageslichtwechsel derzeit diskret, kein fertiges atmosphärisches Beleuchtungssystem.

## Dream Director

Bezirke: `0 Bedroom`, `1 Street`, `2 Garden`, `3 Interior`, `4 Finale`. Phasen: `0 Any`, `1 Day`, `2 Night`. Stage-Werte siehe oben. Aktuelle Bewegungsrate und Aufenthaltszeit kommen aus der Figur; Entdeckungen sind über Questfortschritt repräsentiert.

| ID | Ort / Voraussetzung | Limit / Ruhe |
| --- | --- | --- |
| `ES04.Garden.Laughter` | Garten, Arrival bis BirthdayRestored, mindestens 1 s Aufenthalt | Einmal; Audio endet bei <300 cm Nähe oder Verlassen |
| `ES04.Entity.Homecomer` | Straße, ab BirthdayRestored, mindestens 2 s Aufenthalt | Einmal; 7 s Sichtung + 2 s Rückzug |
| `ES04.Interior.Knock` | Hausinnenraum, nachts, ab PhotoFound, mindestens 4 s, Geschwindigkeit ≤200 cm/s | Einmal; mindestens 45 s nach anderem Schreckereignis |

Alle Regeln prüfen sicheren Bereich, bestehende Bedrohung, Phase, Questfenster, Aufenthalt, Geschwindigkeit, Häufigkeit und Cooldown. Bedroom und Finale sind sichere Bereiche. Kritische Ereignisse haben Vorrang vor optionalen; sie werden bei gültigem Kontext zuverlässig gewählt und nicht durch eine Zufallswahrscheinlichkeit verloren. Auch kritische Schreckereignisse halten Ruhezeiten ein. Bei gleichrangigen Kandidaten verwendet der Director gespeichertes xorshift32-RNG mit Startseed 2004. Die Standardregeln brauchen wegen getrennter Orte meist keinen Zufallsentscheid.

Kataloge werden vollständig validiert: genau drei eindeutige, gültige IDs. Fehlerhafte Assets verändern die aktiven Regeln nicht; validierte C++-Defaults bleiben aktiv, mit Log-Fehler. Konfiguration später in `/Game/Data/DA_EndlessSummerEvents`; neue Event-IDs benötigen eine explizite Code-/Save-Schema-Erweiterung, nicht bloß weitere Arrayzeilen.

## Übergänge und Blockout

Einheiten: Zentimeter. Raumstart `(-1200,0,100)`, Zimmerausgang bei X=-200, Wohnbereich bis zur Haustür X=600. Der Wohnbereich gehört zum sicheren Zuhause. Straße entlang +X, Garten um `(2200,1100,0)`, Haus-10-Eingang bei `(1200,-1130,120)`, größerer Innenraum um `(-1000,3200,0)`. STOP bei X=5700 führt zunächst nach X=950 außerhalb der Haustür, nach Erinnerungsänderung nach X=7200. Finale mit dunkler Laterne X=8500 und Spielhaus X=8900.

Der Straßenübergang behält seitliche Position und Blickrichtung. Erst erfolgreiche kollisionsgeprüfte Platzierung schreibt den Fortschritt. Drei Sekunden Übergangsschutz verhindern Trigger-Kaskaden; auch der Core speichert seine Schleifen-Abklingzeit. Bei einem Fall unter Z=-500 Rückkehr zum Zimmer mit erhaltenem Fortschritt. Die Rückkehr setzt die Fallgeschwindigkeit zurück.

Hausinnenräume sind feste separate Geometrie mit expliziten Toren, keine zufälligen Teleports. Es gibt noch kein Level-Streaming, keine nahtlose Portalansicht und keine Übergangsanimation. Physische Straßen-/Türkollision, Orientierung, Raumgefühl und Rückweg müssen in PIE abgenommen werden. Die Fassade ist ein Blockout; nur Haus 10 besitzt einen Innenraum.

Grafikprofil für den ersten schwachen PC: DX11/SM5, TAA, keine Lumen-GI/-Reflexionen, kein Raytracing und keine virtuellen Schatten. 1280×720, 75 % interne Auflösung und 30-FPS-Limit. Punktlichter ohne eigene Schatten; eine schattenlose Richtungsaufhellung unterstützt den Nachtpfad. Sky-Capture nur beim Aufbau. Keine Performance-Messung vorhanden. Details im [Änderungsprotokoll](CHANGES_LOW_SPEC.md).

## Speicherung

Ein versionierter `USaveGame` speichert Stage, Minute, vergangene Zeit, RNG-Zustand, Schleifenzähler, Tür-Bitmaske, Abklingzeiten, Eventzähler/-zeitpunkte und Spielerposition/-blick. Laden prüft Schema/Kapitel, endliche Werte, zulässige Bereiche, plausible Geschichte und kollisionsgeprüfte Zielposition, bevor der Live-Zustand ersetzt wird. Ungültige Saves liefern eine Meldung und ändern den Questzustand nicht.

Weltvarianten, Lampen und Türziele werden aus dem wiederhergestellten Zustand abgeleitet. Laufendes Audio und Heimkehrer-Präsentation werden beim Laden beendet; bereits konsumierte Ereignisse werden nicht wiederholt. Augenblicklicher Türwinkel, Audio-Abspielposition und Sichtungsrestzeit werden nicht gespeichert. Ein Save während eines Events lädt dessen konsumierten Fortschritt, ohne den kurzen Effekt erneut zu starten. Ein Slot, synchrones Speichern; keine Autosave-/Backup-/Migrationsgarantie über Version 1 hinaus.

## Asset-Erzeugung

`prepare_unreal.py` ist ausschließlich im Editor auszuführen. Die echte Map referenziert native Klassen; die eigentliche Blockout-Geometrie entsteht in `BeginPlay`. Quell-WAVs werden deterministisch unter `Artifacts/Audio/` erzeugt und als SoundWave importiert. Python-/Editor-Plugins sind nur für das Editor-Target aktiviert. Packaged Runtime verwendet ausschließlich die erzeugten Pakete und C++.

`DirectoriesToAlwaysCook` nimmt die dynamisch geladenen Audio-/Data-Pakete mit. Das Silhouettenmaterial ist ebenfalls explizit im Cook-Verzeichnis eingetragen. Die Engine-Grundformen sind harte CDO-Referenzen. Der tatsächliche Cook-/Package-Erfolg bleibt bis zum UAT-Lauf offen.
