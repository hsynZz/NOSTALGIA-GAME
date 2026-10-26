# NOSTALGIA – spielbare Browser-Testversion

Diese separate Three.js-Testversion setzt Endless Summer (2004) als kleine, lokal und online startbare 3D-Erfahrung um. Das Unreal-Projekt im Repository bleibt das langfristige Hauptprojekt. Der Browser läuft mit einer ausdrücklich portierten JavaScript-Logik; die native C++-Logik wird nicht im Browser ausgeführt.

## Ohne Installation ausprobieren

1. [Testversion als ZIP herunterladen](https://github.com/hsynZz/NOSTALGIA-GAME/raw/refs/heads/main/BrowserPrototype/Delivery/NOSTALGIA-Browser-0.1.zip).
2. ZIP vollständig entpacken.
3. `index.html` mit Microsoft Edge oder Chrome öffnen und im Spiel auf Start klicken.

Die ausgelieferte HTML-Datei enthält Three.js, Spielcode und Stil. Geometrie, Texturen und Testklänge werden lokal erzeugt; kein CDN und kein Server sind zur Laufzeit erforderlich. Ein aktueller Browser mit WebGL2 wird benötigt. Die ZIP-Datei ist ein Browser-Build, keine Windows-EXE und kein Unreal-Build. Audio und Maussteuerung werden erst durch die Startaktion freigegeben.

WASD bewegt, die Maus dreht den Blick, Shift sprintet, Leertaste springt, C duckt (Strg ist ebenfalls belegt) und E interagiert. Esc gibt die Maus frei. Weitere Aktionen werden im Spiel angezeigt. Für Touch-Geräte ist eine Bildschirmsteuerung vorgesehen; ein echter iPhone-/Safari-Hardwaretest bleibt gesondert erforderlich.

J zeigt das gefundene Foto erneut, H gibt einen Hinweis, F5 speichert und F9 lädt. Auf dem Bildschirm gibt es dieselben Aktionen als Schaltflächen. Falls die Maus nicht automatisch eingefangen wird, die linke Maustaste halten und zum Umsehen ziehen. Der Ton wird per Startaktion aktiviert; die Gartenstimme ist ausdrücklich ein synthetischer Testklang.

Der vollständige Escape-Ablauf und die Touch-Eingaben wurden in echtem Chromium geprüft. Ergebnisse, konkrete Abkürzungen des automatisierten Testfahrers und offene Gerätetests stehen in [VALIDATION.md](VALIDATION.md). Das Standalone-HTML wurde mit deaktiviertem Browsernetz ausgeführt; direkter Windows-Doppelklick und iPhone-Safari sind noch nicht auf echter Hardware geprüft.

## GitHub Pages

Vorgesehene Adresse nach erfolgreicher Veröffentlichung: **https://hsynzz.github.io/NOSTALGIA-GAME/**

Diese Adresse gilt erst dann als veröffentlicht, wenn der GitHub-Actions-Job `deploy` erfolgreich ist. Die Cloud-Sitzung konnte die Pages-Einstellungen nicht über die GitHub-API lesen oder ändern; der API-Zugriff war verboten beziehungsweise die vorhandene Authentifizierung ungültig. Ein HTTPS-Aufruf der vorgesehenen Pages-Adresse wurde vom Netzwerkproxy mit CONNECT 403 blockiert. Das beweist weder einen vorhandenen noch einen fehlenden öffentlichen Build. Es wird deshalb kein erfolgreiches Hosting behauptet.

Falls GitHub Pages noch nicht eingerichtet ist, ist genau diese einmalige Repository-Einstellung nötig:

1. Repository auf GitHub öffnen → **Settings → Pages**.
2. Unter **Build and deployment → Source** die Option **GitHub Actions** wählen.
3. Unter **Actions → Browser prototype - build and GitHub Pages → Run workflow** den Workflow auf `main` starten.
4. Falls die Umgebung `github-pages` eine Freigabe verlangt, den Deployment-Job dort freigeben.
5. Nach erfolgreichem Deployment den vom Job ausgegebenen Link verwenden.

Der Workflow nutzt GitHub Pages und GitHub Actions; keine kostenpflichtigen Drittanbieter werden eingebunden. GitHub-Pläne und Repository-Sichtbarkeit können die Verfügbarkeit von Pages beeinflussen. Die herunterladbare Testversion ist davon unabhängig. Die bestehende Unreal-Offline-Prüfung bleibt ein eigener Workflow.

Der Build-Job führt auch die echten Chromium-Spielprüfungen aus und archiviert deren Screenshots/Logs. Sein zukünftiger Erfolg auf GitHub wird nicht aus dem lokalen Cloud-Test abgeleitet.

## Entwicklung und reproduzierbarer Build

Voraussetzungen für Entwickler: Node.js 22+, npm, Python 3 für das ZIP und GCC/C++20 für die Paritätstests.

```sh
cd BrowserPrototype
npm ci
npm test
npm run build
```

`dist/index.html` ist die direkt startbare Datei. `Delivery/NOSTALGIA-Browser-0.1.zip` enthält sie, eine kurze Anleitung, die Three.js-Lizenz und `BUILD.json` mit HTML-Prüfsumme. `BUILD.json` nennt den Basiscommit vor dem Build; er ist kein Nachweis eines Unreal-Builds oder eines ausgeführten Spieltests. Statisches Hosting liefert `dist/` unverändert aus. Ein Webserver ist für die lokale HTML-Auslieferung nicht nötig.

Die Three.js-MIT-Lizenz liegt der Testversion bei. Abhängigkeiten sind mit Lockfile festgelegt. Der Build lädt zur Laufzeit keine fremden Modelle, Bilder, Stimmen oder Schriftarten.
