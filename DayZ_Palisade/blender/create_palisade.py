# -----------------------------------------------------------------------------
#  DayZ Palisade - Blender model generator
#
#  Creates two models ready for export with Arma Toolbox (P3D exporter):
#    * collection "palisade"          -> palisade.p3d
#    * collection "palisade_placing"  -> palisade_placing.p3d (hologram)
#
#  Look: rough vertical logs sharpened with an axe, three rows of flat boards
#  nailed on the inner side, ends tied with wire, thicker corner posts.
#  Outer side: a round pole tied with wire near the top and a stretched wire.
#
#  Construction parts:
#    base      - corner posts
#    wall_down - even logs + two lower board rows
#    wall_up   - odd logs + top board row + outer pole + wire
#    spikes    - sharpened tops of the logs
#
#  Usage: Blender -> Scripting -> Open this file -> Run Script.
#  Tested API: Blender 2.93 - 5.x
#
#  Axes: Blender Z = up, Blender +Y = model front (Arma +Z after export),
#  wall runs along X, origin = ground level, centre of the wall.
#  The builder stands on the -Y (inner) side, where the boards are;
#  the pole and the stretched wire are on the +Y (outer) side.
# -----------------------------------------------------------------------------
import bpy
import bmesh
import math
import random
import traceback
from mathutils import Matrix, Vector

# ----------------------------------------------------------------- SETTINGS --
WIDTH         = 4.2     # wall length between corner posts (m)
LOG_COUNT     = 22      # vertical logs
LOG_OVERLAP   = 1.18    # logs are thicker than their spacing -> no gaps between them
LOG_RADIUS    = WIDTH / LOG_COUNT / 2 * LOG_OVERLAP   # derived average radius
LOG_TAPER     = 0.97    # top radius = bottom radius * taper
HEIGHT        = 3.0     # log height above ground (without spike)
HEIGHT_RAND   = 0.15    # random +- height
TILT_DEG      = 1.2     # random lean of each log (front/back only, so no gaps open)
BARK_JITTER   = 0.05    # radial noise of the bark (fraction of radius)
SPIKE_HEIGHT  = (0.30, 0.48)   # min/max sharpened tip height
SPIKE_SEGS    = 6       # axe facets on the tip
BURY          = 0.15    # how deep logs go under ground (visual LODs only)

POST_RADIUS   = 0.12    # corner posts (part "base")
POST_EXTRA    = 0.10    # corner posts are a bit taller

PLANK_ROWS    = (0.55, 1.65, 2.60)   # board centre heights
PLANK_HEIGHT  = 0.22
PLANK_THICK   = 0.05
PLANK_TILT    = 1.5     # random board rotation (deg)

POLE_RADIUS   = 0.07    # outer horizontal pole
POLE_Z        = 2.45
WIRE_LINE_Z   = 1.95    # straight wire stretched along the outer side
TIE_STEP      = 0.9     # wire ties of the pole every N metres

WIRE_RADIUS   = 0.006
WIRE_LOOPS    = 4       # loops per wire binding

SEED          = 1337

# texture paths written into the P3D (Arma Toolbox material props)
MATERIALS = {
    "logs":   (r"PalisadeMod\data\palisade_logs_co.paa",   r"PalisadeMod\data\palisade_logs.rvmat"),
    "planks": (r"PalisadeMod\data\palisade_planks_co.paa", r"PalisadeMod\data\palisade_planks.rvmat"),
    "wire":   (r"PalisadeMod\data\palisade_wire_co.paa",   r"PalisadeMod\data\palisade_wire.rvmat"),
}
M_LOGS, M_PLANKS, M_WIRE = 0, 1, 2     # material slot indices

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
ROW_PARTS = (P_DOWN, P_DOWN, P_UP)     # which part every board row belongs to
UP = Vector((0, 0, 1))


