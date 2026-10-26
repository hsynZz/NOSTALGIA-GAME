# Aufnahmebrief und vorhandene Umsetzung

**Status: Quellaufnahme fehlt. Kein natürlicher Kinderlachen-Clip und kein fertiges Klangbild werden behauptet.**

Für den ersten Gartenmoment wird eine einzelne kurze, natürliche Aufnahme benötigt: etwa 3–8 Sekunden, mehrere leise Reaktionen ohne verständlichen Dialog, kein Schreien, keine Monsterstimme, keine Musik, keine eingebackene breite Stereo-/Halleffektkulisse. Ausgangsmaterial möglichst trocken, mono, 48 kHz. Die räumliche Perspektive soll Unreal erzeugen. Vor Einsatz müssen kommerzielle Nutzung, Bearbeitung, Weitergabe im Spiel und notwendige Aufnahmerechte dokumentiert sein. Das betrifft die konkrete Aufnahme; ein pauschaler Quellenname reicht nicht.

Nach Rechteprüfung die WAV über Unreal importieren und als `/Game/Quality02/ApprovedAudio/A_GardenLaughter` speichern. Im SoundWave Looping deaktivieren. Das Aufbauskript bindet eine vorhandene Mono-SoundWave zu; andernfalls protokolliert es ausdrücklich, dass Audio fehlt. Bei bereits gebauter Karte kann die Aufnahme im `AudioDirector` des `NostalgiaSliceDirector` gesetzt werden.

Vorhandene C++-Umsetzung: Quellenort am Tisch, sphärische Distanzdämpfung, Occlusion-Lowpass und Lautstärkedämpfung mit Übergang, Luftabsorption, Annäherungs-/Gebiets-Fade, ein bestätigter Start pro Director-Historie. Ein fehlgeschlagener Audiostart verbraucht das Ereignis nicht. Die Quellposition wird aus dem Szenenmanifest zugewiesen.

Spätere Hörabnahme mit Kopfhörern: links/rechts vorne/hinten plausibel; durch Hauswand merklich gedämpft; am Fenster anders als hinter der Wand; keine abrupten Pegelsprünge; keine Wiederholung beim Wiedereintritt; unter 3 m Annäherung ein sauberes Verstummen. Das sind **offene Unreal-Tests**, keine Cloud-Hörergebnisse.

Weitere benötigte Aufnahmen: ruhige lange Sommeratmosphäre, einzelne entfernte Vögel, sehr leiser Ventilator, Schritte Holz/Stein/Gras und passende Türmechanik. Diese Ebene wird nicht mit kurzen identischen Dauerschleifen oder permanentem Horror-Drone ersetzt. Kein automatischer Download ungeklärter Aufnahmen.
