"""Author/export the Quality 0.2 scene in Blender 4.3+. No Unreal binary fabrication.

blender -b -t 8 --python Scripts/quality02/build_scene.py -- --render
Source units: meters, Z up. Exported GLBs retain PBR maps; UE import is separate.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import sys

import bpy
from mathutils import Vector
import numpy as np
sys.path.insert(0, str(Path(__file__).resolve().parent))
from repair_tangents import repair
from denoise_render import finish_render

ROOT = Path(__file__).resolve().parents[2]
SCRIPT_HASH = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
ART = ROOT / "SourceArt/Quality02"
OUT = ART / "Generated"
TEMP = ROOT / "Artifacts/Quality02"
RENDERS = ROOT / "Docs/Quality02/Renders"
for directory in (OUT, TEMP, RENDERS):
    directory.mkdir(parents=True, exist_ok=True)

args = argparse.ArgumentParser()
args.add_argument("--render", action="store_true")
args.add_argument("--samples", type=int, default=48)
args.add_argument("--width", type=int, default=1440)
args.add_argument("--oidn", type=Path, help="Optional Intel Open Image Denoise executable; no automatic download")
options = args.parse_args(sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else [])
bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene
scene.unit_settings.system = "METRIC"
scene.unit_settings.scale_length = 1
groups = {}
current = "Architecture"


def track(ob, name):
    ob.name = name
    groups.setdefault(current, []).append(ob)
    return ob


def material(name, color, roughness=.6, metal=0):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    p = m.node_tree.nodes.get("Principled BSDF")
    p.inputs["Base Color"].default_value = (*color, 1)
    p.inputs["Roughness"].default_value = roughness
    p.inputs["Metallic"].default_value = metal
    m.diffuse_color = (*color, 1)
    return m


def textile(m, color, scale=1):
    """Original woven microdetail stored as real image maps, not Blender-only noise."""
    size = 512
    yy, xx = np.mgrid[:size, :size]
    weave = np.sin(xx * math.pi / 2) * np.sin(yy * math.pi / 2)
    broad = .5 * np.sin(xx / 23) * np.sin(yy / 19)
    pixels = np.ones((size, size, 4), dtype=np.float32)
    for c in range(3):
        pixels[:, :, c] = np.clip(color[c] * (1 + .035 * weave + .025 * broad), 0, 1)
    im = bpy.data.images.new(m.name + "_BaseColor", size, size)
    im.pixels.foreach_set(pixels.ravel())
    im.filepath_raw = str(TEMP / (im.name + ".png"))
    im.file_format = "PNG"
    im.save()
    im.pack()
    n = m.node_tree.nodes.new("ShaderNodeTexImage")
    n.image = im
    m.node_tree.links.new(n.outputs["Color"], m.node_tree.nodes.get("Principled BSDF").inputs["Base Color"])
    normal = np.ones_like(pixels)
    normal[:, :, 0] = .5 + .065 * np.cos(xx * math.pi / 2)
    normal[:, :, 1] = .5 + .065 * np.cos(yy * math.pi / 2)
    normal[:, :, 2] = 1
    nm = bpy.data.images.new(m.name + "_Normal", size, size)
    nm.colorspace_settings.name = "Non-Color"
    nm.pixels.foreach_set(normal.ravel())
    nm.filepath_raw = str(TEMP / (nm.name + ".png"))
    nm.file_format = "PNG"
    nm.save()
    nm.pack()
    tex = m.node_tree.nodes.new("ShaderNodeTexImage")
    tex.image = nm
    normal_node = m.node_tree.nodes.new("ShaderNodeNormalMap")
    normal_node.inputs["Strength"].default_value = .3
    m.node_tree.links.new(tex.outputs["Color"], normal_node.inputs["Color"])
    m.node_tree.links.new(normal_node.outputs["Normal"], m.node_tree.nodes.get("Principled BSDF").inputs["Normal"])
    return m


def box(name, loc, size, mat, bevel=.01, rotation=0):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
    ob = track(bpy.context.object, name)
    ob.dimensions = size
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    ob.rotation_euler.z = rotation
    ob.data.materials.append(mat)
    if bevel:
        mod = ob.modifiers.new("Crafted rounded edges", "BEVEL")
        mod.width = bevel
        mod.segments = 3
        mod = ob.modifiers.new("Weighted normals", "WEIGHTED_NORMAL")
        mod.keep_sharp = True
    return ob


def sphere(name, loc, size, mat):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=32, ring_count=16, location=loc)
    ob = track(bpy.context.object, name)
    ob.scale = size
    ob.data.materials.append(mat)
    for p in ob.data.polygons:
        p.use_smooth = True
    return ob


def rod(name, a, b, radius, mat, vertices=24):
    a, b = Vector(a), Vector(b)
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices, radius=radius, depth=(b-a).length, location=(a+b)/2)
    ob = track(bpy.context.object, name)
    ob.rotation_euler = (b-a).to_track_quat("Z", "Y").to_euler()
    ob.data.materials.append(mat)
    bevel = ob.modifiers.new("Edge radius", "BEVEL")
    bevel.width = min(radius / 3, .005)
    bevel.segments = 2
    for p in ob.data.polygons:
        p.use_smooth = True
    return ob


def surface(name, nu, nv, fn, mat, thickness=0):
    verts = [fn(i/nu, j/nv) for j in range(nv+1) for i in range(nu+1)]
    faces = [(j*(nu+1)+i, j*(nu+1)+i+1, (j+1)*(nu+1)+i+1, (j+1)*(nu+1)+i)
             for j in range(nv) for i in range(nu)]
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    ob = bpy.data.objects.new(name, mesh)
    scene.collection.objects.link(ob)
    track(ob, name)
    ob.data.materials.append(mat)
    uv = mesh.uv_layers.new()
    for poly in mesh.polygons:
        poly.use_smooth = True
        for loop in poly.loop_indices:
            k = mesh.loops[loop].vertex_index
            uv.data[loop].uv = ((k % (nu+1))/nu, (k//(nu+1))/nv)
    if thickness:
        mod = ob.modifiers.new("Real fabric thickness", "SOLIDIFY")
        mod.thickness = thickness
    return ob


def gable(name, x, y0, y1, base, peak, mat):
    mesh=bpy.data.meshes.new(name)
    mesh.from_pydata([(x,y0,base),(x,y1,base),(x,(y0+y1)/2,peak)],[],[(0,1,2)])
    mesh.update()
    ob=bpy.data.objects.new(name,mesh)
    scene.collection.objects.link(ob)
    track(ob,name)
    ob.data.materials.append(mat)
    solid=ob.modifiers.new('Gable wall thickness','SOLIDIFY')
    solid.thickness=.14
    return ob


def wall_with_openings(name, axis, fixed, coordinates, heights, holes, mat, thickness):
    """One connected wall surface: no coplanar overlapping boxes at jamb/lintel seams."""
    verts=[]
    for z in heights:
        for u in coordinates:
            verts.append((fixed,u,z) if axis=='X' else (u,fixed,z))
    faces=[]
    nu=len(coordinates)
    for j in range(len(heights)-1):
        for i in range(nu-1):
            u=(coordinates[i]+coordinates[i+1])/2
            z=(heights[j]+heights[j+1])/2
            if any(a<u<b and c<z<d for a,b,c,d in holes):
                continue
            faces.append((j*nu+i,j*nu+i+1,(j+1)*nu+i+1,(j+1)*nu+i))
    mesh=bpy.data.meshes.new(name)
    mesh.from_pydata(verts,[],faces)
    mesh.update()
    ob=bpy.data.objects.new(name,mesh)
    scene.collection.objects.link(ob)
    track(ob,name)
    ob.data.materials.append(mat)
    uv=mesh.uv_layers.new()
    for poly in mesh.polygons:
        for loop in poly.loop_indices:
            v=mesh.vertices[mesh.loops[loop].vertex_index].co
            uv.data[loop].uv=((v.y if axis=='X' else v.x)/3,v.z/3)
    solid=ob.modifiers.new('Continuous wall thickness','SOLIDIFY')
    solid.thickness=thickness
    solid.offset=0
    return ob


def label(name, body, loc, size, mat, rotation=(math.pi/2, 0, 0)):
    curve = bpy.data.curves.new(name, "FONT")
    curve.body = body
    curve.size = size
    curve.extrude = .0002
    ob = bpy.data.objects.new(name, curve)
    scene.collection.objects.link(ob)
    track(ob, name)
    ob.location = loc
    ob.rotation_euler = rotation
    ob.data.materials.append(mat)
    return ob


def imported(asset, loc, width, yaw=0):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(ART / "ThirdParty" / asset / (asset + ".glb")))
    new = set(bpy.data.objects)-before
    meshes = [o for o in new if o.type == "MESH" and "firefl" not in o.name.lower()
              and not (asset == "ToyCar" and o.name == "Fabric")]
    # Discard source lights/cameras/animations: the scene owns lighting and staging.
    for ob in meshes:
        matrix = ob.matrix_world.copy()
        ob.parent = None
        ob.matrix_world = matrix
        ob.animation_data_clear()
    for ob in new-set(meshes):
        bpy.data.objects.remove(ob, do_unlink=True)
    bpy.ops.object.select_all(action="DESELECT")
    for ob in meshes:
        ob.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    bpy.ops.object.join()
    ob = bpy.context.object
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    mins = Vector([min(v.co[i] for v in ob.data.vertices) for i in range(3)])
    maxs = Vector([max(v.co[i] for v in ob.data.vertices) for i in range(3)])
    s = width / (maxs.x-mins.x)
    center = Vector(((mins.x+maxs.x)/2, (mins.y+maxs.y)/2, mins.z))
    for v in ob.data.vertices:
        v.co = (v.co-center)*s
    ob.location = loc
    ob.rotation_mode = "XYZ"
    ob.rotation_euler.z = yaw
    track(ob, asset)
    return ob


plaster = material("M_WarmChalkPlaster", (.73, .69, .57), .88)
ceiling = material("M_ChalkCeiling", (.82, .79, .69), .9)
paint = material("M_AgedIvoryJoinery", (.68, .63, .49), .4)
sage = material("M_SoftSageWall", (.25, .35, .29), .85)
blue = material("M_FadedBluePaint", (.16, .28, .34), .48)
cream = material("M_CreamPlastic", (.57, .54, .44), .38)
brass = material("M_DullBrass", (.45, .28, .09), .3, .8)
dark = material("M_CharcoalPlastic", (.017, .021, .019), .36)
paper = material("M_AgedPaper", (.86, .78, .58), .9)
ink = material("M_Ink", (.055, .075, .075), .85)
terra = material("M_Terracotta", (.29, .105, .055), .85)
concrete = material("M_PathStone", (.45, .44, .36), .92)
grass = material("M_SummerGrass", (.19, .26, .075), .93)
leaf = material("M_Leaf", (.09, .19, .043), .72)
bark = material("M_Bark", (.12, .08, .045), .96)
linen = textile(material("M_CottonBedlinen", (.57, .67, .69), .93), (.57, .67, .69))
curtain = textile(material("M_CurtainWeave", (.78, .73, .58), .92), (.78, .73, .58))
curtain.node_tree.nodes.get("Principled BSDF").inputs["Subsurface Weight"].default_value = .06
glass = material("M_ClearWindowGlass", (.92, .97, 1), .04)
glass.node_tree.nodes.get("Principled BSDF").inputs["Transmission Weight"].default_value = 1
glass.node_tree.nodes.get("Principled BSDF").inputs["IOR"].default_value = 1.45

# A worn timber material from the licensed chair supplies genuine photographed wood maps.
current = "LivingFurniture"
chair = imported("SheenChair", (-9.9, .75, 0), .76, -.3)
wood = bpy.data.materials["wood Brown"].copy()
wood.name = "M_WornTimber_CC0"
# The chair's baked AO UV2 describes a chair, never a floorboard or desk.
for node in list(wood.node_tree.nodes):
    if node.type == "GROUP":
        wood.node_tree.nodes.remove(node)
sofa = imported("SheenWoodLeatherSofa", (-8.25, -2.55, 0), 2.25, math.pi)
plant = imported("DiffuseTransmissionPlant", (-6.5, 1.0, 0), .75)

current = "Architecture"
# House shell, actual openings. Bedroom 4.6 x 4.8 m, living room 5.4 x 4.8 m.
box("House foundation", (-11, -.8, -.12), (10.4, 5.2, .24), concrete)
box("South wall", (-11, -3.3, 1.4), (10.4, .2, 2.8), plaster)
box("West wall", (-16.1, -.8, 1.4), (.2, 5.2, 2.8), sage)
for x in (-11.4, -5.9):
    wall_with_openings('Continuous door wall','X',x,[-3.3,-2.65,-1.65,1.7],[0,2.12,2.8],
                       [(-2.65,-1.65,0,2.12)],plaster,.16)
    for side in (-1,1):
        for y in (-2.69,-1.61):
            box("Door architrave", (x+side*.095,y,1.075),(.028,.07,2.15),paint,.008)
        box("Door architrave top", (x+side*.095,-2.15,2.15),(.028,1.15,.07),paint,.008)
wall_with_openings('Continuous window wall','Y',1.7,[-16.2,-15.4,-12.1,-9.9,-6.9,-5.8],
                   [0,.85,2.48,2.8],[(-15.4,-12.1,.85,2.48),(-9.9,-6.9,.85,2.48)],plaster,.2)
box("Ceiling", (-11, -.8, 2.85), (10.4, 5.2, .1), ceiling)
# Individual timber planks have seams and real bevels, with phase-varied UVs.
for row in range(24):
    for col in range(5):
        x0 = -16 + col*2.0
        ob = box("Floorboard", (x0+1, -3.2+(row+.5)*.2, .012), (1.997, .197, .024), wood, .0015)
        for loop in ob.data.uv_layers.active.data:
            loop.uv.x += col*.179 + row*.073
            loop.uv.y += row*.191
for y in (-3.18, 1.57):
    box("Stepped skirting", (-11, y, .065), (9.98, .028, .13), paint, .004)
    rod("Skirting cap bead", (-15.98,y-.013,.132), (-6.02,y-.013,.132), .009, paint)
box("West skirting", (-15.97, -.8, .065), (.03, 4.8, .13), paint, .004)

current = "WindowsCurtains"
for x0, x1 in [(-15.4, -12.1), (-9.9, -6.9)]:
    for x in (x0, (x0+x1)/2, x1):
        box("Window jamb", (x, 1.65, 1.665), (.075, .13, 1.69), paint, .009)
    for z in (.85, 2.48):
        box("Window frame", ((x0+x1)/2, 1.65, z), (x1-x0+.12, .13, .08), paint, .009)
    box("Deep timber sill", ((x0+x1)/2, 1.56, .82), (x1-x0+.22, .31, .055), wood, .015)
    for a,b in [(x0+.06,(x0+x1)/2-.04),((x0+x1)/2+.04,x1-.06)]:
        current = "WindowGlass"
        box("Window glass", ((a+b)/2, 1.7, 1.665), (b-a, .006, 1.52), glass, 0)
        current = "WindowsCurtains"
    rod("Curtain rail", (x0-.15,1.36,2.59),(x1+.15,1.36,2.59),.016,brass)
    for start in (x0-.1,x1-.66):
        surface("Gathered cotton curtain", 56, 36,
                lambda u,v,s=start:(s+.76*u, 1.32+.065*math.sin(u*math.pi*14)*(1-.18*v),
                                    .22+2.32*v+.022*math.sin(u*math.pi*14)*(1-v)), curtain, .0015)
    rod("Casement handle", ((x0+x1)/2+.07,1.53,1.4),((x0+x1)/2+.07,1.53,1.55),.009,brass)

current = "BedroomFurniture"
# Timber bed: frame, slats, turned posts, layered mattress, sewn fabric geometry.
for x in (-15.75,-14.65):
    for y in (-2.7,-.45):
        rod("Bedpost", (x,y,.03),(x,y,.95 if y< -2 else .62),.037,wood)
        sphere("Bedpost finial", (x,y,1.0 if y< -2 else .67),(.052,.052,.052),wood)
    box("Bed side rail",(x,-1.58,.29),(.065,2.25,.16),wood,.01)
for y,z in [(-2.7,.77),(-.45,.49)]:
    box("Bed end top rail",(-15.2,y,z),(1.1,.075,.09),wood,.025)
    for i in range(7):
        rod("Headboard spindle",(-15.66+i*.155,y,.3),(-15.66+i*.155,y,z),.012,wood)
box("Mattress piped edge",(-15.2,-1.57,.42),(1.04,2.12,.24),paper,.09)
surface("Duvet with hanging edges", 64, 72,
        lambda u,v:(-15.2+(u-.5)*1.33,-2.45+v*1.97,
                    .58-.22*(abs(u-.5)*2)**10+.025*math.sin(u*48+v*13)*math.sin(v*math.pi)
                    +.018*math.sin(v*47+u*9)), linen, .018)
for y in (-2.32,):
    ob=sphere("Soft pillow",(-15.2,y,.65),(.46,.28,.115),curtain)
    ob.rotation_euler.z=.04
for x in (-14.15,-12.75):
    for y in (.58,1.2):
        box("Desk tapered leg",(x,y,.36),(.045,.045,.7),wood,.008)
box("Desk rounded top",(-13.45,.9,.745),(1.55,.78,.06),wood,.018)
box("Desk drawer carcass",(-13.45,.82,.63),(1.42,.62,.16),wood,.008)
for x in (-13.8,-13.1):
    box("Drawer inset",(x,.493,.63),(.66,.018,.13),paint,.007)
    rod("Drawer handle",(x-.065,.465,.64),(x+.065,.465,.64),.007,brass)
radio=imported("BoomBox",(-13.08,.94,.78),.38,0)
toy=imported("ToyCar",(-13.82,.64,.78),.19,.5)
# CRT with sculpted housing, curved dark glass, controls and vent slots.
box("CRT pedestal",(-14.0,1.0,.8),(.34,.29,.045),cream,.025)
box("CRT monitor shell",(-14.0,1.0,1.04),(.43,.39,.4),cream,.055)
box("CRT bezel",(-14.0,.795,1.065),(.397,.035,.315),cream,.025)
screen=box("CRT convex glass",(-14.0,.775,1.067),(.335,.016,.25),dark,.055)
for i in range(10):
    box("CRT air vent",(-14.2,.86+i*.022,1.15),(.003,.009,.12),dark,.001)
sphere("Power button",(-13.86,.77,.9),(.012,.006,.012),dark)
box("Keyboard body",(-14,.47,.805),(.37,.14,.036),cream,.008)
for row in range(4):
    for col in range(12):
        box("Keyboard key",(-14.16+col*.028,.425+row*.026,.83),(.022,.021,.009),paint,.002)
# Hand-built desk chair, not the living-room upholstered asset.
box("Desk chair seat",(-13.4,-.1,.45),(.42,.42,.045),blue,.035)
for x in (-13.57,-13.23):
    for y in (-.27,.07):
        rod("Chair leg",(x,y,.02),(x,y,.46),.017,wood)
    rod("Chair back upright",(x,-.27,.4),(x,-.31,.92),.018,wood)
box("Chair curved back rail",(-13.4,-.3,.84),(.42,.045,.19),blue,.035)
# A modest shelf: authored books and an original August 2004 calendar.
box("Wall shelf",(-12.3,-3.02,1.62),(1.3,.3,.045),wood,.01)
book_colors=[blue,terra,paint,sage]
for i in range(8):
    box("Paperback cover",(-12.79+i*.09,-3.01,1.78),(.07,.19,.27+(i%3)*.02),book_colors[i%4],.003)
    box("Paperback pages",(-12.79+i*.09,-2.909,1.78),(.058,.006,.25+(i%3)*.02),paper,.001)
box("Calendar paper",(-15.95,-1.1,1.68),(.004,.42,.55),paper,.001)
label("Original calendar", "AUGUST\n2004\n\n17", (-15.944,-1.28,1.85), .07, ink, (math.pi/2,0,math.pi/2))
for x in (-14.05,-12.85):
    box("Wall socket plate",(x,-3.185,.28),(.085,.012,.085),paint,.009)
    for dx in (-.013,.013):
        sphere("Socket hole",(x+dx,-3.177,.28),(.003,.002,.003),dark)
box("Notebook",(-13.42,.63,.791),(.21,.29,.018),paper,.004,.12)
rod("Pencil",(-13.48,.52,.806),(-13.4,.7,.806),.003,terra)
box("Woven bedside rug",(-14,-1.5,.03),(1.12,1.9,.014),curtain,.008)

current = "LivingFurniture"
box("Coffee table top",(-8.8,-.95,.43),(1.25,.65,.05),wood,.045)
for x in (-9.3,-8.3):
    for y in (-1.19,-.71):
        box("Coffee table leg",(x,y,.2),(.045,.045,.4),wood,.009)
for i in range(3):
    box("Coffee table paperback",(-8.65+i*.015,-.95,.47+i*.024),(.2,.28,.022),book_colors[i],.003,.12*i)

current = "Garden"
box("Garden soil",(-7,5,-.12),(24,16,.2),grass,.1)
# Deterministic grass within the authored lawn, with clear path/table exclusions.
# This is vegetation distribution, never random placement of story objects.
grass_vertices=[]
grass_faces=[]
for ix in range(110):
    for iy in range(34):
        x=-18+ix*.145+.025*math.sin(ix*7+iy*13)
        y=2+iy*.15+.025*math.cos(ix*11+iy*3)
        if (-5.95<x<-4.55) or (-14.5<x<-11.1 and 4.0<y<6.5):
            continue
        angle=ix*2.399+iy*1.71
        height=.045+.035*(.5+.5*math.sin(ix*13+iy*31))
        for turn in (0,1.57):
            dx,dy=math.cos(angle+turn)*.007,math.sin(angle+turn)*.007
            n=len(grass_vertices)
            grass_vertices.extend([(x-dx,y-dy,-.015),(x+dx,y+dy,-.015),
                                   (x+dx*2+.012,y+dy*2,height)])
            grass_faces.append((n,n+1,n+2))
mesh=bpy.data.meshes.new('Authored lawn blades')
mesh.from_pydata(grass_vertices,[],grass_faces)
mesh.update()
ob=bpy.data.objects.new('Authored lawn blades',mesh)
scene.collection.objects.link(ob)
ob.data.materials.append(grass)
track(ob,'Authored lawn blades')
for i in range(12):
    box("Path limestone slab",(-5.25, -2.3+i*.65,.015),(1.12,.62,.06),concrete,.014)
# A quiet, abandoned table visible from the bedroom, no labels or glowing quest object.
for x in (-13.7,-12.1):
    for y in (4.55,5.25):
        box("Garden table leg",(x,y,.36),(.055,.055,.72),paint,.01)
box("Garden table",(-12.9,4.9,.75),(1.85,.95,.06),paint,.025)
surface("Birthday tablecloth",60,40,
        lambda u,v:(-12.9+(u-.5)*2.3,4.9+(v-.5)*1.42,
                    .79-.3*max((abs(u-.5)*2)**12,(abs(v-.5)*2)**12)
                    +.012*math.sin(u*55+v*32)),curtain,.002)
for x,y,yaw in [(-14.1,4.9,math.pi/2),(-11.7,4.9,-math.pi/2),(-12.9,5.8,math.pi),(-11.3,6.2,.55)]:
    for dx in (-.18,.18):
        for dy in (-.18,.18):
            rod("Garden chair leg",(x+dx,y+dy,.02),(x+dx,y+dy,.44),.016,paint)
    box("Garden chair seat",(x,y,.45),(.44,.44,.035),paint,.02,yaw)
    for dx in (-.18,.18):
        rod("Garden chair back",(x+dx,y+.18,.43),(x+dx,y+.18,.92),.016,paint)
    for z in (.68,.79,.89):
        box("Garden chair back slat",(x,y+.18,z),(.42,.025,.06),paint,.01)
for x,y in [(-13.45,4.7),(-12.35,4.7),(-12.9,5.15)]:
    rod("Ceramic plate",(x,y,.793),(x,y,.804),.105,paper,48)
    rod("Water glass",(x+.12,y+.11,.8),(x+.12,y+.11,.91),.033,glass,32)
# Fence pickets and rails, a boundary with physical construction detail.
for x in np.arange(-18,3,.2):
    box("Painted fence picket",(float(x),7.3,.54),(.09,.035,1.08),paint,.008)
for z in (.32,.82):
    box("Fence rail",(-7.5,7.32,z),(21,.045,.07),paint,.005)
# Curated trees: branches and individual leaf clusters, not sphere canopies.
def tree(x,y,height=5):
    rod("Tree trunk",(x,y,0),(x+.12,y+.1,height*.7),.12,bark,16)
    for branch in range(9):
        angle=branch*2.4
        end=Vector((x+math.cos(angle)*1.25,y+math.sin(angle)*1.25,height*.64+branch*.12))
        rod("Tree branch",(x,y,height*.47),end,.037,bark,10)
        for j in range(65):
            a=j*2.399+branch
            radius=.16+.72*((j*17%61)/61)**.5
            center=end+Vector((math.cos(a)*radius,math.sin(a)*radius,math.sin(j*1.7)*.42))
            ob=surface("Leaf cluster",2,2,lambda u,v,c=center,a=a:
                       tuple(c+Vector(((u-.5)*.29*math.cos(a)-(v-.5)*.12*math.sin(a),
                                      (u-.5)*.29*math.sin(a)+(v-.5)*.12*math.cos(a),
                                      .025*math.sin(u*math.pi)+.02*(v-.5)))),leaf)
    return
tree(-17.4,4.6,5.6)
tree(-2.7,6.1,5)

current = "NeighborhoodVista"
box("Residential street",(-7.5,9.7,-.03),(28,4,.08),material("M_Asphalt",(.12,.13,.12),.96),.02)
for y in (7.8,11.7):
    box("Road curb",(-7.5,y,.055),(28,.16,.14),concrete,.025)
# Exactly three composed silhouettes: pitched roofs, chimneys, jambs and glazing.
for x,c in [(-17,plaster),(-9,sage),(-1,plaster)]:
    box("Neighbor rendered facade",(x,16,1.7),(5.2,5.0,3.4),c,.025)
    for side in (-1,1):
        ob=box("Pitched tiled roof",(x,16+side*1.38,3.86),(5.8,3.0,.13),terra,.018)
        ob.rotation_euler.x=-side*math.radians(27)
    for end in (-1,1):
        gable('Neighbor gable',x+end*2.6,13.5,18.5,3.35,4.54,c)
    box("Chimney",(x+1.4,16.5,4.4),(.42,.5,1.2),terra,.01)
    for wx in (x-1.5,x+1.5):
        box("Vista window recess",(wx,13.47,1.9),(.95,.04,1.25),dark,.01)
        for xx in (wx-.5,wx,wx+.5):
            box("Vista window jamb",(xx,13.42,1.9),(.055,.08,1.35),paint,.005)
        for z in (1.24,1.9,2.56):
            box("Vista window crossbar",(wx,13.42,z),(1.06,.08,.055),paint,.005)
    box("Neighbor front door",(x,13.45,1.05),(.95,.06,2.1),wood,.01)

current = "Roof"
for side in (-1,1):
    ob=box("Home pitched roof",(-11,-.8+side*1.48,3.45),(10.7,3.3,.14),terra,.015)
    ob.rotation_euler.x=-side*math.radians(25)
    box('Home fascia',(-11,-.8+side*2.52,2.95),(10.7,.13,.24),paint,.01)
for x in (-16.13,-5.87):
    gable('Home gable',x,-3.32,1.72,2.8,4.147,plaster)

current = "DoorLeaf"
# Local hinge is origin; x thickness, y width, z height. Actor owns motion.
box("Door frame leaf",(0,.475,1.05),(.045,.95,2.1),paint,.006)
for z,h in [(.56,.73),(1.55,.73)]:
    box("Raised door panel",(-.025,.475,z),(.016,.72,h),wood,.012)
    for y in (.105,.845):
        box("Door panel molding",(-.037,y,z),(.02,.024,h+.05),paint,.005)
    for zz in (z-h/2,z+h/2):
        box("Door horizontal molding",(-.037,.475,zz),(.02,.76,.024),paint,.005)
for z in (.25,1.8):
    rod("Door hinge barrel",(0,.01,z-.05),(0,.01,z+.05),.015,brass)
for side in (-1,1):
    rod("Door rose",(side*.026,.82,1.02),(side*.045,.82,1.02),.027,brass)
    rod("Door lever",(side*.065,.72,1.02),(side*.065,.84,1.02),.01,brass)

# Freeze/export each authored group as one static mesh, preserving material slots.
# Imported lights/cameras never leak into the delivery.
assets=[]
for name,objects in groups.items():
    bpy.ops.object.select_all(action="DESELECT")
    for ob in objects:
        ob.select_set(True)
    bpy.context.view_layer.objects.active=objects[0]
    bpy.ops.object.convert(target="MESH")
    bpy.ops.object.join()
    ob=bpy.context.object
    ob.name="SM_"+name
    if name == "WindowGlass":
        # Thin clear glazing: transparent direct shadows; avoid unsupported refractive caustics.
        ob.visible_shadow = False
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    ob.modifiers.new("Stable tangent triangles", "TRIANGULATE")
    # Normal map tangent export requires UVs on every combined primitive.
    if not ob.data.uv_layers:
        ob.data.uv_layers.new()
    path=OUT/(ob.name+".glb")
    bpy.ops.export_scene.gltf(filepath=str(path),export_format="GLB",use_selection=True,
                             export_yup=True,export_apply=True,export_cameras=False,
                             export_lights=False,export_animations=False,export_image_format="AUTO",
                             export_tangents=True)
    repaired_tangents=repair(path)
    verts=ob.data.vertices
    bounds=[[min(v.co[i] for v in verts),max(v.co[i] for v in verts)] for i in range(3)]
    assets.append({"name":ob.name,"path":path.relative_to(ROOT).as_posix(),
                   "sha256":hashlib.sha256(path.read_bytes()).hexdigest(),
                   "bytes":path.stat().st_size,"bounds_m":bounds,
                   "repaired_undefined_tangents":repaired_tangents,
                   "triangles":sum(len(p.vertices)-2 for p in ob.data.polygons),
                   "material_slots":[m.name if m else "" for m in ob.data.materials],
                   "collision":"complex_static" if name not in ("WindowsCurtains","Roof","DoorLeaf") else "none",
                   "cast_shadow":name != "WindowGlass",
                   "dynamic":name=="DoorLeaf"})

door=bpy.data.objects["SM_DoorLeaf"]
door.hide_render=True
for location in [(-11.4,-2.65,0),(-5.9,-2.65,0)]:
    copy=door.copy()
    copy.data=door.data
    scene.collection.objects.link(copy)
    copy.hide_render=False
    copy.location=location
    copy.rotation_euler.z=-math.pi/2

manifest={"schema":1,"engine":"5.8.0","source_units":"meters_Z_up_right_handed",
          "map":"/Game/Quality02/Maps/EndlessSummer_Quality02",
          "assets":assets,"spawn_cm":[-1220,220,92],
          "doors_cm":[[-1140,265,0],[-590,265,0]],
          "garden_origin_cm":[-1290,-490,110],
          "unreal_import_tested":False,"unreal_runtime_tested":False}
(ART/"scene.manifest.json").write_text(json.dumps(manifest,indent=2)+"\n")

# Actual offline previews. This light setup is NOT claimed to match UE Lumen.
world=bpy.data.worlds.new("Summer daylight")
world.use_nodes=True
scene.world=world
nodes=world.node_tree.nodes
sky=nodes.new("ShaderNodeTexSky")
sky.sky_type="NISHITA"
sky.sun_elevation=math.radians(36)
sky.sun_rotation=math.radians(40)
sky.air_density=1.1
sky.dust_density=1.8
sky.sun_disc=False
world.node_tree.links.new(sky.outputs["Color"],nodes.get("Background").inputs["Color"])
nodes.get("Background").inputs["Strength"].default_value=.65
light=bpy.data.lights.new("Late summer sun", "SUN")
light.energy=2.2
light.angle=math.radians(.8)
light.color=(1,.88,.7)
ob=bpy.data.objects.new("Late summer sun",light)
scene.collection.objects.link(ob)
ob.rotation_euler=(math.radians(40),0,math.pi)

scene.render.engine="CYCLES"
scene.cycles.device="CPU"
scene.cycles.samples=options.samples
scene.cycles.use_denoising=False  # Cloud Blender build has no OpenImageDenoise.
scene.cycles.max_bounces=12
scene.render.resolution_x=options.width
scene.render.resolution_y=round(options.width*9/16)
scene.render.resolution_percentage=100
scene.render.image_settings.file_format="JPEG"
scene.render.image_settings.quality=94
scene.view_settings.view_transform="AgX"
scene.view_settings.look="AgX - Medium High Contrast"
scene.view_settings.exposure=1.3
camera=bpy.data.cameras.new("Proof camera")
cam=bpy.data.objects.new("Proof camera",camera)
scene.collection.objects.link(cam)
scene.camera=cam
camera.lens=24
views=[("01_Bedroom_Blender",(-11.92,-2.65,1.58),(-14.25,.65,1.16)),
       ("02_Garden_Blender",(-10,6.8,1.65),(-13.7,1.6,1.3)),
       ("03_Living_Blender",(-11.0,.8,1.58),(-8.3,-1.8,1.05))]
if options.render:
    for name,location,target in views:
        # Camera adaptation between interior and exterior, not a horror color filter.
        scene.view_settings.exposure=-1.8 if name.startswith("02_") else 1.3
        cam.location=location
        cam.rotation_euler=(Vector(target)-cam.location).to_track_quat("-Z","Y").to_euler()
        if options.oidn:
            scene.render.image_settings.file_format="OPEN_EXR"
            scene.render.image_settings.color_depth="32"
            scene.render.filepath=str(TEMP/(name+".exr"))
        else:
            scene.render.filepath=str(RENDERS/(name+".jpg"))
        bpy.ops.render.render(write_still=True)
        if options.oidn:
            finish_render(TEMP/(name+".exr"),RENDERS/(name+".jpg"),RENDERS/(name+"_raw.jpg"),options.oidn,scene)
proof={"renderer":bpy.app.version_string+" / Cycles CPU", "samples":options.samples,
       "unreal":False,"ai_generated_images":False,"rendered":options.render,
       "cameras":[{"file":n+".jpg","position_m":p,"target_m":t,"exposure_stops":-1.8 if n.startswith("02_") else 1.3} for n,p,t in views],
       "script_sha256":SCRIPT_HASH,"denoising":"Intel Open Image Denoise, color-only; raw render retained" if options.oidn else "none",
       "render_sha256":{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(RENDERS.glob('*.jpg'))},
       "scene_manifest_sha256":hashlib.sha256((ART/"scene.manifest.json").read_bytes()).hexdigest()}
(ROOT/"Docs/Quality02/Validation/blender-build.json").write_text(json.dumps(proof,indent=2)+"\n")
bpy.ops.wm.save_as_mainfile(filepath=str(TEMP/"EndlessSummer_Quality02.blend"))
print("QUALITY02_SCENE_EXPORTED",len(assets),"Unreal import NOT RUN")
