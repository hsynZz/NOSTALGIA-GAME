# Unreal-Abnahme — noch nicht durchgeführt

Für eine später verfügbare UE-5.8.0-Maschine vorbereitet; aktuell kein Auftrag zu lokaler Installation oder Fehlersuche.

Pro Test Ergebnis PASS/FAIL/BLOCKED, Engine-Version einschließlich Changelist, Commit, Windows-/GPU-Daten, Datum und Beleg protokollieren. Nicht ausgeführte Schritte bleiben UNRUN. Ein fehlerfreier NullRHI-Test ist kein sichtbarer Spieltest.

## Automatisiert in UE

`Scripts/unreal.ps1 -EngineRoot <UE_5.8-Verzeichnis> -Action All`:

| Test | Erwartung |
| --- | --- |
| `Nostalgia.Core.CausalEscapeChain` | Korrekte Reihenfolge bis Complete, frühes Verlassen abgelehnt |
| `Nostalgia.Data.EventCatalogValidation` | C++-Defaults gültig; doppelte/unvollständige IDs abgelehnt |
| `Nostalgia.Save.UnrealSerialization` | Validierter Snapshot über tatsächlichen USaveGame-Roundtrip, vollständige Historie; korrupte Daten und inkompatibles Schema/Kapitel abgelehnt |
| `Nostalgia.Assets.PreparedPackages` | Echte Map, Katalog, Material und SoundWaves laden; Audio hat Dauer |

Export unter `Artifacts/Unreal/Automation_<Zeit>/index.json`. Der Runner fordert alle vier Namen mit `Success`; alte Ergebnisdateien und Null-Test-Läufe gelten nicht. Diese Tests prüfen noch keine vollständige Spielsitzung und keinen Datenträger-Save-Roundtrip.

## Manuell: erste Szene und Bedienung

1. `EndlessSummer` öffnen und Play starten. Erwartet: Start im Zimmer, Kamera auf Augenhöhe, erkennbare Geometrie, HUD `Endless Summer — 2004`, Uhr `17.08.2004`. Kein Spawn unter dem Boden. Screenshot aufnehmen.
2. WASD, Maus, Sprint, Sprung und Strg-Ducken prüfen. Keine Wanddurchdringung, kein Feststecken unter Decken; Aufrichten mit freiem Raum. Kein permanenter Kamerashake.
3. Foto auf dem Tisch mit E ansehen. Schematische Ansicht mit Haus und vier Stühlen öffnet sich; J schließt/öffnet sie. Ziel wechselt genau einmal. Solange die Ansicht offen ist, keine WASD-/Mausbewegung oder E-Weltinteraktion und keine neuen Ereignisstarts; Uhr läuft weiter. E ohne Treffer oder durch eine Wand darf kein Objekt auslösen.
4. Zimmertür mit E öffnen, durch den Wohnbereich gehen, Haustür mit E öffnen. Beide Blätter schwenken am Scharnier, Öffnungen lassen Figur durch. Schließen neben und im Türbereich testen: Figur wird nicht eingeklemmt; Bewegung setzt nach Freigabe fort. Save/Load mit beiden Türen unabhängig offen/geschlossen. Der Wohnbereich bleibt im Debug ein sicherer Bezirk.
5. Fenster im Kinderzimmer prüfen: Außenboden sichtbar, Figur kann nicht durch die Öffnung entkommen. Haus verlassen, Vorgarten und sechs Hausfassaden prüfen. Keine fehlenden Materialien oder massiven Belichtungsfehler. Nacht später erneut prüfen; Finale muss trotz dunkler Laterne ohne Lumen navigierbar bleiben.
6. Haus 10 über E betreten, längeren Innenraum mit wiederholten Durchgängen bis zum Ende begehen und zum Ausgang zurückgehen. E bringt zur Straße zurück; Blickrichtung und Eingangsbezug bleiben verständlich. Mehrfach aus beiden Richtungen testen.

## Manuell: Ereignisse und Quest

