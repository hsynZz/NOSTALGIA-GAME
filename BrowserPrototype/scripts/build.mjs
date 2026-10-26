/** Self-contained static delivery. No server, CDN, account or paid service at runtime. */
import { build } from 'esbuild';
import { createHash } from 'node:crypto';
import { readFile, writeFile, mkdir, rm } from 'node:fs/promises';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const dist = resolve(root, 'dist');
const delivery = resolve(root, 'Delivery');
const template = await readFile(resolve(root, 'index.html'), 'utf8');
for (const marker of ['<!--GAME_STYLE-->', '<!--GAME_SCRIPT-->']) {
  if (template.split(marker).length !== 2) throw new Error(`index.html needs exactly one ${marker}`);
}
const bundled = await build({
  absWorkingDir: root, entryPoints: ['src/main.js'], bundle: true, write: false,
  format: 'iife', platform: 'browser', target: ['es2020'], minify: true,
  legalComments: 'inline', sourcemap: false,
});
const css = await readFile(resolve(root, 'src/style.css'), 'utf8');
const javascript = bundled.outputFiles[0].text.replace(/<\/script/gi, '<\\/script');
const html = template.replace('<!--GAME_STYLE-->', `<style>${css}</style>`)
  .replace('<!--GAME_SCRIPT-->', `<script>${javascript}</script>`);
// Classic inline script and generated geometry/audio keep file:// working after unzip.
if (/<script\b[^>]*\bsrc\s*=/i.test(html) || /<link\b[^>]*\bhref\s*=/i.test(html)) {
  throw new Error('Offline delivery must not depend on external script or stylesheet files.');
}
await rm(dist, { recursive: true, force: true });
await mkdir(dist, { recursive: true });
await mkdir(delivery, { recursive: true });
await writeFile(resolve(dist, 'index.html'), html);
// Direct single-file alternative for users whose browser/app blocks ZIP downloads.
await writeFile(resolve(delivery, 'NOSTALGIA-0.1.html'), html);
await writeFile(resolve(dist, '.nojekyll'), '');
await writeFile(resolve(dist, 'README_STEUERUNG.txt'), `NOSTALGIA - Prototype 0.1 / Endless Summer 2004\n\n1. Das ZIP vollstaendig entpacken.\n2. index.html mit Microsoft Edge oder Google Chrome oeffnen.\n3. Im Spiel auf Aufwachen klicken. Zum Umschauen Mausklick ins Spiel; Esc gibt die Maus frei.\n\nWASD: bewegen. Maus: umsehen. Shift: sprinten. Leertaste: springen. C: ducken.\nE: interagieren. J: Foto. H: Hinweis. F5: speichern. F9: laden.\nAuf Touch-Geraeten: Bildschirmsteuerung; Safari verlangt den Start per Beruehrung fuer Audio.\n\nDie gesamte Testversion liegt in index.html. Kein Unreal Editor, Python, Node oder Internet erforderlich.\nWebGL2 muss im Browser verfuegbar sein. Bei einer WebGL-Fehlermeldung Hardwarebeschleunigung im Browser pruefen.\n\nDies ist die separate Browser-Testversion. Das Unreal-Projekt bleibt das langfristige Hauptprojekt.\nAlle Modelle und Klaenge sind einfache Testdarstellungen. Der Gartenklang ist synthetisch erzeugt.\nUnterstuetzung und Quellen: https://github.com/hsynZz/NOSTALGIA-GAME\n`);
const license = await readFile(resolve(root, 'node_modules/three/LICENSE'), 'utf8');
await writeFile(resolve(dist, 'THREE-LICENSE.txt'), license);
const packageInfo = JSON.parse(await readFile(resolve(root, 'package.json'), 'utf8'));
const sourceCommit = spawnSync('git', ['rev-parse', 'HEAD'], { cwd: root, encoding: 'utf8' });
await writeFile(resolve(dist, 'BUILD.json'), JSON.stringify({
  name: 'NOSTALGIA Browser Prototype', version: packageInfo.version,
  source_base_commit: sourceCommit.status === 0 ? sourceCommit.stdout.trim() : null,
  html_sha256: createHash('sha256').update(html).digest('hex'),
  runtime: 'standalone HTML / WebGL2', unreal_build: false,
}, null, 2) + '\n');
const zipPath = resolve(delivery, 'NOSTALGIA-Browser-0.1.zip');
const zipper = spawnSync('python3', ['-c', `
import pathlib, sys, zipfile
source, target = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
with zipfile.ZipFile(target, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
    for name in ['index.html', 'README_STEUERUNG.txt', 'THREE-LICENSE.txt', 'BUILD.json']:
        info = zipfile.ZipInfo(name, date_time=(1980,1,1,0,0,0))
        info.compress_type = zipfile.ZIP_DEFLATED
        info.external_attr = 0o100644 << 16
        archive.writestr(info, (source / name).read_bytes())
`, dist, zipPath], { encoding: 'utf8' });
if (zipper.status !== 0) throw new Error(`ZIP generation failed: ${zipper.stderr || zipper.error}`);
console.log(`Built ${resolve(dist, 'index.html')} (${html.length.toLocaleString('en')} characters)`);
console.log(`Offline delivery: ${zipPath}`);