# ------------------------------------------------------------ MESH BUILDING --
class Lod:
    """One LOD = one Blender object; named selections = vertex groups."""

    def __init__(self, name):
        self.name = name
        self.bm = bmesh.new()
        self.uv = self.bm.loops.layers.uv.verify()
        self.deform = self.bm.verts.layers.deform.verify()
        self.groups = []
        self.components = 0

    def _gid(self, name):
        if name not in self.groups:
            self.groups.append(name)
        return self.groups.index(name)

    def tag(self, verts, selections=(), component=False):
        names = list(selections)
        if component:
            self.components += 1
            names.append("Component%02d" % self.components)
        for v in verts:
            for n in names:
                v[self.deform][self._gid(n)] = 1.0

    def face(self, verts, uvs, mat=0, smooth=False):
        f = self.bm.faces.new(verts)
        f.material_index = mat
        f.smooth = smooth
        for loop, uv in zip(f.loops, uvs):
            loop[self.uv].uv = uv
        return f

    def point(self, name, co):
        v = self.bm.verts.new(co)
        v[self.deform][self._gid(name)] = 1.0

    def build(self, collection, lod, resolution=0.0, materials=(),
              triangulate=False, props=None):
        if triangulate:
            bmesh.ops.triangulate(self.bm, faces=self.bm.faces[:])
        mesh = bpy.data.meshes.new(self.name)
        self.bm.to_mesh(mesh)
        self.bm.free()
        obj = bpy.data.objects.new(self.name, mesh)
        for g in self.groups:               # same order as indices in bmesh
            obj.vertex_groups.new(name=g)
        for m in materials:
            mesh.materials.append(m)
        collection.objects.link(obj)
        set_arma_lod(obj, lod, resolution, props or {})
        return obj


def _basis(axis, rot):
    return UP.rotation_difference(axis).to_matrix() @ Matrix.Rotation(rot, 3, "Z")


def ring(lod, centre, radius, segs, basis, noise=None):
    out = []
    for j in range(segs):
        a = math.tau * j / segs
        r = radius * (1.0 + (noise[j] if noise else 0.0))
        out.append(lod.bm.verts.new(centre + basis @ Vector((math.cos(a) * r, math.sin(a) * r, 0))))
    return out


def cap(lod, verts, flip, mat):
    vs = list(reversed(verts)) if flip else list(verts)
    c = sum((v.co for v in vs), Vector()) / len(vs)
    uvs = [((v.co.x - c.x) * 2 + 0.5, (v.co.y - c.y) * 2 + 0.5) for v in vs]
    lod.face(vs, uvs, mat)


def tube(lod, rings, vs, segs, mat, smooth, u_scale=1.0):
    """Quads between consecutive rings. UV: u around, v along the log."""
    for k in range(len(rings) - 1):
        a, b = rings[k], rings[k + 1]
        for j in range(segs):
            j2 = (j + 1) % segs
            u0, u1 = j / segs * u_scale, (j + 1) / segs * u_scale
            lod.face([a[j], a[j2], b[j2], b[j]],
                     [(u0, vs[k]), (u1, vs[k]), (u1, vs[k + 1]), (u0, vs[k + 1])],
                     mat, smooth)


def log_ends(l, bury=0.0):
    """Bottom (optionally extended under ground) and top point of a log."""
    start = l["p0"] - l["axis"] * (bury / max(l["axis"].z, 0.2))
    return start, l["p0"] + l["axis"] * l["len"]


def make_log(lod, l, segs, rings, bury, bark, rnd):
    """Rough log along l["axis"]. Returns (verts, top centre, top radius)."""
    basis = _basis(l["axis"], l["rot"])
    start, top = log_ends(l, bury)
    length = (top - start).length
    rs, vs, verts = [], [], []
    for k in range(rings + 1):
        t = k / rings
        r = l["r"] * (1 - (1 - LOG_TAPER) * t)
        noise = [rnd.uniform(-bark, bark) for _ in range(segs)] if bark else None
        rs.append(ring(lod, start + (top - start) * t, r, segs, basis, noise))
        vs.append(length * t)
        verts += rs[-1]
    tube(lod, rs, vs, segs, M_LOGS, True, u_scale=2 * math.pi * l["r"])
    cap(lod, rs[0], True, M_LOGS)
    cap(lod, rs[-1], False, M_PLANKS)
    return verts, top, l["r"] * LOG_TAPER


def make_spike(lod, base, radius, l, segs):
    """Axe-cut tip: low-poly cone with an off-centre apex, flat shaded."""
    basis = _basis(l["axis"], l["rot"] + 0.3)
    base_ring = ring(lod, base, radius * 1.02, segs, basis)
    apex = lod.bm.verts.new(base + basis @ Vector((l["tip_off"].x, l["tip_off"].y, l["spike"])))
    for j in range(segs):
        j2 = (j + 1) % segs
        lod.face([base_ring[j], base_ring[j2], apex],
                 [(j / segs, 0), ((j + 1) / segs, 0), ((j + 0.5) / segs, 0.6)], M_PLANKS)
    cap(lod, base_ring, True, M_PLANKS)
    return base_ring + [apex]