7. Vor Geburtstag in den Garten gehen. Bei 300–1100 cm Quellentfernung und mindestens 1 s Aufenthalt wird `ES04.Garden.Laughter` nach erfolgreichem Audiostart protokolliert; synthetisches Testsignal räumlich hörbar. Bei einem Start direkt innerhalb des 300-cm-Radius zunächst kein Verbrauch; entfernen und erneut prüfen. Quelle bei `(2400,1250,110)` annähern: unter 300 cm ausblenden. Entfernen/Wiedereintritt startet denselben Event nicht neu. Wanddämpfung separat hören.
8. Ohne erfüllte Voraussetzungen die Tisch-/Laternen-/Spielhausobjekte benutzen: hilfreicher Hinweis, keine Abkürzung der Quest. H zeigt aktuellen Hinweis. Das Foto zeigt eine schematische Ansicht; das Vergleichsobjekt bleibt ein beschrifteter Platzhalter. Das Spielhausportal ist vor DarkLampFound gesperrt.
9. Frühe Straßenrunde zum STOP: Rückkehr auf vertraute Straße bei X=950 außerhalb der Haustür, neue Hausnummer sichtbar, Blickrichtung erhalten. Kein Spawn im neuen Wohnbereich oder auf der Tür. Sofortiges Hin-/Zurücklaufen darf keine Teleport-Kaskade auslösen. Trigger innerhalb der Drei-Sekunden-Sperre betreten und dort stehenbleiben: nach Ablauf wird der Übergang nachgeholt. Verlassen des Triggers stoppt den Retry. Gartenränder und Straßenende auf Umgehung oder Einklemmen prüfen. Danach weiter normal erkunden können.
10. Foto vergleichen und Geburtstagstisch benutzen. Uhr springt mindestens auf 20:04, vorhandene Laternen leuchten, Haus erscheint an der fehlenden Stelle, der vierte Stuhl steht wieder am Tisch. Vorher/nachher mit J vergleichen. Der bewegte Stuhl klemmt eine danebenstehende Figur nicht ein. Uhr über Tab schließen/öffnen. Kein Rücksprung beim zweiten Interagieren.
11. Straße nach Erinnerungsänderung erneut entlanggehen. Heimkehrer bei `(3500,850,192)`: zuerst abgewandt oder hinter Hindernis bleiben, kein Ereignisverbrauch. Dann aus 600–2200 cm Entfernung bei freier Sicht im Bild ansehen: einmaliger Start. Erste 2 s still, beim Wegsehen höchstens 150 cm Annäherung und mindestens 450 cm Restabstand. Wieder hinsehen oder Sichtlinie verdecken stoppt Annäherung. Nach maximal etwa 11 s unsichtbar. Engen Weg/Hindernis testen: kein Durchgleiten, kein endlos aktiver Zustand. Kinderzimmer und Finale beenden aktive Sichtung sofort. WindowKnock im Innenraum nur nachts, bei langsamer Bewegung und mit mindestens 45 s Abstand zu anderem Schreckmoment. Debug-Ereignis-ID und Sichtbarkeit müssen zeitlich zusammenpassen.
12. STOP öffnet jetzt die neue Route. Dunkle Laterne bleibt aus. Vor Untersuchung ist das Spielhaus gesperrt. Danach E am Spielhaus → 22 m langer Flur → E an der Endtür → Complete. Vor Abschluss den Flur-Rückweg zur Laterne und das Rückportal zur Nachbarschaft testen; Fortschritt darf nicht zurückfallen. Nochmals STOP und Spielhaus durchlaufen. Weitere Kapitel werden nicht behauptet. Uhr/Director stoppen. Alt+F4 beendet das Windows-Paket.

## Manuell: Persistenz und Recovery

13. F5 nach Foto, nach früher Schleife und nach BirthdayRestored prüfen. Einige Schritte weiterspielen, F9: Zeit, Quest, offene Tür, Position, Schleifenzähler und Eventhistorie stimmen; konsumiertes Gartenereignis startet nicht neu. Anwendung schließen und neu starten, dann F9: derselbe Zustand. Save/Load auch im Innenraum, Finale und Spielhausflur testen. Fotofenster schließt sich beim erfolgreichen Laden; Stuhl/Hausdarstellung folgt dem geladenen Zustand.
14. Laden bei fehlendem Save, mit inkompatibler Version oder ungültigem Zustand (separate Testkopie, Original sichern) wird ohne Fortschrittsverlust abgelehnt. Speichern in Luft/geduckt liefert Hinweis. Bei blockierter Ladeposition keine unkontrollierte Platzierung. Konsistenz nach Ablehnung prüfen.
15. Save während Sichtung/Audio, danach laden: Effekt endet, Event bleibt konsumiert; das ist dokumentiertes Verhalten. Director-Ruhezeit und Loop-Abklingzeit dürfen nicht durch Laden umgangen werden.
16. In kontrolliertem Editor-Test unter den Boden geraten: Sicherheitsrückkehr ins Zimmer, Bewegung wieder möglich, Fortschritt erhalten. Insel-/Randbereiche und Trigger von mehreren Winkeln auf Softlocks prüfen.

