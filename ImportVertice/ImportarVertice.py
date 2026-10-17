# Importa o Vértice (FBX + texturas), cria materiais, IK Rig, retargeting das animações da Unreal e o mapa de teste.
import unreal, os
DIR = os.path.dirname(os.path.abspath(__file__))
AT = unreal.AssetToolsHelpers.get_asset_tools()
EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary
def log(*a): unreal.log("[VERTICE] " + " ".join(str(x) for x in a))

def task(src, dst, name, options=None):
    t = unreal.AssetImportTask()
    t.filename = os.path.join(DIR, src); t.destination_path = dst; t.destination_name = name
    t.automated = True; t.replace_existing = True; t.save = True
    if options: t.options = options
    AT.import_asset_tasks([t])
    return t.imported_object_paths

# limpa importação anterior (somente a pasta do Vértice)
for d in ("/Game/Vertice/Anims", "/Game/Vertice/Rig", "/Game/Vertice/Mesh"):
    if EAL.does_directory_exist(d): EAL.delete_directory(d)
# ---------- 1. malha esquelética ----------
ui = unreal.FbxImportUI()
ui.import_mesh = True; ui.import_as_skeletal = True; ui.import_animations = False
ui.import_materials = False; ui.import_textures = False; ui.create_physics_asset = True
ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
sd = ui.skeletal_mesh_import_data
sd.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
sd.set_editor_property("import_morph_targets", False)
sd.set_editor_property("convert_scene", True)
sd.set_editor_property("use_t0_as_ref_pose", False)
log("malha:", task("SK_Vertice.fbx", "/Game/Vertice/Mesh", "SK_Vertice", ui))
sk = unreal.load_asset("/Game/Vertice/Mesh/SK_Vertice")
bb = sk.get_bounds()
log("bounds origem", bb.origin, "extensao", bb.box_extent)

# ---------- 2. texturas ----------
tex = {}
for f, comp, srgb in (("T_Vertice_Basecolor.jpg", unreal.TextureCompressionSettings.TC_DEFAULT, True),
                      ("T_Vertice_Normal.png", unreal.TextureCompressionSettings.TC_NORMALMAP, False),
                      ("T_Vertice_Roughness.jpg", unreal.TextureCompressionSettings.TC_MASKS, False),
                      ("T_Vertice_Metallic.jpg", unreal.TextureCompressionSettings.TC_MASKS, False),
                      ("T_Vertice_Emissive.png", unreal.TextureCompressionSettings.TC_DEFAULT, True)):
    n = os.path.splitext(f)[0]
    task(f, "/Game/Vertice/Textures", n)
    t = unreal.load_asset("/Game/Vertice/Textures/" + n)
    t.set_editor_property("compression_settings", comp); t.set_editor_property("srgb", srgb)
    EAL.save_loaded_asset(t); tex[n] = t
log("texturas:", list(tex))

# ---------- 3. materiais (opacos) ----------
def new_mat(name):
    p = "/Game/Vertice/Materials/" + name
    if EAL.does_asset_exist(p): EAL.delete_asset(p)
    return AT.create_asset(name, "/Game/Vertice/Materials", unreal.Material, unreal.MaterialFactoryNew())
m = new_mat("M_Vertice")
m.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
def ts(t, x, y, st):
    e = MEL.create_material_expression(m, unreal.MaterialExpressionTextureSample, x, y)
    e.set_editor_property("texture", t); e.set_editor_property("sampler_type", st); return e
