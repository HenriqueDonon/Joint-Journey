import unreal
def log(*a): unreal.log("[VERTICE] " + " ".join(str(x) for x in a))
sk = unreal.load_asset("/Game/Vertice/Mesh/SK_Vertice")
ss = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
try:
    ok = ss.regenerate_lod(sk, 3, True, False); log("regenerate 1", ok)
    infos = sk.get_editor_property("lod_info")
    for i, pct, sz in ((1, 0.42, 0.5), (2, 0.21, 0.25)):
        if i < len(infos):
            rs = infos[i].get_editor_property("reduction_settings")
            rs.set_editor_property("num_of_triangles_percentage", pct)
            infos[i].set_editor_property("reduction_settings", rs)
            infos[i].set_editor_property("screen_size", unreal.PerPlatformFloat(sz))
    sk.set_editor_property("lod_info", infos)
    ok = ss.regenerate_lod(sk, 3, True, False); log("regenerate 2", ok)
    for i in range(ss.get_lod_count(sk)): log("LOD", i, "verts", ss.get_num_verts(sk, i))
except Exception as e: log("LOD erro:", e)
unreal.EditorAssetLibrary.save_loaded_asset(sk)
log("FIM OK")
