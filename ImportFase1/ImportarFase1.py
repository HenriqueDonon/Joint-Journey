# Importa todos os props da Fase 1 (cada subpasta: SM_<Nome>.fbx + T_<Nome>_basecolor/normal/roughness/metallic.png)
# para /Game/Fase1/<Nome>, com material instanciado de um mestre unico (M_Fase1_Prop). Pode rodar varias vezes.
import unreal, os
DIR = os.path.dirname(os.path.abspath(__file__))
AT = unreal.AssetToolsHelpers.get_asset_tools(); EAL = unreal.EditorAssetLibrary; MEL = unreal.MaterialEditingLibrary
def log(*a): unreal.log("[FASE1] " + " ".join(str(x) for x in a))

def task(path, dst, name, options=None):
    t = unreal.AssetImportTask(); t.filename = path; t.destination_path = dst; t.destination_name = name
    t.automated = True; t.replace_existing = True; t.save = True
    if options: t.options = options
    AT.import_asset_tasks([t]); return t.imported_object_paths

jobs = []
for sub in sorted(os.listdir(DIR)):
    folder = os.path.join(DIR, sub)
    if not os.path.isdir(folder): continue
    fbx = [f for f in os.listdir(folder) if f.startswith("SM_") and f.lower().endswith(".fbx")]
    if not fbx: continue
    name = fbx[0][3:-4]; dst = "/Game/Fase1/" + name
    ui = unreal.FbxImportUI()
    ui.import_mesh = True; ui.import_as_skeletal = False; ui.import_materials = False; ui.import_textures = False
    ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    sd = ui.static_mesh_import_data
    sd.set_editor_property("auto_generate_collision", True); sd.set_editor_property("combine_meshes", True)
    task(os.path.join(folder, fbx[0]), dst, "SM_" + name, ui)
    tex = {}
    for kind, comp, srgb in (("basecolor", unreal.TextureCompressionSettings.TC_DEFAULT, True),
                             ("normal", unreal.TextureCompressionSettings.TC_NORMALMAP, False),
                             ("roughness", unreal.TextureCompressionSettings.TC_MASKS, False),
                             ("metallic", unreal.TextureCompressionSettings.TC_MASKS, False)):
        f = os.path.join(folder, "T_%s_%s.png" % (name, kind))
        if not os.path.exists(f): continue
        tn = "T_%s_%s" % (name, kind); task(f, dst, tn)
        t = unreal.load_asset(dst + "/" + tn)
        t.set_editor_property("compression_settings", comp); t.set_editor_property("srgb", srgb); EAL.save_loaded_asset(t)
        tex[kind] = t
    jobs.append((name, dst, tex))


# material mestre V2 (o V1 nao compilava: parametros de textura sem textura padrao -> material padrao cinza no jogo)
MP = "/Game/Fase1/M_Fase1_PropV2"
if EAL.does_asset_exist(MP): EAL.delete_asset(MP)
ref = next((t for _, _, t in jobs if len(t) == 4), None)
m = AT.create_asset("M_Fase1_PropV2", "/Game/Fase1", unreal.Material, unreal.MaterialFactoryNew())
def p(name, x, y, st, tx):
    e = MEL.create_material_expression(m, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
    e.set_editor_property("parameter_name", name); e.set_editor_property("sampler_type", st)
    if tx: e.set_editor_property("texture", tx)
    return e
bc = p("BaseColor", -700, -300, unreal.MaterialSamplerType.SAMPLERTYPE_COLOR, ref["basecolor"])
nm = p("Normal", -700, 0, unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL, ref["normal"])
ro = p("Roughness", -700, 250, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS, ref["roughness"])
mt = p("Metallic", -700, 500, unreal.MaterialSamplerType.SAMPLERTYPE_MASKS, ref["metallic"])
eb = MEL.create_material_expression(m, unreal.MaterialExpressionScalarParameter, -700, -500)
eb.set_editor_property("parameter_name", "EmissiveBoost"); eb.set_editor_property("default_value", 0.0)
mu = MEL.create_material_expression(m, unreal.MaterialExpressionMultiply, -300, -400)
MEL.connect_material_expressions(bc, "RGB", mu, "A"); MEL.connect_material_expressions(eb, "", mu, "B")
MEL.connect_material_property(bc, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
MEL.connect_material_property(mu, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
MEL.connect_material_property(nm, "RGB", unreal.MaterialProperty.MP_NORMAL)
MEL.connect_material_property(ro, "R", unreal.MaterialProperty.MP_ROUGHNESS)
MEL.connect_material_property(mt, "R", unreal.MaterialProperty.MP_METALLIC)
MEL.recompile_material(m); EAL.save_loaded_asset(m)
master = unreal.load_asset(MP)
GLOW = {"Lava": 2.5}

for name, dst, tex in jobs:
    mip = dst + "/MI_" + name
    if EAL.does_asset_exist(mip): EAL.delete_asset(mip)
    mi = AT.create_asset("MI_" + name, dst, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    MEL.set_material_instance_parent(mi, master)
    for kind, pn in (("basecolor", "BaseColor"), ("normal", "Normal"), ("roughness", "Roughness"), ("metallic", "Metallic")):
        if kind in tex: MEL.set_material_instance_texture_parameter_value(mi, pn, tex[kind])
    if name in GLOW: MEL.set_material_instance_scalar_parameter_value(mi, "EmissiveBoost", GLOW[name])
    EAL.save_loaded_asset(mi)
    sm = unreal.load_asset(dst + "/SM_" + name)
    for i in range(len(sm.static_materials)): sm.set_material(i, mi)
    EAL.save_loaded_asset(sm)
    log("ok", name, sm.get_bounding_box())
log("FIM")
