import unreal
def log(*a): unreal.log("[DIAG] " + " ".join(str(x) for x in a))
sk = unreal.load_asset("/Game/Vertice/Mesh/SK_Vertice")
log("classe", type(sk), "skeleton", sk.skeleton)
for n in ("get_lod_num",):
    try: log(n, getattr(sk, n)())
    except Exception as e: log(n, e)
try:
    sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    log("lods", sub.get_lod_count(sk), "verts", sub.get_num_verts(sk, 0), "secoes", sub.get_num_sections(sk, 0) if hasattr(sub,'get_num_sections') else '-')
except Exception as e: log("sub", e)
try:
    log("lods(lib)", unreal.EditorSkeletalMeshLibrary.get_lod_count(sk), "verts", unreal.EditorSkeletalMeshLibrary.get_num_verts(sk, 0))
except Exception as e: log("lib", e)
log("materiais", [ (str(m.material_slot_name), m.material_interface.get_name() if m.material_interface else None) for m in sk.materials])
lod = sk.get_editor_property("lod_info")
for i, l in enumerate(lod):
    log("lodinfo", i, l.get_editor_property("screen_size"), "secoes", [ (s.get_editor_property('section_index') if hasattr(s,'get_editor_property') else s) for s in [] ])
b = sk.get_bounds(); log("bounds", b.origin, b.box_extent)
log("positive bounds ext", sk.get_editor_property("positive_bounds_extension"), sk.get_editor_property("negative_bounds_extension"))
