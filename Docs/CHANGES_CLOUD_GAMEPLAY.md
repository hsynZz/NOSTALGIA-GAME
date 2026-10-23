# Cloud-Fortsetzung: Ereignisse und räumlicher Ablauf

Stand: 23.10.2026, auf bestehendem Commit `cc77949121443aa03ef9d3d95145f8f7061bc511`. Ziel bleibt UE **5.8.0**, Prototype **0.1**, Endless Summer **2004**. Kein neues Kapitel, keine neue Lore und keine Sammelaufgaben. Bestehende Systeme wurden erweitert.

## Geliefert im Quellcode

| Bereich | Konkrete Änderung |
| --- | --- |
| Dream Director | `PlanEvent` wählt ohne Zustandsänderung; `CommitEvent` schreibt Historie, Ruhezeit und RNG erst nach Startbestätigung. Nicht verfügbare Präsentationen nehmen nicht an der Auswahl teil. Veraltete oder doppelte Bestätigungen werden abgelehnt. |
| Heimkehrer | Separater produktiver Core `NostalgiaEncounter`: Dormant → Observing / Approaching → Retreating → Cooldown. Start nur außerhalb sicherer Zonen, bei 600–2200 cm Abstand, freier Sicht und projiziertem Punkt innerhalb des Bildschirms. Material/Mesh müssen vorhanden sein. |
| Begegnungsgrenzen | Zunächst 2 s still; danach beim Wegsehen höchstens insgesamt 150 cm Annäherung mit 60 cm/s und mindestens 450 cm Abstand. Ab 9 s Rückzug für maximal 2 s. Hindernisse leiten ebenfalls den begrenzten Rückzug ein. Sichere Bereiche beenden die Sichtung sofort. Kein Schaden, keine vollständige Verfolgungs-KI. |
| UE-Anbindung | Der Blockout aktualisiert Wahrnehmung und Begegnung vor der Director-Auswahl. Bewegung verwendet Capsule-Sweeps; der Zustand behandelt blockierte Bewegung. Nur ein Orchestrator statt voneinander abhängiger Actor-Ticks. F3 zeigt Phase und Präsentationsbereitschaft. |
| Ortsgebundenes Audio | Quelle muss vorhanden, innerhalb der Reichweite und frei von einem bereits laufenden eigenen Clip sein. Garten startet nicht innerhalb des 300-cm-Abbruchradius. FadeOut wird einmal ausgelöst; beendete Komponenten werden freigegeben. Klopfen blendet beim Verlassen des Innenraums aus. Laden und EndPlay bereinigen Audio. |
| Foto und Geburtstag | E am Foto öffnet eine originale schematische Ansicht mit Haus und vier Stühlen; J öffnet/schließt sie später erneut. Garten zeigt drei Stühle am Tisch und einen abseits. Erinnerung stellt den vierten Stuhl dazu und zeigt das fehlende Haus. Keine externen Bildassets. |
| Räumliche Anomalien | Wiederholte Durchgänge verdeutlichen Haus 10 von innen. Das kleine Spielhaus führt erst nach Untersuchung der dunklen Laterne in einen 22 m langen Flur. Die Endtür bestätigt den bisherigen Abschlusszustand. Ein Rückweg führt zur Laterne, ein weiterer aus dem Finale zurück in die Nachbarschaft. |
| Übergänge | Eine während der Übergangssperre betretene Straßenschleife versucht den Übergang erneut, solange die Figur im Trigger bleibt. Randbegrenzungen erschweren das Umgehen des kuratierten Straßenendes. Blockierte Hausportale liefern einen Hinweis. |
| Prüfwerkzeug | `validate.py` baut alle produktiven Core-`.cpp` gemeinsam. Berichte enthalten Basiscommit, SHA-256 der Eingabedateien sowie native und Python-Logs. |

Die bisherige Fluchtfolge und ihre gespeicherten Stage-Werte bleiben erhalten: Foto → Abweichung → Geburtstag → Straßenschleife → dunkle Laterne → Spielhausabschluss. Der letzte Schritt findet jetzt an der Endtür des Spielhausflurs statt. Save-Schema bleibt Version 1; zusätzliche Darstellung wird aus dem Questzustand abgeleitet. Die Fotoansicht wird beim Laden geschlossen. Bereits bestätigte Sichtungen und Audioereignisse bleiben konsumiert; deren laufende Präsentation wird nicht gespeichert.

Die Fotoansicht blockiert WASD, Kameradrehen und Weltinteraktion; die Uhr läuft weiter. Während sie geöffnet ist, startet kein neues Director-Ereignis. Sie ist eine einfache Hinweisoberfläche, noch kein frei drehbares 3D-Inspektionssystem. Das wiedergekehrte Haus ist weiterhin eine sichtbare, nicht begehbare Erinnerungsdarstellung.

## Wirklich geprüft

`python3 Scripts/validate.py --sanitize`: **47/47 C++-Tests**, **9/9 Python-Tests**, keine gemeldeten AddressSanitizer-/UndefinedBehaviorSanitizer-Befunde. **17 zusätzliche C++-Tests** prüfen fehlgeschlagene und veraltete Bestätigungen, stabile Zufallsauswahl bei erneutem Versuch, Verfügbarkeit, Wahrnehmungsbedingungen, Annäherungsgrenzen, Abbruch, sichere Bereiche und die zusammenhängende Quest nach Ereignisfehlern und Zustandswiederherstellung.

Belege: [JSON mit Quellcode-Hashes](Validation/offline-cloud-gameplay-2026-10-23.json), [C++-Log](Validation/core-cloud-gameplay-2026-10-23.log), [Python-Log](Validation/python-cloud-gameplay-2026-10-23.log). Das JSON nennt den Basiscommit vor dieser Änderung; die Hashes identifizieren die tatsächlich geprüften Dateien einschließlich der neuen Core-Dateien.

## Noch nicht geprüft

In dieser Cloud-Sitzung sind keine Live-Unreal-Werkzeuge verbunden; lokale Projektdateien auf dem Windows-PC sind nicht zugänglich. Unreal-Binaries, UHT/UBT und eine Windows-Toolchain stehen hier nicht zur Verfügung. Es gab keine neuen lokalen Installationsversuche.

Die Tests kompilieren ausschließlich den Engine-unabhängigen produktiven Core. Sie liefern keine Bestätigung für UE-C++-API-Kompatibilität, Reflection, reale Bildschirmprojektion, Capsule-Kollision, Tür-/Portalplatzierung, Lesbarkeit der Fotoansicht oder Audioausgabe. Ein erfolgreicher Audioadapter-Start bestätigt eine spielende UE-Komponente, keine vom Menschen gehörte Ausgabe. Auch die begrenzte Begegnung ist noch nicht visuell oder akustisch beurteilt.

Echte `.umap`-/`.uasset`-Erzeugung, vier vorbereitete Unreal-Automationstests, PIE, SaveGame-Datenträger-Roundtrip, Windows-Cook/Packaging und Hardwaremessung bleiben offen. Es gibt weiterhin **keinen bestätigten spielbaren Build**. [TEST_PLAN.md](TEST_PLAN.md) enthält die ergänzte spätere Abnahme; diese ist kein Auftrag zu weiterer lokaler Fehlersuche in der aktuellen Cloud-Iteration.