def make_rod(lod, p0, p1, r, segs, mat):
    """Plain cylinder between two points (pole, stretched wire)."""
    axis = (p1 - p0).normalized()
    basis = _basis(axis, 0.0)
    a, b = ring(lod, p0, r, segs, basis), ring(lod, p1, r, segs, basis)
    tube(lod, [a, b], [0.0, (p1 - p0).length], segs, mat, True, u_scale=2 * math.pi * r)
    cap(lod, a, True, mat)
    cap(lod, b, False, mat)
    return a + b


def make_box(lod, centre, size, rot_y=0.0, rot_z=0.0, mat=M_PLANKS):
    m = (Matrix.Translation(centre) @ Matrix.Rotation(rot_z, 4, "Z")
         @ Matrix.Rotation(rot_y, 4, "Y") @ Matrix.Diagonal((size[0], size[1], size[2], 1.0)))
    verts = bmesh.ops.create_cube(lod.bm, size=1.0, matrix=m, calc_uvs=True)["verts"]
    faces = {f for v in verts for f in v.link_faces}
    for f in faces:
        f.material_index = mat
        f.smooth = False
        for loop in f.loops:        # planar UV along the board (wood grain along X)
            co = loop.vert.co
            n = f.normal
            loop[lod.uv].uv = (co.x, co.z if abs(n.z) < 0.5 else co.y)
    return verts


def make_wire(lod, centre, a, b, segs=16, tube_segs=4):
    """Several loops of wire around an elliptic outline (a = half X, b = half Y)."""
    verts = []
    for i in range(WIRE_LOOPS):
        c = centre + Vector((0, 0, (i - (WIRE_LOOPS - 1) / 2) * WIRE_RADIUS * 3.5))
        rings_ = []
        for s in range(segs):
            t = math.tau * s / segs
            p = c + Vector((math.cos(t) * a, math.sin(t) * b, math.sin(t * 2 + i) * 0.004))
            n = Vector((math.cos(t) * b, math.sin(t) * a, 0)).normalized()
            rr = []
            for k in range(tube_segs):
                w = math.tau * k / tube_segs
                rr.append(lod.bm.verts.new(p + n * math.cos(w) * WIRE_RADIUS
                                           + Vector((0, 0, math.sin(w) * WIRE_RADIUS))))
            rings_.append(rr)
            verts += rr
        for s in range(segs):
            r0, r1 = rings_[s], rings_[(s + 1) % segs]
            for k in range(tube_segs):
                k2 = (k + 1) % tube_segs
                lod.face([r0[k], r0[k2], r1[k2], r1[k]],
                         [(s / segs, k / tube_segs), (s / segs, (k + 1) / tube_segs),
                          ((s + 1) / segs, (k + 1) / tube_segs), ((s + 1) / segs, k / tube_segs)],
                         M_WIRE, True)
    return verts


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


