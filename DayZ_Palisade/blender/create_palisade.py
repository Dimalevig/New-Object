# -----------------------------------------------------------------------------
#  DayZ Palisade - Blender model generator
#
#  Creates two models ready for export with Arma Toolbox (P3D exporter):
#    * collection "palisade"          -> palisade.p3d
#    * collection "palisade_placing"  -> palisade_placing.p3d (hologram)
#
#  Usage: Blender -> Scripting -> Open this file -> Run Script.
#  Tested API: Blender 2.93 - 4.x
#
#  Axes: Blender Z = up, Blender +Y = model front (Arma +Z after export),
#  wall runs along X, origin = ground level, centre of the wall.
#  The builder stands on the -Y (inner) side, where the cross beams are.
# -----------------------------------------------------------------------------
import bpy
import bmesh
import math
import random
from mathutils import Matrix, Vector

# ----------------------------------------------------------------- SETTINGS --
WIDTH        = 4.0     # wall length (m)
LOG_RADIUS   = 0.11    # vertical log radius
HEIGHT       = 2.6     # vertical log height above ground
HEIGHT_RAND  = 0.08    # random +- height variation (rustic look)
BURY         = 0.15    # how deep logs go under ground (visual LODs only)
SPIKE_HEIGHT = 0.35    # sharpened top
SILL_RADIUS  = 0.13    # ground logs of the base
BEAM_RADIUS  = 0.08    # horizontal cross beams
BEAM_LOW_Z   = 0.75
BEAM_HIGH_Z  = 1.95
SEED         = 1337

TEXTURE = r"PalisadeMod\data\palisade_co.paa"
RVMAT   = r"PalisadeMod\data\palisade.rvmat"

# Arma Toolbox LOD ids (armaObjProps.lod). Change here if your add-on version differs.
ARMA_LOD = {
    "RES":       "-1.0",        # resolution LOD, value from lodDistance
    "SHADOW":    "1.000e+4",    # ShadowVolume 0
    "GEOMETRY":  "1.000e+13",
    "MEMORY":    "1.000e+15",
    "VIEW_GEO":  "6.000e+15",
    "FIRE_GEO":  "7.000e+15",
}

# construction parts (must match config.cpp / model.cfg)
P_BASE, P_DOWN, P_UP, P_SPIKES = "base", "wall_down", "wall_up", "spikes"


# ------------------------------------------------------------------ HELPERS --
def _cone(bm, segs, r1, r2, depth, matrix):
    kw = dict(cap_ends=True, cap_tris=False, segments=segs, depth=depth,
              matrix=matrix, calc_uvs=True)
    try:
        res = bmesh.ops.create_cone(bm, radius1=r1, radius2=r2, **kw)
    except TypeError:   # Blender < 3.0
        res = bmesh.ops.create_cone(bm, diameter1=r1, diameter2=r2, **kw)
    verts = res["verts"]
    if r2 == 0.0:
        bmesh.ops.remove_doubles(bm, verts=verts, dist=1e-5)
    return [v for v in verts if v.is_valid]


def vertical_log(bm, segs, x, y, z0, z1, r, rot):
    h = z1 - z0
    m = Matrix.Translation((x, y, z0 + h / 2)) @ Matrix.Rotation(rot, 4, "Z")
    return _cone(bm, segs, r, r, h, m)


def horizontal_log(bm, segs, y, z, length, r):
    m = Matrix.Translation((0, y, z)) @ Matrix.Rotation(math.radians(90), 4, "Y")
    return _cone(bm, segs, r, r, length, m)


def spike(bm, segs, x, y, z, r, h, rot):
    m = Matrix.Translation((x, y, z + h / 2)) @ Matrix.Rotation(rot, 4, "Z")
    return _cone(bm, segs, r, 0.0, h, m)


def box(bm, vmin, vmax):
    vmin, vmax = Vector(vmin), Vector(vmax)
    size = vmax - vmin
    centre = (vmin + vmax) / 2
    m = Matrix.Translation(centre) @ Matrix.Diagonal((size.x, size.y, size.z, 1.0))
    return bmesh.ops.create_cube(bm, size=1.0, matrix=m, calc_uvs=True)["verts"]


