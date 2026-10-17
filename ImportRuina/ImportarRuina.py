# Importa o Ruina (FBX + texturas), cria materiais, IK Rig e retargeting das animacoes do Manny.
import unreal, os
DIR = os.path.dirname(os.path.abspath(__file__))
AT = unreal.AssetToolsHelpers.get_asset_tools()
EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary
def log(*a): unreal.log("[RUINA] " + " ".join(str(x) for x in a))

def task(src, dst, name, options=None):
    t = unreal.AssetImportTask()
    t.filename = os.path.join(DIR, src); t.destination_path = dst; t.destination_name = name
    t.automated = True; t.replace_existing = True; t.save = True
    if options: t.options = options
    AT.import_asset_tasks([t])
    return t.imported_object_paths

for d in ("/Game/Ruina/Anims", "/Game/Ruina/Rig", "/Game/Ruina/Mesh"):
    if EAL.does_directory_exist(d): EAL.delete_directory(d)

# ---------- 1. malha esqueletica ----------
ui = unreal.FbxImportUI()
ui.import_mesh = True; ui.import_as_skeletal = True; ui.import_animations = False
ui.import_materials = False; ui.import_textures = False; ui.create_physics_asset = True
ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
sd = ui.skeletal_mesh_import_data
sd.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
sd.set_editor_property("import_morph_targets", False)
sd.set_editor_property("convert_scene", True)
sd.set_editor_property("use_t0_as_ref_pose", False)
log("malha:", task("SK_Ruina.fbx", "/Game/Ruina/Mesh", "SK_Ruina", ui))
sk = unreal.load_asset("/Game/Ruina/Mesh/SK_Ruina")
log("bounds", sk.get_bounds().box_extent)

# ---------- 2. texturas ----------
tex = {}
for f, comp, srgb in (("T_Ruina_Basecolor.jpg", unreal.TextureCompressionSettings.TC_DEFAULT, True),
                      ("T_Ruina_Normal.png", unreal.TextureCompressionSettings.TC_NORMALMAP, False),
                      ("T_Ruina_Roughness.jpg", unreal.TextureCompressionSettings.TC_MASKS, False),
                      ("T_Ruina_Metallic.jpg", unreal.TextureCompressionSettings.TC_MASKS, False)):
    n = os.path.splitext(f)[0]
    task(f, "/Game/Ruina/Textures", n)
    t = unreal.load_asset("/Game/Ruina/Textures/" + n)
    t.set_editor_property("compression_settings", comp); t.set_editor_property("srgb", srgb)
    EAL.save_loaded_asset(t); tex[n] = t
log("texturas:", list(tex))

# ---------- 3. material (emissivo derivado do ciano do basecolor) ----------
p = "/Game/Ruina/Materials/M_Ruina"
if EAL.does_asset_exist(p): EAL.delete_asset(p)
m = AT.create_asset("M_Ruina", "/Game/Ruina/Materials", unreal.Material, unreal.MaterialFactoryNew())
m.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
def ts(t, x, y, st):
    e = MEL.create_material_expression(m, unreal.MaterialExpressionTextureSample, x, y)
    e.set_editor_property("texture", t); e.set_editor_property("sampler_type", st); return e