# ---------------------------------------------------------------- MATERIALS --
def _preview_nodes(mat, kind):
    """Procedural look for the Blender viewport only - the game uses the .paa textures."""
    try:
        mat.use_nodes = True
        nt = mat.node_tree
        bsdf = next(n for n in nt.nodes if n.type == "BSDF_PRINCIPLED")
        coord = nt.nodes.new("ShaderNodeTexCoord")
        mapping = nt.nodes.new("ShaderNodeMapping")
        noise = nt.nodes.new("ShaderNodeTexNoise")
        ramp = nt.nodes.new("ShaderNodeValToRGB")
        bump = nt.nodes.new("ShaderNodeBump")
        nt.links.new(coord.outputs["Object"], mapping.inputs["Vector"])
        nt.links.new(mapping.outputs["Vector"], noise.inputs["Vector"])
        nt.links.new(noise.outputs["Fac"], ramp.inputs["Fac"])
        nt.links.new(ramp.outputs["Color"], bsdf.inputs["Base Color"])
        nt.links.new(noise.outputs["Fac"], bump.inputs["Height"])
        nt.links.new(bump.outputs["Normal"], bsdf.inputs["Normal"])
        noise.inputs["Detail"].default_value = 8.0
        el = ramp.color_ramp.elements
        if kind == "logs":        # dark bark, vertical streaks
            mapping.inputs["Scale"].default_value = (40, 40, 3)
            noise.inputs["Scale"].default_value = 3.0
            el[0].color, el[1].color = (0.03, 0.02, 0.012, 1), (0.22, 0.14, 0.07, 1)
            bump.inputs["Strength"].default_value = 0.6
            bsdf.inputs["Roughness"].default_value = 0.9
        elif kind == "planks":    # sawn wood, grain along X
            mapping.inputs["Scale"].default_value = (2, 30, 30)
            noise.inputs["Scale"].default_value = 2.5
            el[0].color, el[1].color = (0.22, 0.15, 0.09, 1), (0.55, 0.42, 0.28, 1)
            bump.inputs["Strength"].default_value = 0.3
            bsdf.inputs["Roughness"].default_value = 0.8
        else:                     # rusty wire
            noise.inputs["Scale"].default_value = 40.0
            el[0].color, el[1].color = (0.25, 0.25, 0.25, 1), (0.45, 0.35, 0.28, 1)
            bsdf.inputs["Metallic"].default_value = 0.8
            bsdf.inputs["Roughness"].default_value = 0.5
    except Exception as e:
        print("[palisade] preview material %s skipped: %s" % (kind, e))


def make_materials():
    mats = []
    colors = {"logs": (0.2, 0.14, 0.09, 1), "planks": (0.45, 0.34, 0.22, 1), "wire": (0.35, 0.35, 0.35, 1)}
    for kind in ("logs", "planks", "wire"):
        name = "palisade_" + kind
        old = bpy.data.materials.get(name)
        if old:
            bpy.data.materials.remove(old)
        mat = bpy.data.materials.new(name)
        mat.diffuse_color = colors[kind]
        _preview_nodes(mat, kind)
        mp = getattr(mat, "armaMatProps", None)
        if mp is not None:
            try:
                mp.texType = "Texture"
                mp.texture, mp.rvMat = MATERIALS[kind]
            except Exception as e:
                print("[palisade] Arma Toolbox material props failed:", e)
        mats.append(mat)
    return mats


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
    step = WIDTH / LOG_COUNT
    tilt = math.tan(math.radians(TILT_DEG))

    def stake(p0, axis, length, r, part):
        return dict(
            p0=p0, axis=axis.normalized(), len=length, r=r, part=part,
            rot=rnd.uniform(0, math.tau),
            spike=rnd.uniform(*SPIKE_HEIGHT),
            tip_off=Vector((rnd.uniform(-0.03, 0.03), rnd.uniform(-0.03, 0.03), 0)),
        )

    logs = []
    for i in range(LOG_COUNT):
        x = -WIDTH / 2 + step * (i + 0.5)
        lean = Vector((rnd.uniform(-tilt, tilt) * 0.1, rnd.uniform(-tilt, tilt), 1))
        logs.append(stake(Vector((x, 0, 0)), lean,
                          HEIGHT + rnd.uniform(-HEIGHT_RAND, HEIGHT_RAND),
                          LOG_RADIUS * rnd.uniform(0.97, 1.06),
                          P_DOWN if i % 2 == 0 else P_UP))
    posts = [stake(Vector((side * (WIDTH / 2 + POST_RADIUS), 0, 0)), UP,
                   HEIGHT + POST_EXTRA, POST_RADIUS, P_BASE) for side in (-1, 1)]
    left, right = posts[0]["p0"].x, posts[1]["p0"].x

    # inner side: two boards per row, joint near the middle
    boards = []
    plank_y = -(LOG_RADIUS * 1.1 + PLANK_THICK / 2)
    for row, z in enumerate(PLANK_ROWS):
        joint = rnd.uniform(-0.3, 0.3)
        x0s = ((left - POST_RADIUS * 0.5, joint - 0.01), (joint + 0.01, right + POST_RADIUS * 0.5))
        for x0, x1 in x0s:
            boards.append(dict(
                centre=Vector(((x0 + x1) / 2, plank_y, z + rnd.uniform(-0.04, 0.04))),
                size=(x1 - x0, PLANK_THICK, PLANK_HEIGHT * rnd.uniform(0.9, 1.05)),
                rot=math.radians(rnd.uniform(-PLANK_TILT, PLANK_TILT)),
                part=ROW_PARTS[row]))

    # wire bindings: corner posts + boards (inner), pole + logs (outer)
    wires = []
    for p in posts:
        for z in PLANK_ROWS:
            y0, y1 = plank_y - PLANK_THICK / 2, POST_RADIUS
            wires.append(dict(centre=Vector((p["p0"].x, (y0 + y1) / 2, z)),
                              a=POST_RADIUS + 0.015, b=(y1 - y0) / 2 + 0.015))
    pole_y = LOG_RADIUS * 1.1 + POLE_RADIUS
    n_ties = max(2, int((right - left) / TIE_STEP) + 1)
    for i in range(n_ties):
        x = left + (right - left) * i / (n_ties - 1)
        y0, y1 = -LOG_RADIUS, pole_y + POLE_RADIUS
        wires.append(dict(centre=Vector((x, (y0 + y1) / 2, POLE_Z)),
                          a=LOG_RADIUS + 0.015, b=(y1 - y0) / 2 + 0.015))

    pole = dict(p0=Vector((left - 0.15, pole_y, POLE_Z)), p1=Vector((right + 0.15, pole_y, POLE_Z + 0.03)))
    wire_line = dict(p0=Vector((left, LOG_RADIUS * 1.1 + WIRE_RADIUS, WIRE_LINE_Z)),
                     p1=Vector((right, LOG_RADIUS * 1.1 + WIRE_RADIUS, WIRE_LINE_Z - 0.04)))

    return dict(logs=logs, posts=posts, boards=boards, wires=wires,
                pole=pole, wire_line=wire_line)