class Lod:
    """One LOD = one Blender object; named selections = vertex groups."""

    def __init__(self, name):
        self.name = name
        self.bm = bmesh.new()
        self.bm.loops.layers.uv.verify()
        self.deform = self.bm.verts.layers.deform.verify()
        self.groups = []
        self.components = 0

    def _gid(self, name):
        if name not in self.groups:
            self.groups.append(name)
        return self.groups.index(name)

    def add(self, verts, selections=(), smooth=False, component=False, shading=True):
        names = list(selections)
        if component:
            self.components += 1
            names.append("Component%02d" % self.components)
        for v in verts:
            for n in names:
                v[self.deform][self._gid(n)] = 1.0
        if shading:
            for f in {f for v in verts for f in v.link_faces}:
                f.smooth = smooth and len(f.verts) <= 4

    def point(self, name, co):
        v = self.bm.verts.new(co)
        v[self.deform][self._gid(name)] = 1.0

    def build(self, collection, lod, resolution=0.0, material=None,
              triangulate=False, props=None):
        if triangulate:
            bmesh.ops.triangulate(self.bm, faces=self.bm.faces[:])
        mesh = bpy.data.meshes.new(self.name)
        self.bm.to_mesh(mesh)
        self.bm.free()
        obj = bpy.data.objects.new(self.name, mesh)
        for g in self.groups:               # same order as indices in bmesh
            obj.vertex_groups.new(name=g)
        if material:
            mesh.materials.append(material)
        collection.objects.link(obj)
        set_arma_lod(obj, lod, resolution, props or {})
        return obj


def set_arma_lod(obj, lod, resolution, props):
    obj["dayz_lod"] = "%s %s" % (lod, resolution)    # readable hint for manual setup
    p = getattr(obj, "armaObjProps", None)
    if p is None:
        return
    try:
        p.isArmaObject = True
        p.lod = ARMA_LOD[lod]
        p.lodDistance = resolution
        for k, v in props.items():
            np = p.namedProps.add()
            np.name, np.value = k, v
    except Exception as e:
        print("[palisade] Arma Toolbox props on %s failed: %s" % (obj.name, e))


def make_material():
    mat = bpy.data.materials.get("palisade_wood") or bpy.data.materials.new("palisade_wood")
    mat.diffuse_color = (0.33, 0.22, 0.13, 1.0)
    mp = getattr(mat, "armaMatProps", None)
    if mp is not None:
        try:
            mp.texType = "Texture"
            mp.texture = TEXTURE
            mp.rvMat = RVMAT
        except Exception as e:
            print("[palisade] Arma Toolbox material props failed:", e)
    return mat


def new_collection(name):
    old = bpy.data.collections.get(name)
    if old:
        for o in list(old.objects):
            bpy.data.objects.remove(o, do_unlink=True)
        bpy.data.collections.remove(old)
    col = bpy.data.collections.new(name)
    bpy.context.scene.collection.children.link(col)
    return col