bc = ts(tex["T_Ruina_Basecolor"], -900, -300, unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
nm = ts(tex["T_Ruina_Normal"], -900, 0, unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
ro = ts(tex["T_Ruina_Roughness"], -900, 250, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
mt = ts(tex["T_Ruina_Metallic"], -900, 500, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
def mask(src, x, y, r=False, g=False, b=False):
    e = MEL.create_material_expression(m, unreal.MaterialExpressionComponentMask, x, y)
    e.set_editor_property("r", r); e.set_editor_property("g", g); e.set_editor_property("b", b); e.set_editor_property("a", False)
    MEL.connect_material_expressions(src, "RGB", e, "")
    return e
mb = mask(bc, -650, 700, b=True)
mg = mask(bc, -650, 820, g=True)
sub0 = MEL.create_material_expression(m, unreal.MaterialExpressionSubtract, -560, 760)
MEL.connect_material_expressions(mb, "", sub0, "A"); MEL.connect_material_expressions(mg, "", sub0, "B")
sub = MEL.create_material_expression(m, unreal.MaterialExpressionSubtract, -480, 760)
sub.set_editor_property("const_b", 0.35)
MEL.connect_material_expressions(sub0, "", sub, "A")
ganho = MEL.create_material_expression(m, unreal.MaterialExpressionScalarParameter, -480, 900)
ganho.set_editor_property("parameter_name", "Corte"); ganho.set_editor_property("default_value", 6.0)
mul1 = MEL.create_material_expression(m, unreal.MaterialExpressionMultiply, -330, 800)
MEL.connect_material_expressions(sub, "", mul1, "A"); MEL.connect_material_expressions(ganho, "", mul1, "B")
sat = MEL.create_material_expression(m, unreal.MaterialExpressionClamp, -200, 800)
MEL.connect_material_expressions(mul1, "", sat, "")
mul2 = MEL.create_material_expression(m, unreal.MaterialExpressionMultiply, -80, 700)
MEL.connect_material_expressions(bc, "RGB", mul2, "A"); MEL.connect_material_expressions(sat, "", mul2, "B")
brilho = MEL.create_material_expression(m, unreal.MaterialExpressionScalarParameter, -200, 1000)
brilho.set_editor_property("parameter_name", "Brilho"); brilho.set_editor_property("default_value", 3.0)
mul3 = MEL.create_material_expression(m, unreal.MaterialExpressionMultiply, 60, 800)
MEL.connect_material_expressions(mul2, "", mul3, "A"); MEL.connect_material_expressions(brilho, "", mul3, "B")
MEL.connect_material_property(bc, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
MEL.connect_material_property(nm, "RGB", unreal.MaterialProperty.MP_NORMAL)
MEL.connect_material_property(ro, "R", unreal.MaterialProperty.MP_ROUGHNESS)
MEL.connect_material_property(mt, "R", unreal.MaterialProperty.MP_METALLIC)
MEL.connect_material_property(mul3, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
m.set_editor_property("used_with_skeletal_mesh", True)
MEL.recompile_material(m); EAL.save_loaded_asset(m)
pi = "/Game/Ruina/Materials/MI_Ruina"
if EAL.does_asset_exist(pi): EAL.delete_asset(pi)
mi = AT.create_asset("MI_Ruina", "/Game/Ruina/Materials", unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
MEL.set_material_instance_parent(mi, m)
MEL.set_material_instance_scalar_parameter_value(mi, "Brilho", 3.0)
MEL.set_material_instance_scalar_parameter_value(mi, "Corte", 6.0)
EAL.save_loaded_asset(mi)
mats = []
for sm in sk.get_editor_property("materials"):
    mats.append(unreal.SkeletalMaterial(material_interface=mi, material_slot_name=sm.get_editor_property("material_slot_name")))
sk.set_editor_property("materials", mats); EAL.save_loaded_asset(sk)
log("materiais ok")

# ---------- 4. IK Rig + retarget ----------
def mk(name, path, cls, fac):
    q = path + "/" + name
    if EAL.does_asset_exist(q): EAL.delete_asset(q)
    return AT.create_asset(name, path, cls, fac)
ikn = mk("IK_Ruina", "/Game/Ruina/Rig", unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
c = unreal.IKRigController.get_controller(ikn)
c.set_skeletal_mesh(sk)
c.apply_auto_generated_retarget_definition()
log("cadeias:", [str(ch.chain_name) for ch in c.get_retarget_chains()], "raiz:", c.get_retarget_root())
EAL.save_loaded_asset(ikn)
ikm = unreal.load_asset("/Game/Characters/Mannequins/Rigs/IK_Mannequin")
manny = unreal.load_asset("/Game/Characters/Mannequins/Meshes/SKM_Manny")
rtg = mk("RTG_Manny_para_Ruina", "/Game/Ruina/Rig", unreal.IKRetargeter, unreal.IKRetargetFactory())
rc = unreal.IKRetargeterController.get_controller(rtg)
rc.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, ikm)
rc.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, ikn)
try:
    rc.set_preview_mesh(unreal.RetargetSourceOrTarget.SOURCE, manny); rc.set_preview_mesh(unreal.RetargetSourceOrTarget.TARGET, sk)
except Exception as e: log("preview:", e)
rc.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
try: rc.auto_align_all_bones(unreal.RetargetSourceOrTarget.TARGET); log("pose alinhada")
except Exception as e: log("auto_align:", e)
EAL.save_loaded_asset(rtg)
log("FIM OK")
