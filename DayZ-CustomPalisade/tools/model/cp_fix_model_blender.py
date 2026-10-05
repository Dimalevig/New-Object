"""Cleans up the palisade P3D in Blender (Arma 3 Object Builder add-on required).

For every visual LOD (Resolution N):
  - removes exactly duplicated faces (same vertices),
  - turns inside-out faces of the wire material outwards,
  - drops the broken custom split normals and recomputes normals
    (smooth logs, sharp plank / wire edges by angle).
All other LODs (shadow, geometry, memory, view/fire geometry) are exported unchanged.
Selections, named properties, materials and vertex mass are kept by the add-on.

    blender --background --python cp_fix_model_blender.py -- <in.p3d> <out.p3d>
"""
import math
import sys

import bmesh
import bpy

SMOOTH_ANGLE_DEG = 40.0
INSIDE_OUT_MATERIALS = ("metalwire",)


def face_key(mesh, poly):
    return tuple(sorted(tuple(round(c, 5) for c in mesh.vertices[v].co) for v in poly.vertices))


def fix_visual_lod(obj):
    mesh = obj.data
    report = {}

    # 1) duplicated faces
    seen = set()
    dup = []
    for poly in mesh.polygons:
        k = face_key(mesh, poly)
        if k in seen:
            dup.append(poly.index)
        else:
            seen.add(k)
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bm.faces.ensure_lookup_table()
    if dup:
        bmesh.ops.delete(bm, geom=[bm.faces[i] for i in dup], context='FACES_ONLY')
    report["duplicates_removed"] = len(dup)

    # 2) inside-out faces of selected materials
    flip_idx = {i for i, m in enumerate(mesh.materials)
                if m and any(s in m.a3ob_properties_material.texture_path.lower() for s in INSIDE_OUT_MATERIALS)}
    flip_faces = [f for f in bm.faces if f.material_index in flip_idx]
    before = {f.index: f.normal.copy() for f in flip_faces}
    if flip_faces:
        bmesh.ops.recalc_face_normals(bm, faces=flip_faces)   # point them outwards
    report["faces_flipped"] = sum(1 for f in flip_faces if f.normal.dot(before[f.index]) < 0)
    bm.to_mesh(mesh)
    bm.free()

    # 3) recompute normals
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    if mesh.has_custom_normals:
        bpy.ops.mesh.customdata_custom_splitnormals_clear()
    bpy.ops.object.shade_smooth_by_angle(angle=math.radians(SMOOTH_ANGLE_DEG))
    return report


def main():
    argv = sys.argv[sys.argv.index("--") + 1:]
    src, dst = argv[0], argv[1]

    for obj in list(bpy.data.objects):          # empty scene, keep the user's add-ons
        bpy.data.objects.remove(obj, do_unlink=True)
    if not hasattr(bpy.ops, "a3ob") or not hasattr(bpy.ops.a3ob, "import_p3d"):
        raise SystemExit("Arma 3 Object Builder add-on is not enabled")

    bpy.ops.a3ob.import_p3d(filepath=src)
    for obj in bpy.data.objects:
        if obj.type != 'MESH' or not obj.a3ob_properties_object.is_a3_lod:
            continue
        if obj.a3ob_properties_object.lod == '0':
            print("CP_FIX", obj.name, fix_visual_lod(obj))

    any_obj = next(o for o in bpy.data.objects if o.type == 'MESH')
    bpy.context.view_layer.objects.active = any_obj
    result = bpy.ops.a3ob.export_p3d(
        filepath=dst,
        use_selection=False,
        visible_only=False,
        apply_transforms=True,
        apply_modifiers=True,
        preserve_normals=True,
        generate_components=False,
        renumber_components=False,
        validate_lods=False,
        force_lowercase=True,
        relative_paths=True,
    )
    print("CP_EXPORT", result, dst)


if __name__ == "__main__":
    main()
