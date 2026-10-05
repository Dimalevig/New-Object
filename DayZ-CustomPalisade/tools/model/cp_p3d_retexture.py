"""Retexture an MLOD P3D in place: change texture / material paths and remap UVs.

Everything that is not a face texture, material or UV (vertices, normals,
selections incl. face selection data, sharp edges, mass, properties) is copied
byte for byte. The LOD count in the header is rewritten to the number of LODs
actually present in the file.

MLOD layout (community wiki "P3D File Format - MLOD"):
    "MLOD" uint32 version(257) uint32 lodCount
    per LOD: "P3DM" uint32 major uint32 minor uint32 nPoints uint32 nNormals uint32 nFaces uint32 flags
             points  nPoints  * (float x, y, z, uint32 flags)
             normals nNormals * (float x, y, z)
             faces   nFaces   * (uint32 nVerts, 4 * (uint32 point, uint32 normal, float u, float v),
                                 uint32 flags, asciiz texture, asciiz material)
             "TAGG"  tags: (uint8 active, asciiz name, uint32 size, data) ... "#EndOfFile#"
             float resolution

Usage:
    python cp_p3d_retexture.py <in.p3d> <out.p3d>
"""
import struct
import sys

# ---------------------------------------------------------------- remap rules
# Matched against the lower-case texture path of each face.
# uv(u, v) receives raw P3D UVs (v = 0 at the top of the image).

PILE_OF_PLANKS_CO = "dz\\gear\\consumables\\data\\pile_of_planks_co.paa"
WOODEN_LOG_RVMAT = "dz\\gear\\camping\\data\\wooden_log.rvmat"
WOODEN_PLANKS_CO = "dz\\gear\\consumables\\data\\wooden_planks_co.paa"
WOODEN_PLANKS_RVMAT = "dz\\gear\\consumables\\data\\wooden_planks.rvmat"
METALWIRE_CO = "dz\\gear\\crafting\\data\\string_metalwire_co.paa"
METALWIRE_RVMAT = "dz\\gear\\camping\\data\\fence_metalwire.rvmat"
# bullet penetration material for the Fire Geometry LOD (used by DayZ-Samples)
FIRE_WOOD_RVMAT = "dz\\data\\data\\penetration\\wood_desk.rvmat"
FIRE_GEOMETRY_RES = 7e15

# pile_of_planks_co.paa: bark column (tiles vertically) is u 0.585 .. 1.0
BARK_U0, BARK_U1 = 0.595, 0.990


def uv_logs(u, v):
    # source: bark mapped once around the log on u 0 .. 0.75, tiling on v
    return BARK_U0 + (u / 0.75) * (BARK_U1 - BARK_U0), v


def uv_planks(u, v):
    # source: planar front projection, u ~ length (m), v ~ height
    # wooden_planks_co.paa: grain runs along v, 4 planks across u, tiles on both
    return (1.0 - v) * 1.25, u * 0.25


def uv_same(u, v):
    return u, v


RULES = [
    ("palisade_logs", PILE_OF_PLANKS_CO, WOODEN_LOG_RVMAT, uv_logs),
    ("palisade_planks", WOODEN_PLANKS_CO, WOODEN_PLANKS_RVMAT, uv_planks),
    ("palisade_wire", METALWIRE_CO, METALWIRE_RVMAT, uv_same),
]


def find_rule(texture):
    t = texture.lower()
    for key, tex, mat, fn in RULES:
        if key in t:
            return tex, mat, fn
    return None


# ---------------------------------------------------------------- binary helpers

def read_asciiz(d, p):
    e = d.index(b"\0", p)
    return d[p:e], e + 1


def asciiz(s):
    return s.encode("ascii") + b"\0"


def lod_resolution(d, p):
    """Reads the resolution float stored after the LOD's tags (without modifying anything)."""
    nv, nn, nf = struct.unpack_from("<3I", d, p + 12)
    q = p + 28 + nv * 16 + nn * 12
    for _ in range(nf):
        q = d.index(b"\0", q + 72) + 1
        q = d.index(b"\0", q) + 1
    q += 4
    while True:
        e = d.index(b"\0", q + 1)
        name = d[q + 1:e]
        size = struct.unpack_from("<I", d, e + 1)[0]
        q = e + 5 + size
        if name == b"#EndOfFile#":
            return struct.unpack_from("<f", d, q)[0]