def bounds(lay):
    half = lay["posts"][1]["p0"].x + POST_RADIUS + 0.15
    front = LOG_RADIUS * 1.1 + POLE_RADIUS * 2 + 0.03
    back = LOG_RADIUS * 1.1 + PLANK_THICK + 0.03
    top = max(log_ends(l)[1].z + l["spike"] for l in lay["logs"] + lay["posts"])
    return half, front, back, top


# --------------------------------------------------------------------- LODS --
def tip_part(l):
    """Corner posts are sharpened together with the base, logs get the "spikes" part."""
    return P_BASE if l["part"] == P_BASE else P_SPIKES


def visual_lod(name, lay, segs, rings, bark, bury, wire=True, spike_segs=SPIKE_SEGS):
    rnd = random.Random(SEED + segs)
    lod = Lod(name)
    for l in lay["posts"] + lay["logs"]:
        verts, top, r = make_log(lod, l, segs, rings, bury, bark, rnd)
        lod.tag(verts, [l["part"]])
        lod.tag(make_spike(lod, top, r, l, spike_segs), [tip_part(l)])
    for b in lay["boards"]:
        lod.tag(make_box(lod, b["centre"], b["size"], rot_y=b["rot"]), [b["part"]])
    p = lay["pole"]
    lod.tag(make_rod(lod, p["p0"], p["p1"], POLE_RADIUS, max(5, segs - 2), M_LOGS), [P_UP])
    if wire:
        w = lay["wire_line"]
        lod.tag(make_rod(lod, w["p0"], w["p1"], WIRE_RADIUS, 4, M_WIRE), [P_UP])
        for w in lay["wires"]:
            lod.tag(make_wire(lod, w["centre"], w["a"], w["b"]), [P_UP])
    return lod


def _aabb(points, r):
    lo = Vector((min(p.x for p in points) - r, min(p.y for p in points) - r, max(0.0, min(p.z for p in points) - r)))
    hi = Vector((max(p.x for p in points) + r, max(p.y for p in points) + r, max(p.z for p in points) + r))
    return lo, hi


