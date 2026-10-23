# Unreal-Abnahme — noch nicht durchgeführt

Pro Test Ergebnis PASS/FAIL/BLOCKED, Engine-Version einschließlich Changelist, Commit, Windows-/GPU-Daten, Datum und Beleg protokollieren. Nicht ausgeführte Schritte bleiben UNRUN. Ein fehlerfreier NullRHI-Test ist kein sichtbarer Spieltest.

## Automatisiert in UE

`Scripts/unreal.ps1 -EngineRoot <UE_5.8-Verzeichnis> -Action All`:

| Test | Erwartung |
| --- | --- |
| `Nostalgia.Core.CausalEscapeChain` | Korrekte Reihenfolge bis Complete, frühes Verlassen abgelehnt |
| `Nostalgia.Data.EventCatalogValidation` | C++-Defaults gültig; doppelte/unvollständige IDs abgelehnt |
| `Nostalgia.Save.UnrealSerialization` | Tatsächlicher `USaveGame`-Speicher-Roundtrip erhält wichtige Felder |
| `Nostalgia.Assets.PreparedPackages` | Echte Map, Katalog, Material und SoundWaves laden; Audio hat Dauer |

Export unter `Artifacts/Unreal/Automation_<Zeit>/index.json`. Der Runner fordert alle vier Namen mit `Success`; alte Ergebnisdateien und Null-Test-Läufe gelten nicht. Diese Tests prüfen noch keine vollständige Spielsitzung und keinen Datenträger-Save-Roundtrip.

## Manuell: erste Szene und Bedienung

1. `EndlessSummer` öffnen und Play starten. Erwartet: Start im Zimmer, Kamera auf Augenhöhe, erkennbare Geometrie, HUD `Endless Summer — 2004`, Uhr `17.08.2004`. Kein Spawn unter dem Boden. Screenshot aufnehmen.
2. WASD, Maus, Sprint, Sprung und Strg-Ducken prüfen. Keine Wanddurchdringung, kein Feststecken unter Decken; Aufrichten mit freiem Raum. Kein permanenter Kamerashake.
3. Foto auf dem Tisch mit E ansehen. Ziel wechselt genau einmal. E ohne Treffer oder durch eine Wand darf kein Objekt auslösen.
4. Tür ansehen und mit E öffnen. Blatt schwenkt am Scharnier, Öffnung lässt Figur durch. Schließen neben und im Türbereich testen: Figur wird nicht eingeklemmt; Bewegung setzt nach Freigabe fort. Save/Load bei offener/geschlossener Tür.
5. Zimmer verlassen, Vorgarten und sechs Hausfassaden prüfen. Keine fehlenden Materialien oder massiven Belichtungsfehler. Nacht später erneut prüfen; Finale muss trotz dunkler Laterne navigierbar bleiben.
6. Haus 10 über E betreten, längeren Innenraum ansehen und zum Ausgang zurückgehen. E bringt zur Straße zurück; Blickrichtung und Eingangsbezug bleiben verständlich. Mehrfach aus beiden Richtungen testen.

## Manuell: Ereignisse und Quest

7. Vor Geburtstag in den Garten gehen. Nach mindestens 1 s wird `ES04.Garden.Laughter` protokolliert; synthetisches Testsignal räumlich hörbar. Quelle bei `(2400,1250,110)` annähern: unter 300 cm ausblenden. Entfernen/Wiedereintritt startet denselben Event nicht neu. Wanddämpfung separat hören.
8. Ohne erfüllte Voraussetzungen die Tisch-/Laternen-/Spielhausobjekte benutzen: hilfreicher Hinweis, keine Abkürzung der Quest. H zeigt aktuellen Hinweis. Fotoobjekt und Vergleichsobjekt sind derzeit beschriftete Platzhalter.
9. Frühe Straßenrunde zum STOP: Rückkehr auf vertraute Straße, neue Hausnummer sichtbar, Blickrichtung erhalten. Sofortiges Hin-/Zurücklaufen darf keine Teleport-Kaskade auslösen. Danach weiter normal erkunden können.
10. Foto vergleichen und Geburtstagstisch benutzen. Uhr springt mindestens auf 20:04, vorhandene Laternen leuchten, Haus erscheint an der fehlenden Stelle. Uhr über Tab schließen/öffnen. Kein Rücksprung beim zweiten Interagieren.
11. Straße nach Erinnerungsänderung erneut entlanggehen. Erste Heimkehrer-Sichtung: kurze Silhouette am kuratierten Ort, verschwindet wieder, keine Dauerverfolgung. Sicherheit im Kinderzimmer prüfen. WindowKnock im Innenraum nur nachts, bei langsamer Bewegung und mit mindestens 45 s Abstand zu anderem Schreckmoment. Debug-Ereignis-ID und Sichtbarkeit müssen zeitlich zusammenpassen.
12. STOP öffnet jetzt die neue Route. Dunkle Laterne bleibt aus, untersuchen → Spielhaus benutzen → klarer Complete-Zustand. Weitere Kapitel werden nicht behauptet. Uhr/Director stoppen. Alt+F4 beendet das Windows-Paket.

## Manuell: Persistenz und Recovery

13. F5 nach Foto, nach früher Schleife und nach BirthdayRestored prüfen. Einige Schritte weiterspielen, F9: Zeit, Quest, offene Tür, Position, Schleifenzähler und Eventhistorie stimmen; konsumiertes Gartenereignis startet nicht neu. Anwendung schließen und neu starten, dann F9: derselbe Zustand. Save/Load auch im Innenraum und Finale testen.
14. Laden bei fehlendem Save, mit inkompatibler Version oder ungültigem Zustand (separate Testkopie, Original sichern) wird ohne Fortschrittsverlust abgelehnt. Speichern in Luft/geduckt liefert Hinweis. Bei blockierter Ladeposition keine unkontrollierte Platzierung. Konsistenz nach Ablehnung prüfen.
15. Save während Sichtung/Audio, danach laden: Effekt endet, Event bleibt konsumiert; das ist dokumentiertes Verhalten. Director-Ruhezeit und Loop-Abklingzeit dürfen nicht durch Laden umgangen werden.
16. In kontrolliertem Editor-Test unter den Boden geraten: Sicherheitsrückkehr ins Zimmer, Bewegung wieder möglich, Fortschritt erhalten. Insel-/Randbereiche und Trigger von mehreren Winkeln auf Softlocks prüfen.

## Manuell: Windows-Paket und Kosten

17. `-Action Package`, danach vollständiges Buildverzeichnis ohne offenen Editor starten. Schritte 1–16 in komprimierter Form wiederholen. Fehlende gekochte Assets, Editor-Abhängigkeiten und Savepfad prüfen. Erst dieser reale Ablauf kann die Aussage „spielbarer Windows-Prototyp“ rechtfertigen.
18. `stat unit`, `stat gpu`, `stat memory` in Development messen, besonders Nacht/Lichter und später größere Interiors. Auflösung, Hardware und Messwerte speichern. Aktuell gibt es keine FPS-/Speicher-Garantie und kein festgelegtes Performancebudget.