def process(src):
    d = src
    if d[:4] != b"MLOD":
        raise ValueError("not an MLOD P3D")
    version, declared = struct.unpack_from("<II", d, 4)
    p = 12
    out_lods = []
    stats = {}
    while p < len(d):
        if d[p:p + 4] != b"P3DM":
            raise ValueError("LOD signature expected at %d" % p)
        major, minor, nv, nn, nf, flags = struct.unpack_from("<6I", d, p + 4)
        is_fire = abs(lod_resolution(d, p) - FIRE_GEOMETRY_RES) < 1e12
        head = d[p:p + 28]
        p += 28
        pts_norms = d[p:p + nv * 16 + nn * 12]
        p += nv * 16 + nn * 12

        faces_out = bytearray()
        corner_fns = []          # per face corner: uv function (for #UVSet#)
        for _ in range(nf):
            nverts = struct.unpack_from("<I", d, p)[0]
            corners = [list(struct.unpack_from("<IIff", d, p + 4 + 16 * i)) for i in range(4)]
            fflags = struct.unpack_from("<I", d, p + 68)[0]
            tex, q = read_asciiz(d, p + 72)
            mat, q = read_asciiz(d, q)
            p = q
            rule = find_rule(tex.decode("latin1"))
            fn = uv_same
            if rule:
                new_tex, new_mat, fn = rule
                tex, mat = new_tex.encode("ascii"), new_mat.encode("ascii")
                for i in range(nverts):
                    corners[i][2], corners[i][3] = fn(corners[i][2], corners[i][3])
                stats[new_tex] = stats.get(new_tex, 0) + 1
            if is_fire and not mat:
                mat = FIRE_WOOD_RVMAT.encode("ascii")
                stats[FIRE_WOOD_RVMAT] = stats.get(FIRE_WOOD_RVMAT, 0) + 1
            corner_fns.extend([fn] * nverts)
            faces_out += struct.pack("<I", nverts)
            for c in corners:
                faces_out += struct.pack("<IIff", *c)
            faces_out += struct.pack("<I", fflags) + tex + b"\0" + mat + b"\0"

        if d[p:p + 4] != b"TAGG":
            raise ValueError("TAGG expected at %d" % p)
        p += 4
        tags_out = bytearray(b"TAGG")
        while True:
            active = d[p:p + 1]
            name, q = read_asciiz(d, p + 1)
            size = struct.unpack_from("<I", d, q)[0]
            data = d[q + 4:q + 4 + size]
            p = q + 4 + size
            if name == b"#UVSet#":
                set_id = struct.unpack_from("<I", data, 0)[0]
                vals = list(struct.unpack_from("<%df" % ((size - 4) // 4), data, 4))
                if set_id == 0 and len(vals) // 2 == len(corner_fns):
                    for k, fn in enumerate(corner_fns):
                        vals[2 * k], vals[2 * k + 1] = fn(vals[2 * k], vals[2 * k + 1])
                data = struct.pack("<I", set_id) + struct.pack("<%df" % len(vals), *vals)
            tags_out += active + name + b"\0" + struct.pack("<I", len(data)) + data
            if name == b"#EndOfFile#":
                break
        resolution = d[p:p + 4]
        p += 4
        out_lods.append(bytes(head) + bytes(pts_norms) + bytes(faces_out) + bytes(tags_out) + bytes(resolution))

    out = b"MLOD" + struct.pack("<II", version, len(out_lods)) + b"".join(out_lods)
    return out, declared, len(out_lods), stats


def main():
    if len(sys.argv) != 3:
        raise SystemExit(__doc__)
    src = open(sys.argv[1], "rb").read()
    out, declared, found, stats = process(src)
    open(sys.argv[2], "wb").write(out)
    print("LODs declared %d, found %d" % (declared, found))
    for tex, n in stats.items():
        print("  %5d faces -> %s" % (n, tex))


if __name__ == "__main__":
    main()