# ------------------------------------------------------------------- LAYOUT --
def make_layout():
    rnd = random.Random(SEED)
    count = int(WIDTH // (LOG_RADIUS * 2))
    step = WIDTH / count
    logs = []
    for i in range(count):
        logs.append(dict(
            x=-WIDTH / 2 + step * (i + 0.5),
            top=HEIGHT + rnd.uniform(-HEIGHT_RAND, HEIGHT_RAND),
            rot=rnd.uniform(0, math.tau),
            r=LOG_RADIUS * rnd.uniform(0.9, 1.05),
            part=P_DOWN if i % 2 == 0 else P_UP,
        ))
    return logs


SILL_Y = LOG_RADIUS + SILL_RADIUS               # sills on both sides of the logs
BEAM_Y = -(LOG_RADIUS + BEAM_RADIUS)            # cross beams on the inner side
BEAM_LEN = WIDTH + 0.2


def visual_lod(name, logs, segs, material, z_bottom):
    """Resolution / shadow LOD geometry."""
    lod = Lod(name)
    # base: two ground logs
    for y in (SILL_Y, -SILL_Y):
        lod.add(horizontal_log(lod.bm, segs, y, SILL_RADIUS * 0.8, BEAM_LEN, SILL_RADIUS),
                [P_BASE], smooth=True)
    # vertical logs
    for l in logs:
        lod.add(vertical_log(lod.bm, segs, l["x"], 0, z_bottom, l["top"], l["r"], l["rot"]),
                [l["part"]], smooth=True)
        lod.add(spike(lod.bm, segs, l["x"], 0, l["top"], l["r"], SPIKE_HEIGHT, l["rot"]),
                [P_SPIKES], smooth=True)
    # cross beams
    lod.add(horizontal_log(lod.bm, segs, BEAM_Y, BEAM_LOW_Z, BEAM_LEN, BEAM_RADIUS),
            [P_DOWN], smooth=True)
    lod.add(horizontal_log(lod.bm, segs, BEAM_Y, BEAM_HIGH_Z, BEAM_LEN, BEAM_RADIUS),
            [P_UP], smooth=True)
    return lod


def collision_lod(name, logs, with_spikes):
    """Geometry / View / Fire geometry: closed convex components only."""
    lod = Lod(name)
    s = SILL_RADIUS
    for y in (SILL_Y, -SILL_Y):
        lod.add(box(lod.bm, (-BEAM_LEN / 2, y - s, 0), (BEAM_LEN / 2, y + s, s * 1.6)),
                [P_BASE], component=True)
    for l in logs:
        r = l["r"]
        lod.add(box(lod.bm, (l["x"] - r, -r, 0), (l["x"] + r, r, l["top"])),
                [l["part"]], component=True)
        if with_spikes:
            lod.add(spike(lod.bm, 4, l["x"], 0, l["top"], r, SPIKE_HEIGHT, math.radians(45)),
                    [P_SPIKES], component=True)
    for z, part in ((BEAM_LOW_Z, P_DOWN), (BEAM_HIGH_Z, P_UP)):
        b = BEAM_RADIUS
        lod.add(box(lod.bm, (-BEAM_LEN / 2, BEAM_Y - b, z - b), (BEAM_LEN / 2, BEAM_Y + b, z + b)),
                [part], component=True)
    return lod


def memory_lod(name):
    lod = Lod(name)
    half = BEAM_LEN / 2
    depth = SILL_Y + SILL_RADIUS
    top = HEIGHT + HEIGHT_RAND
    # action points (HasProperDistance)
    lod.point(P_BASE, (0, 0, 0.3))
    lod.point(P_DOWN, (0, 0, 1.0))
    lod.point(P_UP, (0, 0, 1.8))
    lod.point(P_SPIKES, (0, 0, top))
    # construction collision boxes (collision_data in config.cpp)
    lod.point("wall_down_min", (-half, -depth, 0))
    lod.point("wall_down_max", (half, depth, top))
    lod.point("wall_up_min", (-half, -depth, 0))
    lod.point("wall_up_max", (half, depth, top))
    # kit drop position when the base is dismantled (inner side)
    lod.point("kit_spawn_position", (0, -1.0, 0))
    return lod


# --------------------------------------------------------------------- MAIN --
def build():
    mat = make_material()
    logs = make_layout()
    geo_props = {"autocenter": "0"}

    # ---------------- palisade.p3d
    col = new_collection("palisade")
    visual_lod("palisade_res0", logs, 12, mat, -BURY).build(col, "RES", 0.0, mat)
    visual_lod("palisade_res1", logs, 8, mat, -BURY).build(col, "RES", 1.0, mat)
    visual_lod("palisade_res2", logs, 5, mat, 0.0).build(col, "RES", 2.0, mat)
    visual_lod("palisade_shadow", logs, 5, None, 0.0).build(col, "SHADOW", 0.0, triangulate=True)
    collision_lod("palisade_geometry", logs, False).build(col, "GEOMETRY", props=geo_props)
    collision_lod("palisade_viewgeo", logs, False).build(col, "VIEW_GEO")
    collision_lod("palisade_firegeo", logs, True).build(col, "FIRE_GEO")
    memory_lod("palisade_memory").build(col, "MEMORY")

    # ---------------- palisade_placing.p3d (hologram)
    col_p = new_collection("palisade_placing")
    holo = visual_lod("palisade_placing_res0", logs, 8, mat, 0.0)
    holo.add(list(holo.bm.verts), ["placing"], shading=False)
    holo.build(col_p, "RES", 0.0, mat)
    g = Lod("palisade_placing_geometry")
    g.add(box(g.bm, (-BEAM_LEN / 2, -SILL_Y - SILL_RADIUS, 0),
              (BEAM_LEN / 2, SILL_Y + SILL_RADIUS, HEIGHT)), ["placing"], component=True)
    g.build(col_p, "GEOMETRY", props={"autocenter": "0"})

    print("[palisade] done: %d logs" % len(logs))


build()
