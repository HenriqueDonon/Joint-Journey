import unreal
EAL = unreal.EditorAssetLibrary; MEL = unreal.MaterialEditingLibrary; AT = unreal.AssetToolsHelpers.get_asset_tools()
def log(*a): unreal.log("[VERTICE] " + " ".join(str(x) for x in a))
for p in ("/Game/Vertice/Materials/M_Vertice", "/Game/Vertice/Materials/M_Vertice_Junta"):
    m = unreal.load_asset(p)
    m.set_editor_property("used_with_skeletal_mesh", True)
    MEL.recompile_material(m); EAL.save_loaded_asset(m); log("uso em malha esqueletica:", p)
# chão neutro (cinza fosco) para contraste com o personagem
cp = "/Game/Vertice/Materials/M_Teste_Chao"
if not EAL.does_asset_exist(cp):
    c = AT.create_asset("M_Teste_Chao", "/Game/Vertice/Materials", unreal.Material, unreal.MaterialFactoryNew())
    v = MEL.create_material_expression(c, unreal.MaterialExpressionConstant3Vector, -300, 0); v.set_editor_property("constant", unreal.LinearColor(0.22, 0.23, 0.25, 1))
    r = MEL.create_material_expression(c, unreal.MaterialExpressionConstant, -300, 200); r.set_editor_property("r", 0.85)
    MEL.connect_material_property(v, "", unreal.MaterialProperty.MP_BASE_COLOR); MEL.connect_material_property(r, "", unreal.MaterialProperty.MP_ROUGHNESS)
    MEL.recompile_material(c); EAL.save_loaded_asset(c)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem); EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LES.load_level("/Game/Vertice/Maps/Teste_Vertice")
mat = unreal.load_asset(cp)
cube = unreal.load_asset("/Engine/BasicShapes/Cube")
for a in EAS.get_all_level_actors():
    if a.get_actor_label() == "Chao":
        a.set_actor_scale3d(unreal.Vector(400, 400, 1)); a.static_mesh_component.set_material(0, mat); log("chao ampliado")
# marcos de referência (colunas) para perceber deslocamento e escala
for i in range(-3, 12):
    for sx in (-1, 1):
        b = EAS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(-600 + sx*500, i*600, 100))
        b.static_mesh_component.set_static_mesh(cube); b.set_actor_scale3d(unreal.Vector(0.4, 0.4, 2.0)); b.static_mesh_component.set_material(0, mat)
        b.set_actor_label("Coluna")
LES.save_current_level(); log("FIM OK")