bc = ts(tex["T_Vertice_Basecolor"], -700, -300, unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
nm = ts(tex["T_Vertice_Normal"], -700, 0, unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
ro = ts(tex["T_Vertice_Roughness"], -700, 250, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
me_ = ts(tex["T_Vertice_Metallic"], -700, 500, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
em = ts(tex["T_Vertice_Emissive"], -700, 750, unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
k = MEL.create_material_expression(m, unreal.MaterialExpressionScalarParameter, -450, 900)
k.set_editor_property("parameter_name", "Brilho"); k.set_editor_property("default_value", 8.0)
mul = MEL.create_material_expression(m, unreal.MaterialExpressionMultiply, -250, 800)
MEL.connect_material_expressions(em, "RGB", mul, "A"); MEL.connect_material_expressions(k, "", mul, "B")
MEL.connect_material_property(bc, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
MEL.connect_material_property(nm, "RGB", unreal.MaterialProperty.MP_NORMAL)
MEL.connect_material_property(ro, "R", unreal.MaterialProperty.MP_ROUGHNESS)
MEL.connect_material_property(me_, "R", unreal.MaterialProperty.MP_METALLIC)
MEL.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
MEL.recompile_material(m); EAL.save_loaded_asset(m)
j = new_mat("M_Vertice_Junta")
c3 = MEL.create_material_expression(j, unreal.MaterialExpressionConstant3Vector, -400, 0); c3.set_editor_property("constant", unreal.LinearColor(0.03, 0.028, 0.03, 1))
r1 = MEL.create_material_expression(j, unreal.MaterialExpressionConstant, -400, 200); r1.set_editor_property("r", 0.5)
m1 = MEL.create_material_expression(j, unreal.MaterialExpressionConstant, -400, 300); m1.set_editor_property("r", 0.6)
MEL.connect_material_property(c3, "", unreal.MaterialProperty.MP_BASE_COLOR)
MEL.connect_material_property(r1, "", unreal.MaterialProperty.MP_ROUGHNESS)
MEL.connect_material_property(m1, "", unreal.MaterialProperty.MP_METALLIC)
MEL.recompile_material(j); EAL.save_loaded_asset(j)
mats = list(sk.get_editor_property("materials"))
new = []
for sm in mats:
    slot = str(sm.get_editor_property("material_slot_name"))
    mi = j if "Junta" in slot else m
    new.append(unreal.SkeletalMaterial(material_interface=mi, material_slot_name=sm.get_editor_property("material_slot_name")))
    log("slot", slot, "->", mi.get_name())
sk.set_editor_property("materials", new); EAL.save_loaded_asset(sk)

# ---------- 4. IK Rig + retargeting das animações do Manny ----------
ok_anims = []
try:
    def mk(name, path, cls, fac):
        p = path + "/" + name
        if EAL.does_asset_exist(p): EAL.delete_asset(p)
        return AT.create_asset(name, path, cls, fac)
    ikv = mk("IK_Vertice", "/Game/Vertice/Rig", unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    c = unreal.IKRigController.get_controller(ikv)
    c.set_skeletal_mesh(sk)
    c.apply_auto_generated_retarget_definition()
    log("cadeias IK_Vertice:", [str(ch.chain_name) for ch in c.get_retarget_chains()], "raiz:", c.get_retarget_root())
    EAL.save_loaded_asset(ikv)
    ikm = unreal.load_asset("/Game/Characters/Mannequins/Rigs/IK_Mannequin")
    manny = unreal.load_asset("/Game/Characters/Mannequins/Meshes/SKM_Manny")
    rtg = mk("RTG_Manny_para_Vertice", "/Game/Vertice/Rig", unreal.IKRetargeter, unreal.IKRetargetFactory())
    rc = unreal.IKRetargeterController.get_controller(rtg)
    rc.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, ikm)
    rc.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, ikv)
    try: rc.set_preview_mesh(unreal.RetargetSourceOrTarget.SOURCE, manny); rc.set_preview_mesh(unreal.RetargetSourceOrTarget.TARGET, sk)
    except Exception as e: log("preview mesh:", e)
    rc.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
    try:
        rc.auto_align_all_bones(unreal.RetargetSourceOrTarget.TARGET); log("pose alinhada automaticamente")
    except Exception as e: log("auto_align indisponivel:", e)
    EAL.save_loaded_asset(rtg)
    srcs = [unreal.load_asset("/Game/Characters/Mannequins/Animations/Manny/" + n) for n in ("MM_Idle","MM_Walk_InPlace","MM_Run_Fwd","MM_Jump","MM_Fall_Loop","MM_Land")]
    srcs = [s for s in srcs if s]
    res = unreal.IKRetargetBatchOperation.duplicate_and_retarget(srcs, manny, sk, rtg, "MM_", "VRT_", "", "", False)
    log("retarget gerou:", [r.get_path_name() for r in res] if res else res)
    for r in (res or []):
        if isinstance(r, unreal.AnimSequence):
            dst = "/Game/Vertice/Anims/" + r.get_name()
            if EAL.does_asset_exist(dst): EAL.delete_asset(dst)
            EAL.rename_asset(r.get_path_name().split(".")[0], dst)
            a = unreal.load_asset(dst)
            a.set_editor_property("enable_root_motion", False); a.set_editor_property("force_root_lock", True)
            EAL.save_loaded_asset(a); ok_anims.append(dst)
except Exception as e:
    log("ERRO retarget:", e)
log("animacoes:", ok_anims)

# ---------- 5. mapa de teste separado ----------
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
MAP = "/Game/Vertice/Maps/Teste_Vertice"
if EAL.does_asset_exist(MAP): EAL.delete_asset(MAP)
LES.new_level(MAP)
cube = unreal.load_asset("/Engine/BasicShapes/Cube")
grid = unreal.load_asset("/Engine/EngineMaterials/WorldGridMaterial")
fl = EAS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -50))
fl.static_mesh_component.set_static_mesh(cube); fl.set_actor_scale3d(unreal.Vector(60, 60, 1))
if grid: fl.static_mesh_component.set_material(0, grid)
fl.set_actor_label("Chao")
sun = EAS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 500), unreal.Rotator(0, -40, -30))
sun.light_component.set_editor_property("atmosphere_sun_light", True)
sun.light_component.set_intensity(6.0)
EAS.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0))
sky = EAS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 300))
sky.light_component.set_editor_property("real_time_capture", True); sky.light_component.set_intensity(1.0)
EAS.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 0))
EAS.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-600, 0, 100), unreal.Rotator(0, 0, 0))
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
gm = unreal.load_class(None, "/Script/JogoCoop.VerticeTestGameMode")
world.get_world_settings().set_editor_property("default_game_mode", gm)
log("game mode do mapa:", gm)
LES.save_current_level()
log("FIM OK")
