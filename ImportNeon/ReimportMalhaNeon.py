import unreal, os
DIR = os.path.dirname(os.path.abspath(__file__))
EAL = unreal.EditorAssetLibrary
def log(*a): unreal.log("[NEON] " + " ".join(str(x) for x in a))
ui = unreal.FbxImportUI()
ui.import_mesh = True; ui.import_as_skeletal = True; ui.import_animations = False
ui.import_materials = False; ui.import_textures = False; ui.create_physics_asset = False
ui.skeleton = unreal.load_asset("/Game/Neon/Mesh/SK_Neon_Skeleton")
ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
ui.skeletal_mesh_import_data.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
t = unreal.AssetImportTask(); t.filename = os.path.join(DIR, "SK_Neon.fbx"); t.destination_path = "/Game/Neon/Mesh"
t.destination_name = "SK_Neon"; t.automated = True; t.replace_existing = True; t.save = True; t.options = ui
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
sk = unreal.load_asset("/Game/Neon/Mesh/SK_Neon")
mi = unreal.load_asset("/Game/Neon/Materials/MI_Neon")
sk.set_editor_property("materials", [unreal.SkeletalMaterial(material_interface=mi, material_slot_name=s.get_editor_property("material_slot_name")) for s in sk.get_editor_property("materials")])
EAL.save_loaded_asset(sk)
sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
log("verts", sub.get_num_verts(sk, 0), "bounds", sk.get_bounds().box_extent)
log("FIM OK")