## Manuell: Windows-Paket und Kosten

17. `-Action Package`, danach vollständiges Buildverzeichnis ohne offenen Editor starten. Schritte 1–16 in komprimierter Form wiederholen. Fehlende gekochte Assets, Editor-Abhängigkeiten und Savepfad prüfen. Erst dieser reale Ablauf kann die Aussage „spielbarer Windows-Prototyp“ rechtfertigen.
18. `stat unit`, `stat gpu`, `stat memory` in Development messen, besonders Nacht/Lichter und später größere Interiors. Auflösung, Hardware und Messwerte speichern. Aktuell gibt es keine FPS-/Speicher-Garantie und kein festgelegtes Performancebudget.

## Manuell: nicht verfügbare Präsentationen

19. In einer separaten Testkopie ohne vorbereitete Audio-/Materialpakete starten: Log-Warnungen, F3-Bereitschaft falsch, keine konsumierten Garten-/Heimkehrer-Ereignisse. Quest bleibt bedienbar. Nach korrektem Prepare in neuer Sitzung müssen entsprechende Ereignisse starten. Fehlender Audiotreiber darf keinen Erfolg vortäuschen. Komponentenfreigabe beim Levelende/Laden und nach normalem Clipende prüfen. Tatsächliche Ausgabe zusätzlich hören; `IsPlaying` allein beweist sie nicht.

## Ergänzte Abnahme: Weltdefinition und Wiederherstellung (26.10.2026)

20. `prepare_unreal.py` in UE ausführen: neue reflektierte Spawn-Methode wird gefunden; PlayerStart stimmt mit GameMode-Fallback und Sicherheitsrückkehr überein. Eine vorhandene bewusst bearbeitete Map wird dabei weiterhin nicht überschrieben.
21. Hinter/seitlich am eigenen Wohnhaus gehen, besonders um `(-1200,1200)` und `(-1000,-1500)`: durchgehender Boden, keine Fall-Rückkehr. Ecken der begrenzten Nachbarschaft sowie Höhenunterschied Straße/Rasen testen.
22. F5 direkt in einem offenen Türdurchgang; anschließend weggehen und Tür schließen, F9: gespeicherte offene Stellung und Position werden gemeinsam wiederhergestellt. Gegenprobe mit geschlossen gespeicherter Tür bei aktueller offener Tür. Während der Türbewegung im künftigen Türblatt darf F5 keinen unladbaren Spielstand überschreiben.
23. Getrennte Test-Saves manipulieren: Punkt zwischen Innenräumen, früh im Finale/Spielhausflur, im Loop oder mit unmöglichem Uhr-/Schleifenzustand. Laden muss vor Zustandsänderung ablehnen. Gültige Version-1-Saves mit korrekter Geschichte bleiben lesbar.
24. Ziel mit temporärem Hindernis blockieren: F9 scheitert, ursprüngliche Position, Türen, Fortschritt und Bedienung bleiben erhalten; erneuter Versuch nach Entfernen des Hindernisses gelingt. Keine durch Laden oder Rollback zusätzlich gezählte Straßenschleife. Eingaben in/kurz nach dem Ladeaufruf prüfen.
25. Foto während Sprint betrachten: Figur driftet auf festem Boden nicht weiter; Sprintstart, Sprung und Ducken reagieren erst nach Schließen wieder. Loslassen von Tasten bleibt wirksam. Betrachtung im Sprung und erfolgreiches F9 prüfen; keine aufgehobene Schwerkraft oder festhängende Eingabe.
