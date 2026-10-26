// Khronos reference validator: validates actual delivered GLB bytes, not mocked imports.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const validator = require('gltf-validator');
const root = path.resolve(__dirname, '../..');
const manifest = JSON.parse(fs.readFileSync(path.join(root, 'SourceArt/Quality02/scene.manifest.json')));
(async () => {
  const reports = [];
  for (const asset of manifest.assets) {
    const bytes = fs.readFileSync(path.join(root, asset.path));
    const report = await validator.validateBytes(new Uint8Array(bytes),
      { uri: asset.name + '.glb', maxIssues: 500 });
    reports.push({ asset: asset.path, sha256: crypto.createHash('sha256').update(bytes).digest('hex'), ...report });
    console.log(`${asset.name}: errors=${report.issues.numErrors} warnings=${report.issues.numWarnings}`);
  }
  fs.mkdirSync(path.join(root, 'Docs/Quality02/Validation'), { recursive: true });
  fs.writeFileSync(path.join(root, 'Docs/Quality02/Validation/gltf-validation.json'), JSON.stringify(reports, null, 2) + '\n');
  if (reports.some(report => report.issues.numErrors)) process.exitCode = 1;
})().catch(error => { console.error(error); process.exitCode = 1; });