def collision_lod(name, lay, with_tips):
    """Geometry / View / Fire geometry: closed convex components only."""
    lod = Lod(name)

    def add_box(lo, hi, part):
        lod.tag(make_box(lod, (lo + hi) / 2, hi - lo), [part], component=True)

    for l in lay["posts"] + lay["logs"]:
        p0, top = log_ends(l)
        add_box(*_aabb([p0, top], l["r"]), l["part"])
        if with_tips:
            fake = dict(l, rot=math.radians(45), tip_off=Vector((0, 0, 0)))
            lod.tag(make_spike(lod, top, l["r"] * LOG_TAPER, fake, 4), [tip_part(l)], component=True)
    for b in lay["boards"]:
        c, s = b["centre"], b["size"]
        h = s[2] / 2 + abs(math.sin(b["rot"])) * s[0] / 2      # cover the tilt
        add_box(Vector((c.x - s[0] / 2, c.y - s[1] / 2, c.z - h)),
                Vector((c.x + s[0] / 2, c.y + s[1] / 2, c.z + h)), b["part"])
    p = lay["pole"]
    add_box(*_aabb([p["p0"], p["p1"]], POLE_RADIUS), P_UP)
    return lod


def memory_lod(name, lay):
    half, front, back, top = bounds(lay)
    lod = Lod(name)
    # action points (HasProperDistance) - on the inner side, where the builder stands
    lod.point(P_BASE, (0, -back, 0.3))
    lod.point(P_DOWN, (0, -back, 1.0))
    lod.point(P_UP, (0, -back, 2.0))
    lod.point(P_SPIKES, (0, -back, HEIGHT))
    # construction collision boxes (collision_data in config.cpp)
    for part in (P_DOWN, P_UP):
        lod.point(part + "_min", (-half, -back, 0))
        lod.point(part + "_max", (half, front, top))
    # kit drop position when the base is dismantled (inner side)
    lod.point("kit_spawn_position", (0, -1.0, 0))
    return lod


# --------------------------------------------------------------------- MAIN --
def build():
    mats = make_materials()
    layout = make_layout()
    geo_props = {"autocenter": "0"}

    # ---------------- palisade.p3d
    col = new_collection("palisade")
    visual_lod("palisade_res0", layout, 10, 6, BARK_JITTER, BURY).build(col, "RES", 0.0, mats)
    visual_lod("palisade_res1", layout, 7, 3, BARK_JITTER * 0.6, BURY).build(col, "RES", 1.0, mats)
    visual_lod("palisade_res2", layout, 5, 1, 0.0, 0.0, wire=False, spike_segs=4).build(col, "RES", 2.0, mats)
    visual_lod("palisade_shadow", layout, 5, 1, 0.0, 0.0, wire=False, spike_segs=4).build(
        col, "SHADOW", 0.0, triangulate=True)
    collision_lod("palisade_geometry", layout, False).build(col, "GEOMETRY", props=geo_props)
    collision_lod("palisade_viewgeo", layout, False).build(col, "VIEW_GEO")
    collision_lod("palisade_firegeo", layout, True).build(col, "FIRE_GEO")
    memory_lod("palisade_memory", layout).build(col, "MEMORY")

    # ---------------- palisade_placing.p3d (hologram)
    col_p = new_collection("palisade_placing")
    holo = visual_lod("palisade_placing_res0", layout, 6, 1, 0.0, 0.0, wire=False, spike_segs=4)
    holo.tag(list(holo.bm.verts), ["placing"])
    holo.build(col_p, "RES", 0.0, mats)
    half, front, back, top = bounds(layout)
    g = Lod("palisade_placing_geometry")
    g.tag(make_box(g, Vector((0, (front - back) / 2, HEIGHT / 2)),
                   (half * 2, front + back, HEIGHT)), ["placing"], component=True)
    g.build(col_p, "GEOMETRY", props={"autocenter": "0"})

    print("[palisade] done: %d logs, %d boards" % (len(layout["logs"]), len(layout["boards"])))


def _popup(lines, icon):
    """Visible feedback in the Blender UI (the Python console does not show script output)."""
    if bpy.app.background or not bpy.context.window_manager.windows:
        return
    def draw(self, context):
        for line in lines:
            self.layout.label(text=line)
    try:
        bpy.context.window_manager.popup_menu(draw, title="Palisade", icon=icon)
    except Exception:
        pass


print("[palisade] start")
try:
    build()
    _popup(["Done: collections 'palisade' and 'palisade_placing' created.",
            "See the Outliner (top right)."], "INFO")
except Exception:
    traceback.print_exc()
    _popup(["Error - details in Window > Toggle System Console:"]
           + traceback.format_exc().strip().splitlines()[-3:], "ERROR")
    raise
# ---- END OF SCRIPT (if you don't see this line in Blender, the script was not copied completely) ----
