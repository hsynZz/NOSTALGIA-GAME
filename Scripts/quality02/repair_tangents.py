"""Repair undefined glTF tangent frames at degenerate UVs without changing geometry.

Blender/MikkTSpace can export zero tangents at beveled UV poles. glTF requires
unit vectors even there. Use a deterministic normal-orthogonal basis only for
undefined frames, retain valid frames and their handedness. Validate afterwards.
"""
import json
import math
from pathlib import Path
import struct


def orthogonal(normal):
    x,y,z=normal
    t=(-y,x,0) if abs(z)<.9 else (0,-z,y)
    length=math.sqrt(sum(v*v for v in t))
    if length<1e-8:
        raise ValueError('Cannot repair a tangent with an invalid normal')
    return tuple(v/length for v in t)


def repair(path):
    data=bytearray(Path(path).read_bytes())
    size,kind=struct.unpack_from('<I4s',data,12)
    if kind!=b'JSON': raise ValueError('Not a GLB JSON chunk')
    model=json.loads(data[20:20+size])
    binary=20+size+8
    checked=set()
    repaired=0
    for mesh in model['meshes']:
        for primitive in mesh['primitives']:
            attrs=primitive['attributes']
            if 'TANGENT' not in attrs or attrs['TANGENT'] in checked: continue
            checked.add(attrs['TANGENT'])
            tangent=model['accessors'][attrs['TANGENT']]
            normal=model['accessors'][attrs['NORMAL']]
            if tangent['componentType']!=5126 or normal['componentType']!=5126:
                raise ValueError('Repair requires float32 attributes')
            tv=model['bufferViews'][tangent['bufferView']]
            nv=model['bufferViews'][normal['bufferView']]
            for i in range(tangent['count']):
                to=binary+tv.get('byteOffset',0)+tangent.get('byteOffset',0)+i*tv.get('byteStride',16)
                no=binary+nv.get('byteOffset',0)+normal.get('byteOffset',0)+i*nv.get('byteStride',12)
                xyz=struct.unpack_from('<3f',data,to)
                length=math.sqrt(sum(v*v for v in xyz))
                if not math.isfinite(length): raise ValueError('Non-finite tangent')
                if length<1e-8:
                    xyz=orthogonal(struct.unpack_from('<3f',data,no))
                    struct.pack_into('<3f',data,to,*xyz)
                    repaired+=1
    Path(path).write_bytes(data)
    return repaired


if __name__=='__main__':
    import sys
    for filename in sys.argv[1:]: print(filename,repair(filename))
