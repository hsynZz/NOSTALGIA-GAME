"""Real Blender mesh checks for the authored first-person route. NOT Unreal physics.

blender -b --python Scripts/quality02/check_scene.py
"""
import hashlib
import json
import math
from pathlib import Path

import bpy
from mathutils import Vector
from mathutils.bvhtree import BVHTree

ROOT = Path(__file__).resolve().parents[2]
manifest = json.loads((ROOT/'SourceArt/Quality02/scene.manifest.json').read_text())
bpy.ops.wm.read_factory_settings(use_empty=True)
for item in manifest['assets']:
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(ROOT/item['path']))
    meshes = [o for o in set(bpy.data.objects)-before if o.type == 'MESH']
    if len(meshes) != 1:
        raise RuntimeError('Combined GLB did not round-trip as one mesh: '+item['name'])
    meshes[0].name=item['name']
door=bpy.data.objects['SM_DoorLeaf']
for location in [(-11.4,-2.65,0),(-5.9,-2.65,0)]:
    copy=door.copy()
    copy.data=door.data
    bpy.context.scene.collection.objects.link(copy)
    copy.location=location
    copy.rotation_mode='XYZ'
    copy.rotation_euler.z=-math.pi/2
bpy.context.view_layer.update()
deps = bpy.context.evaluated_depsgraph_get()
colliders = []
for item in manifest['assets']:
    if item['collision'] == 'complex_static':
        ob = bpy.data.objects[item['name']]
        colliders.append((ob.name, ob.matrix_world, BVHTree.FromObject(ob, deps)))
# Preview door instances are open, exactly as shown in the rendered images.
for ob in bpy.context.scene.objects:
    if ob.name.startswith('SM_DoorLeaf.'):
        colliders.append((ob.name, ob.matrix_world, BVHTree.FromObject(ob, deps)))

route = [(-12.2,-2.15),(-11.0,-2.15),(-10.65,-.2),(-6.45,-.2),
         (-6.45,-2.15),(-5.25,-2.15),(-5.25,4.0),(-9.5,4.0)]
failures = []
samples = 0
for start, end in zip(route,route[1:]):
    a,b = Vector((*start,0)),Vector((*end,0))
    length = (b-a).length
    # Swept horizontal body checks: three heights, eight radial support points.
    for height in (.12,.85,1.58):
        for angle in range(8):
            offset = Vector((.34*math.cos(angle*math.pi/4),.34*math.sin(angle*math.pi/4),height))
            p,q = a+offset,b+offset
            for name,matrix,bvh in colliders:
                inverse=matrix.inverted()
                local_a,local_b=inverse@p,inverse@q
                direction=local_b-local_a
                hit,normal,index,distance=bvh.ray_cast(local_a,direction.normalized(),direction.length)
                samples+=1
                if hit is not None:
                    failures.append({'segment':[start,end], 'mesh':name,'height':height,'ray':angle})
    # Ground samples: no hidden floor gap along the walking line.
    for step in range(int(length/.15)+2):
        p=a.lerp(b,min(1,step*.15/length))+Vector((0,0,.08))
        hit_ground=False
        for name,matrix,bvh in colliders:
            inverse=matrix.inverted()
            hit,normal,index,distance=bvh.ray_cast(inverse@p,Vector((0,0,-1)),.3)
            if hit is not None:
                hit_ground=True
                break
        if not hit_ground:
            failures.append({'floor_gap_at':list(p)})

report={'engine':'Blender mesh BVH; not Unreal collision','unreal':False,
        'scene_manifest_sha256':hashlib.sha256((ROOT/'SourceArt/Quality02/scene.manifest.json').read_bytes()).hexdigest(),
        'route_m':route,'capsule_radius_m':.34,'height_m':1.76,'open_doors':True,
        'ray_checks':samples,'failures':failures,'passed':not failures}
(ROOT/'Docs/Quality02/Validation/walk-route.json').write_text(json.dumps(report,indent=2)+'\n')
print('QUALITY02_WALK_ENVELOPE', 'PASS' if not failures else 'FAIL', len(failures), 'Unreal NOT RUN')
if failures:
    raise RuntimeError(str(failures[:10]))
