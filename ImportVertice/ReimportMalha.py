# Reimporta só a malha do Vértice (mesmo esqueleto) — animações e retargeting continuam válidos.
import unreal, os
DIR = os.path.dirname(os.path.abspath(__file__))
EAL = unreal.EditorAssetLibrary
def log(*a): unreal.log("[VERTICE] " + " ".join(str(x) for x in a))
skel = unreal.load_asset("/Game/Vertice/Mesh/SK_Vertice_Skeleton")
ui = unreal.FbxImportUI()
ui.import_mesh = True; ui.import_as_skeletal = True; ui.import_animations = False
ui.import_materials = False; ui.import_textures = False; ui.create_physics_asset = False
ui.skeleton = skel
ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
ui.skeletal_mesh_import_data.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
t = unreal.AssetImportTask(); t.filename = os.path.join(DIR, "SK_Vertice.fbx"); t.destination_path = "/Game/Vertice/Mesh"
t.destination_name = "SK_Vertice"; t.automated = True; t.replace_existing = True; t.save = True; t.options = ui
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
log("importado:", t.imported_object_paths)
sk = unreal.load_asset("/Game/Vertice/Mesh/SK_Vertice")
log("esqueleto:", sk.skeleton.get_path_name())
mi = unreal.load_asset("/Game/Vertice/Materials/MI_Vertice"); mj = unreal.load_asset("/Game/Vertice/Materials/M_Vertice_Junta")
mats = []
for sm in sk.get_editor_property("materials"):
    nm = str(sm.get_editor_property("material_slot_name"))
    mats.append(unreal.SkeletalMaterial(material_interface=(mj if "Junta" in nm else mi), material_slot_name=sm.get_editor_property("material_slot_name")))
    log("slot", nm)
sk.set_editor_property("materials", mats); EAL.save_loaded_asset(sk)
sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
log("verts", sub.get_num_verts(sk, 0), "bounds", sk.get_bounds().box_extent)
log("FIM OK")
