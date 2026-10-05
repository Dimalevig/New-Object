"""Custom Palisade - log palisade wall model generator for Blender.

HOW TO USE
  1. Blender 4.2 or newer.
  2. Install and enable the free add-on "Arma 3 Object Builder" (by MrClock):
     https://github.com/MrClock8163/Arma3ObjectBuilder
  3. Blender > Scripting tab > Open this file > Run Script.
  4. The model appears in the scene. Files are written to OUTPUT_DIR
     (default: folder "CustomPalisade_model" in your home folder):
        cp_palisade_wall.p3d        model (all LODs)
        cp_palisade_wood_co.paa     texture for the game
        cp_palisade_wood_co.png     same texture as PNG (preview)

Command line alternative:
  blender --background --python cp_palisade_wall_blender.py -- <out_dir>

Coordinates: X = along the wall, Y = depth (-Y outside, +Y inside), Z = up,
ground at Z = 0, origin = ground centre of the wall.

Named selections prepared for the construction stages:
    cp_part_base   two end posts (stage "2 logs")
    cp_part_logs   inner posts, rails and rope lashings (stage "logs + rope")

Textures are generated procedurally by this script (no third-party assets).
"""
import math
import os
import random
import struct
import sys

import bmesh
import bpy
import numpy as np

# Output folder. Empty = <home>/CustomPalisade_model
OUTPUT_DIR = r""


def output_dir():
    return os.path.abspath(OUTPUT_DIR) if OUTPUT_DIR else os.path.join(os.path.expanduser("~"), "CustomPalisade_model")


def save_png(img, path):
    """img: float HxWx3, row 0 = top. Saved through Blender (no extra Python packages)."""
    h, w, _ = img.shape
    rgba = np.concatenate([img[::-1], np.ones((h, w, 1))], axis=2).astype(np.float32)
    name = os.path.basename(path)
    if name in bpy.data.images:
        bpy.data.images.remove(bpy.data.images[name])
    im = bpy.data.images.new(name, w, h, alpha=False)
    im.pixels.foreach_set(rgba.ravel())
    im.filepath_raw = path
    im.file_format = 'PNG'
    im.save()
    bpy.data.images.remove(im)


# ================================================================ PAA writer
def _to565(rgb):
    r = (rgb[..., 0] * 31 + 0.5).astype(np.uint32)
    g = (rgb[..., 1] * 63 + 0.5).astype(np.uint32)
    b = (rgb[..., 2] * 31 + 0.5).astype(np.uint32)
    return (r << 11) | (g << 5) | b


def _from565(c):
    r = ((c >> 11) & 31) / 31.0
    g = ((c >> 5) & 63) / 63.0
    b = (c & 31) / 31.0
    return np.stack([r, g, b], axis=-1)


