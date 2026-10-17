import unreal
EAL = unreal.EditorAssetLibrary; MEL = unreal.MaterialEditingLibrary; AT = unreal.AssetToolsHelpers.get_asset_tools()
def log(*a): unreal.log("[VERTICE] " + " ".join(str(x) for x in a))
mp = "/Game/Vertice/Materials/MI_Vertice"
if EAL.does_asset_exist(mp): EAL.delete_asset(mp)
mi = AT.create_asset("MI_Vertice", "/Game/Vertice/Materials", unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
MEL.set_material_instance_parent(mi, unreal.load_asset("/Game/Vertice/Materials/M_Vertice"))
MEL.set_material_instance_scalar_parameter_value(mi, "Brilho", 3.0)
EAL.save_loaded_asset(mi)
sk = unreal.load_asset("/Game/Vertice/Mesh/SK_Vertice")
mats = []
for sm in sk.get_editor_property("materials"):
    nm = str(sm.get_editor_property("material_slot_name"))
    mats.append(unreal.SkeletalMaterial(material_interface=(mi if "Tripo" in nm else sm.get_editor_property("material_interface")), material_slot_name=sm.get_editor_property("material_slot_name")))
sk.set_editor_property("materials", mats); EAL.save_loaded_asset(sk); log("brilho controlado: MI_Vertice Brilho=3")
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem); EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LES.load_level("/Game/Vertice/Maps/Teste_Vertice")
for a in EAS.get_all_level_actors():
    if isinstance(a, unreal.DirectionalLight):
        a.set_actor_rotation(unreal.Rotator(0, -38, 200), False); log("sol reposicionado (luz frontal)")
LES.save_current_level(); log("FIM OK")
