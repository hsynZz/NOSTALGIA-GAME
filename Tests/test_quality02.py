"""Asset integrity and preparation contracts, not a substitute for Unreal execution."""
import hashlib
import json
import importlib.util
import math
from pathlib import Path
import struct
import tempfile
import unittest
from urllib.parse import urlparse

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / "SourceArt/Quality02"


def read_glb(path):
    data = path.read_bytes()
    magic, version, length = struct.unpack_from('<4sII', data)
    if magic != b'glTF' or version != 2 or length != len(data):
        raise ValueError(f'Invalid GLB header: {path}')
    size, kind = struct.unpack_from('<I4s', data, 12)
    if kind != b'JSON':
        raise ValueError('Missing JSON chunk')
    return json.loads(data[20:20+size])


class QualityAssets(unittest.TestCase):
    def test_vendored_sources_match_commit_pins_and_license_evidence(self):
        lock = json.loads((ART/'sources.lock.json').read_text())
        self.assertEqual(len(lock['assets']), 5)
        for asset in lock['assets']:
            self.assertRegex(asset['commit'], r'^[a-f0-9]{40}$')
            evidence = (ART/'ThirdParty'/asset['id']/'README.md').read_text()
            self.assertIn('## Legal', evidence)
            self.assertTrue('Creative Commons Zero' in evidence or 'Creative Commons Attribution 4.0' in evidence)
            for item in asset['files']:
                path = ROOT/item['path']
                self.assertEqual(path.stat().st_size, item['bytes'])
                self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), item['sha256'])
                self.assertIn(asset['commit'], urlparse(item['url']).path)

    def test_delivered_geometry_contains_real_embedded_materials(self):
        manifest = json.loads((ART/'scene.manifest.json').read_text())
        names = set()
        triangles = 0
        for item in manifest['assets']:
            self.assertNotIn(item['name'], names)
            names.add(item['name'])
            path = ROOT/item['path']
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), item['sha256'])
            data = read_glb(path)
            self.assertEqual(len(data['meshes']), 1, item['name'])
            self.assertTrue(data['materials'])
            for buffer in data['buffers']:
                self.assertNotIn('uri', buffer, 'External dependency in GLB')
            for image in data.get('images', []):
                self.assertIn('bufferView', image)
                self.assertNotIn('uri', image)
            self.assertNotIn('KHR_draco_mesh_compression', data.get('extensionsRequired', []))
            triangles += item['triangles']
        self.assertIn('SM_DoorLeaf', names)
        self.assertGreater(triangles, 10000)
        self.assertLess(triangles, 1500000, 'Unexpected geometry budget regression')
        self.assertFalse(manifest['unreal_runtime_tested'])

    def test_degenerate_tangent_repair_preserves_geometry_and_handedness(self):
        spec=importlib.util.spec_from_file_location('repair',ROOT/'Scripts/quality02/repair_tangents.py')
        module=importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        binary=struct.pack('<10f',1,2,3,0,0,1,0,0,0,-1)
        model={'asset':{'version':'2.0'},'buffers':[{'byteLength':len(binary)}],
               'bufferViews':[{'buffer':0,'byteOffset':0,'byteLength':12},
                              {'buffer':0,'byteOffset':12,'byteLength':12},
                              {'buffer':0,'byteOffset':24,'byteLength':16}],
               'accessors':[{'bufferView':i,'componentType':5126,'count':1,'type':kind}
                            for i,kind in enumerate(['VEC3','VEC3','VEC4'])],
               'meshes':[{'primitives':[{'attributes':{'POSITION':0,'NORMAL':1,'TANGENT':2},'mode':0}]}]}
        encoded=json.dumps(model).encode()
        encoded+=b' '*(-len(encoded)%4)
        data=struct.pack('<4sII',b'glTF',2,28+len(encoded)+len(binary))
        data+=struct.pack('<I4s',len(encoded),b'JSON')+encoded+struct.pack('<I4s',len(binary),b'BIN\0')+binary
        with tempfile.TemporaryDirectory() as folder:
            path=Path(folder)/'test.glb'
            path.write_bytes(data)
            self.assertEqual(module.repair(path),1)
            repaired=path.read_bytes()
            values=struct.unpack('<10f',repaired[-40:])
            self.assertEqual(values[:6],(1,2,3,0,0,1))
            self.assertEqual(values[-1],-1)
            self.assertAlmostEqual(math.sqrt(sum(v*v for v in values[6:9])),1)
            self.assertAlmostEqual(sum(a*b for a,b in zip(values[3:6],values[6:9])),0)
            self.assertEqual(module.repair(path),0)
            self.assertEqual(path.read_bytes(),repaired)


if __name__ == '__main__':
    unittest.main()