def dxt1_encode(img):
    """img: float array HxWx3 in [0,1], H and W multiples of 4. Returns bytes."""
    h, w, _ = img.shape
    blocks = img.reshape(h // 4, 4, w // 4, 4, 3).transpose(0, 2, 1, 3, 4).reshape(-1, 16, 3)

    mean = blocks.mean(axis=1, keepdims=True)
    centered = blocks - mean
    cov = np.einsum("bni,bnj->bij", centered, centered)
    axis = np.ones((blocks.shape[0], 3)) / np.sqrt(3.0)
    for _ in range(8):
        axis = np.einsum("bij,bj->bi", cov, axis)
        norm = np.linalg.norm(axis, axis=1, keepdims=True)
        axis = np.where(norm > 1e-12, axis / np.maximum(norm, 1e-12), 1.0 / np.sqrt(3.0))

    proj = np.einsum("bni,bi->bn", centered, axis)
    pmin = proj.min(axis=1)
    pmax = proj.max(axis=1)
    e0 = np.clip(mean[:, 0, :] + axis * pmax[:, None], 0, 1)
    e1 = np.clip(mean[:, 0, :] + axis * pmin[:, None], 0, 1)

    c0 = _to565(e0)
    c1 = _to565(e1)
    # 4-colour mode requires c0 > c1
    swap = c0 < c1
    c0, c1 = np.where(swap, c1, c0), np.where(swap, c0, c1)
    equal = c0 == c1
    c1 = np.where(equal & (c0 > 0), c0 - 1, c1)
    c0 = np.where(equal & (c0 == 0), 1, c0)

    p0 = _from565(c0)
    p1 = _from565(c1)
    palette = np.stack([p0, p1, (2 * p0 + p1) / 3, (p0 + 2 * p1) / 3], axis=1)  # b,4,3

    dist = ((blocks[:, :, None, :] - palette[:, None, :, :]) ** 2).sum(axis=-1)  # b,16,4
    idx = dist.argmin(axis=-1).astype(np.uint32)  # b,16
    shifts = (np.arange(16, dtype=np.uint32) * 2)[None, :]
    table = (idx << shifts).sum(axis=1).astype(np.uint32)

    out = np.zeros(blocks.shape[0], dtype=[("c0", "<u2"), ("c1", "<u2"), ("t", "<u4")])
    out["c0"] = c0
    out["c1"] = c1
    out["t"] = table
    return out.tobytes()


def _downsample(img):
    h, w, c = img.shape
    return img.reshape(h // 2, 2, w // 2, 2, c).mean(axis=(1, 3))


def _tagg(name, data):
    return b"GGAT" + name[::-1].encode("ascii") + struct.pack("<I", len(data)) + data


def write_paa_dxt1(path, img):
    """img: float HxWx3 [0,1]; power-of-two sizes >= 4."""
    h, w, _ = img.shape
    assert h & (h - 1) == 0 and w & (w - 1) == 0 and h >= 4 and w >= 4

    mips = []
    level = img
    while True:
        lh, lw, _ = level.shape
        mips.append((lw, lh, dxt1_encode(level)))
        if lh <= 4 or lw <= 4:
            break
        level = _downsample(level)

    avg = img.reshape(-1, 3).mean(axis=0)
    mx = img.reshape(-1, 3).max(axis=0)

    def bgra(c):
        c = (np.clip(c, 0, 1) * 255 + 0.5).astype(np.uint8)
        return bytes([c[2], c[1], c[0], 255])

    head_wo_offs = struct.pack("<H", 0xFF01) + _tagg("AVGC", bgra(avg)) + _tagg("MAXC", bgra(mx))
    offs_size = 4 + 4 + 4 + 16 * 4
    pos = len(head_wo_offs) + offs_size + 2  # + palette ushort
    offsets = []
    for mw, mh, data in mips:
        offsets.append(pos)
        pos += 2 + 2 + 3 + len(data)
    offsets += [0] * (16 - len(offsets))

    with open(path, "wb") as f:
        f.write(head_wo_offs)
        f.write(_tagg("OFFS", struct.pack("<16I", *offsets)))
        f.write(struct.pack("<H", 0))
        for mw, mh, data in mips:
            f.write(struct.pack("<HH", mw, mh))
            f.write(struct.pack("<I", len(data))[:3])
            f.write(data)
        f.write(struct.pack("<HHH", 0, 0, 0))


# ================================================================ textures

SIZE = 1024
BARK_U = (0.0, 0.75)
WOOD_UV = (0.75, 1.0, 0.5, 1.0)   # u0, u1, v0, v1
ROPE_UV = (0.75, 1.0, 0.0, 0.5)


def _periodic_noise(h, w, scale_y, scale_x, rng):
    """Smooth noise that tiles in both directions (FFT low-pass)."""
    white = rng.standard_normal((h, w))
    fy = np.fft.fftfreq(h)[:, None] * h
    fx = np.fft.fftfreq(w)[None, :] * w
    filt = np.exp(-((fy / scale_y) ** 2 + (fx / scale_x) ** 2))
    n = np.real(np.fft.ifft2(np.fft.fft2(white) * filt))
    n -= n.mean()
    return n / (n.std() + 1e-9)


def _lerp(a, b, t):
    return a + (b - a) * t[..., None]


def bark(h, w, rng):
    # long vertical fibres: low frequency across, high along the log
    fibres = _periodic_noise(h, w, 6, 90, rng)
    patches = _periodic_noise(h, w, 3, 6, rng)
    fine = _periodic_noise(h, w, 60, 200, rng)
    cracks = _periodic_noise(h, w, 4, 140, rng)
    cracks = np.clip((cracks - 1.3) * 2.0, 0, 1)

    dark = np.array([0.20, 0.16, 0.12])
    mid = np.array([0.36, 0.30, 0.23])
    grey = np.array([0.45, 0.42, 0.37])

    t = np.clip(0.5 + 0.22 * fibres + 0.10 * fine, 0, 1)
    col = _lerp(dark, mid, t)
    col = _lerp(col, grey, np.clip(0.35 + 0.30 * patches, 0, 1) * 0.6)
    col = col * (1.0 - 0.55 * cracks[..., None])
    return np.clip(col, 0, 1)


def cut_wood(h, w, rng):
    yy, xx = np.mgrid[0:h, 0:w]
    cx, cy = w * 0.5, h * 0.5
    r = np.hypot(xx - cx, yy - cy) / (min(h, w) * 0.5)
    ang = np.arctan2(yy - cy, xx - cx)
    wobble = 0.03 * np.sin(ang * 5 + rng.uniform(0, 6)) + 0.02 * np.sin(ang * 11)
    rings = 0.5 + 0.5 * np.sin((r + wobble) * 2 * np.pi * 9)
    grain = _periodic_noise(h, w, 30, 30, rng)

    light = np.array([0.62, 0.50, 0.34])
    darker = np.array([0.48, 0.37, 0.24])
    col = _lerp(light, darker, np.clip(rings * 0.7 + 0.1 * grain, 0, 1))
    # weathering towards the edges
    col = col * (1.0 - 0.25 * np.clip(r - 0.6, 0, 1)[..., None])
    return np.clip(col, 0, 1)


def rope(h, w, rng):
    yy, xx = np.mgrid[0:h, 0:w]
    strands = 0.5 + 0.5 * np.sin((xx + yy * 0.9) / w * 2 * np.pi * 10)
    fibre = _periodic_noise(h, w, 40, 40, rng)
    light = np.array([0.66, 0.58, 0.42])
    dark = np.array([0.40, 0.34, 0.24])
    t = np.clip(strands ** 1.5 + 0.12 * fibre, 0, 1)
    return np.clip(_lerp(dark, light, t), 0, 1)


def build_atlas(seed=7):
    """Returns float array SIZE x SIZE x 3, row 0 = top of the image (v = 1)."""
    rng = np.random.default_rng(seed)
    img = np.zeros((SIZE, SIZE, 3))
    bark_w = int(SIZE * BARK_U[1])
    img[:, :bark_w] = bark(SIZE, bark_w, rng)

    half = SIZE // 2
    img[:half, bark_w:] = cut_wood(half, SIZE - bark_w, rng)   # top half: v 0.5-1
    img[half:, bark_w:] = rope(half, SIZE - bark_w, rng)       # bottom half: v 0-0.5
    return img


# ================================================================ model


# ---------------------------------------------------------------- settings

WALL_LENGTH = 4.0
POST_COUNT = 16
POST_RADIUS = (0.112, 0.130)
POST_HEIGHT = 2.70          # above ground, before the tip
POST_HEIGHT_VAR = 0.12
POST_TIP = 0.34
POST_SINK = 0.35            # below ground
POST_TILT_DEG = 1.2

RAIL_RADIUS = 0.085
RAIL_Z = (0.75, 2.05)
RAIL_OVERHANG = 0.10

ROPE_HEIGHT = 0.075
ROPE_THICK = 0.022
ROPE_WRAP_WIDTH = 0.07      # turns around the rail, centred on each post

GEOMETRY_MASS = 800.0       # kg, spread over Geometry LOD vertices

TEX_PATH = "CustomPalisade\\Data\\Textures\\cp_palisade_wood_co.paa"
RVMAT_PATH = "CustomPalisade\\Data\\Materials\\cp_palisade_wood.rvmat"
FIRE_RVMAT = "dz\\data\\data\\penetration\\wood_desk.rvmat"   # vanilla, used by DayZ-Samples

LOD_VISUAL, LOD_SHADOW, LOD_GEOMETRY, LOD_MEMORY, LOD_VIEWGEO, LOD_FIREGEO = 0, 4, 6, 9, 14, 15

BARK_U0, BARK_U1 = 0.01, 0.74
WOOD_U0, WOOD_U1, WOOD_V0, WOOD_V1 = 0.76, 0.99, 0.51, 0.99
ROPE_U0, ROPE_U1, ROPE_V0, ROPE_V1 = 0.76, 0.99, 0.01, 0.49


# ---------------------------------------------------------------- layout

def make_posts(seed=11):
    rng = random.Random(seed)
    spacing = WALL_LENGTH / POST_COUNT
    posts = []
    for i in range(POST_COUNT):
        x = -WALL_LENGTH / 2 + spacing * (i + 0.5)
        posts.append({
            "index": i,
            "x": x + rng.uniform(-0.01, 0.01),
            "y": rng.uniform(-0.015, 0.015),
            "r": rng.uniform(*POST_RADIUS),
            "h": POST_HEIGHT + rng.uniform(-POST_HEIGHT_VAR, POST_HEIGHT_VAR),
            "tip": POST_TIP + rng.uniform(-0.05, 0.05),
            "tilt_x": math.radians(rng.uniform(-POST_TILT_DEG, POST_TILT_DEG)),
            "tilt_y": math.radians(rng.uniform(-POST_TILT_DEG, POST_TILT_DEG)),
            "v_off": rng.uniform(0, 1),
            "rot": rng.uniform(0, 2 * math.pi),
            "base": i in (0, POST_COUNT - 1),
        })
    return posts


# ---------------------------------------------------------------- mesh helpers

class MeshBuilder:
    """Collects faces with per-corner UVs, smooth flags and selection membership."""

    def __init__(self):
        self.verts = []
        self.faces = []   # (vert indices, uvs or None, smooth, selections)

    def v(self, co):
        self.verts.append(tuple(co))
        return len(self.verts) - 1

    def f(self, idx, uvs=None, smooth=False, sel=()):
        self.faces.append((list(idx), uvs, smooth, tuple(sel)))

    def to_object(self, name, collection, material=None):
        mesh = bpy.data.meshes.new(name)
        obj = bpy.data.objects.new(name, mesh)
        collection.objects.link(obj)
        bm = bmesh.new()
        bverts = [bm.verts.new(co) for co in self.verts]
        bm.verts.ensure_lookup_table()
        uv_layer = bm.loops.layers.uv.new("UVMap")
        sel_names = sorted({s for _, _, _, sels in self.faces for s in sels})
        face_sel = []
        for idx, uvs, smooth, sels in self.faces:
            try:
                face = bm.faces.new([bverts[i] for i in idx])
            except ValueError:
                continue
            face.smooth = smooth
            if uvs:
                for loop, uv in zip(face.loops, uvs):
                    loop[uv_layer].uv = uv
            face_sel.append((face, sels))
        bmesh.ops.recalc_face_normals(bm, faces=bm.faces[:])
        bm.to_mesh(mesh)
        bm.free()
        if material:
            mesh.materials.append(material)
        # selections: a vertex belongs to every selection of its faces
        poly_sels = [sels for _, sels in face_sel]   # same order as mesh.polygons
        for s in sel_names:
            obj.vertex_groups.new(name=s)
        for s in sel_names:
            members = set()
            for poly, sels in zip(mesh.polygons, poly_sels):
                if s in sels:
                    members.update(poly.vertices)
            obj.vertex_groups[s].add(sorted(members), 1.0, 'REPLACE')
        return obj


def add_cap(mb, ring, ring_uvs, center_co, center_uv, sel, reverse=False):
    """Closes a ring with a triangle fan (P3D visual LODs allow at most quads)."""
    c = mb.v(center_co)
    n = len(ring)
    for s in range(n):
        s1 = (s + 1) % n
        idx = [ring[s], ring[s1], c]
        uvs = [ring_uvs[s], ring_uvs[s1], center_uv] if ring_uvs else None
        if reverse:
            idx.reverse()
            if uvs:
                uvs.reverse()
        mb.f(idx, uvs, sel=sel)


def post_frame(p):
    """Returns a function mapping local (lx, ly, z) of a post to world space (with tilt)."""
    cx, cy = p["x"], p["y"]

    def to_world(lx, ly, z):
        return (cx + lx + z * math.sin(p["tilt_x"]), cy + ly + z * math.sin(p["tilt_y"]), z)
    return to_world


def add_post(mb, p, sides, sel, with_tip=True, closed=True, uv=True):
    tw = post_frame(p)
    r = p["r"]
    z_rings = [-POST_SINK, p["h"] * 0.5, p["h"]]
    rings = []
    for zi, z in enumerate(z_rings):
        rr = r * (1.04 if zi == 0 else (1.0 if zi == 1 else 0.97))
        ring = []
        for s in range(sides):
            a = p["rot"] + 2 * math.pi * s / sides
            ring.append(mb.v(tw(rr * math.cos(a), rr * math.sin(a), z)))
        rings.append(ring)

    def bark_uv(s, z):
        u = BARK_U0 + (BARK_U1 - BARK_U0) * s / sides
        return (u, z / 1.6 + p["v_off"])

    for ri in range(len(rings) - 1):
        lo, hi = rings[ri], rings[ri + 1]
        for s in range(sides):
            s1 = (s + 1) % sides
            uvs = None
            if uv:
                uvs = [bark_uv(s, z_rings[ri]), bark_uv(s + 1, z_rings[ri]),
                       bark_uv(s + 1, z_rings[ri + 1]), bark_uv(s, z_rings[ri + 1])]
            mb.f([lo[s], lo[s1], hi[s1], hi[s]], uvs, smooth=True, sel=sel)

    top = rings[-1]
    wc_u, wc_v = (WOOD_U0 + WOOD_U1) / 2, (WOOD_V0 + WOOD_V1) / 2
    wr_u, wr_v = (WOOD_U1 - WOOD_U0) / 2, (WOOD_V1 - WOOD_V0) / 2

    def wood_uv(s):
        a = 2 * math.pi * s / sides
        return (wc_u + wr_u * math.cos(a), wc_v + wr_v * math.sin(a))

    if with_tip:
        apex = mb.v(tw(0, 0, p["h"] + p["tip"]))
        for s in range(sides):
            s1 = (s + 1) % sides
            uvs = [wood_uv(s), wood_uv(s + 1), (wc_u, wc_v)] if uv else None
            mb.f([top[s], top[s1], apex], uvs, smooth=False, sel=sel)
    elif closed:
        uvs = [wood_uv(s) for s in range(sides)] if uv else None
        add_cap(mb, top, uvs, tw(0, 0, p["h"]), (wc_u, wc_v), sel)

    if closed:
        uvs = [wood_uv(s) for s in range(sides)] if uv else None
        add_cap(mb, rings[0], uvs, tw(0, 0, -POST_SINK), (wc_u, wc_v), sel, reverse=True)


def add_rail(mb, z, sides, sel, uv=True, seed=0):
    rng = random.Random(seed)
    x0 = -WALL_LENGTH / 2 - RAIL_OVERHANG
    x1 = WALL_LENGTH / 2 + RAIL_OVERHANG
    y = POST_RADIUS[1] + RAIL_RADIUS - 0.01
    segs = 4
    v_off = rng.uniform(0, 1)
    rings = []
    xs = [x0 + (x1 - x0) * i / segs for i in range(segs + 1)]
    for i, x in enumerate(xs):
        sag = 0.012 * math.sin(math.pi * i / segs)
        ring = []
        for s in range(sides):
            a = 2 * math.pi * s / sides
            ring.append(mb.v((x, y + RAIL_RADIUS * math.cos(a), z - sag + RAIL_RADIUS * math.sin(a))))
        rings.append(ring)
    for i in range(segs):
        for s in range(sides):
            s1 = (s + 1) % sides
            uvs = None
            if uv:
                u0 = BARK_U0 + (BARK_U1 - BARK_U0) * s / sides
                u1 = BARK_U0 + (BARK_U1 - BARK_U0) * (s + 1) / sides
                v0, v1 = xs[i] / 1.6 + v_off, xs[i + 1] / 1.6 + v_off
                uvs = [(u0, v0), (u0, v1), (u1, v1), (u1, v0)]
            a, b, c, d = rings[i][s], rings[i + 1][s], rings[i + 1][s1], rings[i][s1]
            mb.f([a, b, c, d], uvs, smooth=True, sel=sel)
    wc_u, wc_v = (WOOD_U0 + WOOD_U1) / 2, (WOOD_V0 + WOOD_V1) / 2
    wr_u, wr_v = (WOOD_U1 - WOOD_U0) / 2, (WOOD_V1 - WOOD_V0) / 2
    cap = [(wc_u + wr_u * math.cos(2 * math.pi * s / sides), wc_v + wr_v * math.sin(2 * math.pi * s / sides))
           for s in range(sides)]
    sag0 = 0.0
    add_cap(mb, rings[0], cap if uv else None, (xs[0], y, z - sag0), (wc_u, wc_v), sel, reverse=True)
    add_cap(mb, rings[-1], cap if uv else None, (xs[-1], y, z - sag0), (wc_u, wc_v), sel)


def rail_axis(x, z):
    """Centre of the rail cross-section at world X (rails sag slightly)."""
    x0 = -WALL_LENGTH / 2 - RAIL_OVERHANG
    x1 = WALL_LENGTH / 2 + RAIL_OVERHANG
    y = POST_RADIUS[1] + RAIL_RADIUS - 0.01
    return y, z - 0.012 * math.sin(math.pi * (x - x0) / (x1 - x0))


def add_band(mb, path_lo, path_hi, outer_lo, outer_hi, sel, closed=True):
    """Thin closed rope band between an inner and an outer surface."""
    n = len(path_lo)
    count = n if closed else n - 1
    for i in range(count):
        j = (i + 1) % n
        u0 = ROPE_U0 + (ROPE_U1 - ROPE_U0) * i / n
        u1 = ROPE_U0 + (ROPE_U1 - ROPE_U0) * (i + 1) / n
        uv = [(u0, ROPE_V0), (u1, ROPE_V0), (u1, ROPE_V1), (u0, ROPE_V1)]
        mb.f([outer_lo[i], outer_lo[j], outer_hi[j], outer_hi[i]], uv, smooth=True, sel=sel)
        mb.f([path_hi[i], path_hi[j], path_lo[j], path_lo[i]], uv, smooth=True, sel=sel)
        mb.f([outer_hi[i], outer_hi[j], path_hi[j], path_hi[i]], uv, sel=sel)
        mb.f([path_lo[i], path_lo[j], outer_lo[j], outer_lo[i]], uv, sel=sel)


def add_lashing(mb, p, z, sel, segs=10):
    """Rope lashing of one post to one rail.

    - outside: a band around the outer (-Y) half of the post, its ends run
      into the rail (hidden inside it);
    - inside: several turns wrapped around the rail (ROPE_WRAP_WIDTH wide).
    """
    tw = post_frame(p)
    r = p["r"] + 0.006
    rail_y, rail_z = rail_axis(p["x"], z)
    rail_y_local = rail_y - p["y"]

    # band around the post
    path = []
    for i in range(segs + 1):                      # (r,0) -> (0,-r) -> (-r,0)
        a = -math.pi * i / segs
        path.append((r * math.cos(a), r * math.sin(a)))
    path.append((-r, rail_y_local))
    path.append((r, rail_y_local))
    cy = rail_y_local * 0.5

    def offset(pt, d):
        x, y = pt
        L = math.hypot(x, y - cy) or 1.0
        return (x + x / L * d, y + (y - cy) / L * d)

    z0, z1 = rail_z - ROPE_HEIGHT / 2, rail_z + ROPE_HEIGHT / 2
    inner_lo = [mb.v(tw(*pt, z0)) for pt in path]
    inner_hi = [mb.v(tw(*pt, z1)) for pt in path]
    outer_lo = [mb.v(tw(*offset(pt, ROPE_THICK), z0)) for pt in path]
    outer_hi = [mb.v(tw(*offset(pt, ROPE_THICK), z1)) for pt in path]
    add_band(mb, inner_lo, inner_hi, outer_lo, outer_hi, sel)

    # turns around the rail (axis along X)
    ring_r = RAIL_RADIUS + 0.004
    xa = p["x"] - ROPE_WRAP_WIDTH / 2
    xb = p["x"] + ROPE_WRAP_WIDTH / 2

    def ring(x, rad):
        y_c, z_c = rail_axis(x, z)
        return [mb.v((x, y_c + rad * math.cos(2 * math.pi * k / segs), z_c + rad * math.sin(2 * math.pi * k / segs)))
                for k in range(segs)]

    in_a, in_b = ring(xa, ring_r), ring(xb, ring_r)
    out_a, out_b = ring(xa, ring_r + ROPE_THICK), ring(xb, ring_r + ROPE_THICK)
    add_band(mb, in_a, in_b, out_a, out_b, sel)


def add_box(mb, x0, x1, y0, y1, z0, z1, sel=()):
    c = [mb.v((x, y, z)) for z in (z0, z1) for y in (y0, y1) for x in (x0, x1)]
    # indices: z0: 0(x0y0) 1(x1y0) 2(x0y1) 3(x1y1); z1: 4..7
    for quad in ([0, 2, 3, 1], [4, 5, 7, 6], [0, 1, 5, 4], [2, 6, 7, 3], [0, 4, 6, 2], [1, 3, 7, 5]):
        mb.f([c[i] for i in quad], sel=sel)


# ---------------------------------------------------------------- materials

def make_materials(out_dir):
    atlas = build_atlas()
    png = os.path.join(out_dir, "cp_palisade_wood_co.png")
    save_png(atlas, png)
    write_paa_dxt1(os.path.join(out_dir, "cp_palisade_wood_co.paa"), atlas)

    wood = bpy.data.materials.new("cp_palisade_wood")
    wood.a3ob_properties_material.texture_type = 'TEX'
    wood.a3ob_properties_material.texture_path = TEX_PATH
    wood.a3ob_properties_material.material_path = RVMAT_PATH
    wood.use_nodes = True
    nt = wood.node_tree
    bsdf = nt.nodes.get("Principled BSDF")
    img_node = nt.nodes.new("ShaderNodeTexImage")
    img_node.image = bpy.data.images.load(png)
    nt.links.new(img_node.outputs["Color"], bsdf.inputs["Base Color"])
    bsdf.inputs["Roughness"].default_value = 0.85

    fire = bpy.data.materials.new("cp_palisade_fire")
    fire.a3ob_properties_material.texture_type = 'TEX'
    fire.a3ob_properties_material.texture_path = ""
    fire.a3ob_properties_material.material_path = FIRE_RVMAT
    return wood, fire


# ---------------------------------------------------------------- LODs

def triangulate_sharp(obj):
    """Geometry and shadow LODs: triangulated, flat-shaded, all edges sharp."""
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    bmesh.ops.triangulate(bm, faces=bm.faces[:])
    for f in bm.faces:
        f.smooth = False
    for e in bm.edges:
        e.smooth = False
    bm.to_mesh(obj.data)
    bm.free()


def set_lod(obj, lod, resolution=0, props=()):
    p = obj.a3ob_properties_object
    p.is_a3_lod = True
    p.lod = str(lod)
    p.resolution = resolution
    for name, value in props:
        item = p.properties.add()
        item.name = name
        item.value = value


def build_visual(posts, sides_post, sides_rail, rope, coll, mat, name, resolution):
    mb = MeshBuilder()
    for p in posts:
        sel = ["cp_part_base"] if p["base"] else ["cp_part_logs"]
        add_post(mb, p, sides_post, sel)
    for k, z in enumerate(RAIL_Z):
        add_rail(mb, z, sides_rail, ["cp_part_logs"], seed=k)
    if rope:
        for p in posts:
            for z in RAIL_Z:
                add_lashing(mb, p, z, ["cp_part_logs"])
    obj = mb.to_object(name, coll, mat)
    set_lod(obj, LOD_VISUAL, resolution)
    return obj


def build_shadow(posts, coll):
    mb = MeshBuilder()
    for p in posts:
        sel = ["cp_part_base"] if p["base"] else ["cp_part_logs"]
        add_post(mb, p, 6, sel, with_tip=True, closed=True, uv=False)
    for k, z in enumerate(RAIL_Z):
        add_rail(mb, z, 6, ["cp_part_logs"], uv=False, seed=k)
    obj = mb.to_object("ShadowVolume 0", coll)
    set_lod(obj, LOD_SHADOW, 0)
    triangulate_sharp(obj)   # shadow volumes: triangulated, sharp edges
    return obj


def geometry_boxes(mb):
    half = WALL_LENGTH / 2
    top = POST_HEIGHT - POST_HEIGHT_VAR + 0.1
    t = POST_RADIUS[1] + 0.01
    rail_y0 = POST_RADIUS[1] - 0.01
    rail_y1 = rail_y0 + 2 * RAIL_RADIUS + 0.02
    add_box(mb, -half, half, -t, t, 0.0, top)
    for z in RAIL_Z:
        add_box(mb, -half - RAIL_OVERHANG, half + RAIL_OVERHANG, rail_y0, rail_y1,
                z - RAIL_RADIUS - 0.01, z + RAIL_RADIUS + 0.01)


def build_geometry(coll):
    mb = MeshBuilder()
    geometry_boxes(mb)
    obj = mb.to_object("Geometry", coll)
    set_lod(obj, LOD_GEOMETRY, 0, props=[("autocenter", "0")])
    triangulate_sharp(obj)
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    layer = bm.verts.layers.float.new("a3ob_mass")
    per_vert = GEOMETRY_MASS / len(bm.verts)
    for v in bm.verts:
        v[layer] = per_vert
    bm.to_mesh(obj.data)
    bm.free()
    return obj


def build_simple_geo(coll, lod, name, mat=None):
    mb = MeshBuilder()
    geometry_boxes(mb)
    obj = mb.to_object(name, coll, mat)
    set_lod(obj, lod, 0)
    triangulate_sharp(obj)
    return obj


# ---------------------------------------------------------------- render preview

def render_previews(visual, png_dir):
    scene = bpy.context.scene
    for o in scene.objects:
        if o.type == 'MESH':
            o.hide_render = o is not visual
    bpy.ops.mesh.primitive_plane_add(size=30, location=(0, 0, 0))
    ground = bpy.context.active_object
    gmat = bpy.data.materials.new("ground")
    gmat.use_nodes = True
    gmat.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = (0.20, 0.22, 0.12, 1)
    gmat.node_tree.nodes["Principled BSDF"].inputs["Roughness"].default_value = 1.0
    ground.data.materials.append(gmat)
    ground.hide_render = False

    sun_data = bpy.data.lights.new("sun", type='SUN')
    sun_data.energy = 3.5
    sun = bpy.data.objects.new("sun", sun_data)
    sun.rotation_euler = (math.radians(50), math.radians(10), math.radians(-35))
    scene.collection.objects.link(sun)

    world = bpy.data.worlds.new("w")
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.55, 0.62, 0.70, 1)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.8
    scene.world = world

    scene.render.engine = 'CYCLES'
    scene.cycles.device = 'CPU'
    scene.cycles.samples = 48
    scene.render.resolution_x = 1280
    scene.render.resolution_y = 720
    scene.view_settings.view_transform = 'Standard'

    cam_data = bpy.data.cameras.new("cam")
    cam_data.lens = 32
    cam = bpy.data.objects.new("cam", cam_data)
    scene.collection.objects.link(cam)
    scene.camera = cam

    views = {
        "preview_outside": ((-3.6, -5.2, 1.9), (0.3, 0, 1.3)),
        "preview_inside": ((3.0, 4.2, 1.7), (-0.2, 0.2, 1.2)),
        "preview_closeup": ((-1.3, 1.6, 2.4), (-0.6, 0.15, 2.0)),
    }
    from mathutils import Vector
    for name, (loc, target) in views.items():
        cam.location = loc
        direction = Vector(target) - Vector(loc)
        cam.rotation_euler = direction.to_track_quat('-Z', 'Y').to_euler()
        scene.render.filepath = os.path.join(png_dir, name + ".png")
        bpy.ops.render.render(write_still=True)


# ---------------------------------------------------------------- main

def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    out_dir = os.path.abspath(argv[0]) if argv else output_dir()
    render_dir = os.path.abspath(argv[argv.index("--render") + 1]) if "--render" in argv else None
    os.makedirs(out_dir, exist_ok=True)
    print("CP: output folder:", out_dir)

    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)
    coll = bpy.context.scene.collection

    wood, fire = make_materials(out_dir)
    posts = make_posts()

    visual = build_visual(posts, 12, 10, True, coll, wood, "Resolution 1", 1)
    build_visual(posts, 7, 6, False, coll, wood, "Resolution 2", 2)
    build_visual(posts, 4, 4, False, coll, wood, "Resolution 3", 3)
    build_shadow(posts, coll)
    build_geometry(coll)
    build_simple_geo(coll, LOD_VIEWGEO, "View Geometry")
    build_simple_geo(coll, LOD_FIREGEO, "Fire Geometry", fire)

    p3d_path = os.path.join(out_dir, "cp_palisade_wall.p3d")
    if not hasattr(bpy.ops, "a3ob") or not hasattr(bpy.ops.a3ob, "export_p3d"):
        print("CP: 'Arma 3 Object Builder' add-on is not enabled - model built, P3D NOT exported.")
        print("CP: enable the add-on and run the script again, or use File > Export > Arma 3 model (.p3d).")
        return
    # the exporter switches modes and needs an active object
    if bpy.context.object and bpy.context.object.mode != 'OBJECT':
        bpy.ops.object.mode_set(mode='OBJECT')
    bpy.context.view_layer.objects.active = visual
    result = bpy.ops.a3ob.export_p3d(
        filepath=p3d_path,
        use_selection=False,
        visible_only=False,
        apply_transforms=True,
        apply_modifiers=True,
        generate_components=True,
        validate_lods=True,
        force_lowercase=True,
        relative_paths=True,
    )
    print("CP_EXPORT", result, p3d_path, os.path.getsize(p3d_path) if os.path.exists(p3d_path) else -1)

    if render_dir:
        os.makedirs(render_dir, exist_ok=True)
        render_previews(visual, render_dir)


if __name__ == "__main__":
    main()
