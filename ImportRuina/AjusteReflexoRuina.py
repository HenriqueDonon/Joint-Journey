# Reduz o reflexo espelhado da Ruina: aspereza minima e metal reduzido (parametros ajustaveis no MI_Ruina)
import unreal
EAL = unreal.EditorAssetLibrary; MEL = unreal.MaterialEditingLibrary
def log(*a): unreal.log("[RUINA] " + " ".join(str(x) for x in a))
m = unreal.load_asset("/Game/Ruina/Materials/M_Ruina")
ro = MEL.get_material_property_input_node(m, unreal.MaterialProperty.MP_ROUGHNESS)
mt = MEL.get_material_property_input_node(m, unreal.MaterialProperty.MP_METALLIC)
log("nos:", ro, mt)
def par(nome, v, x, y):
    p = MEL.create_material_expression(m, unreal.MaterialExpressionScalarParameter, x, y)
    p.set_editor_property("parameter_name", nome); p.set_editor_property("default_value", v); return p
# aspereza = lerp(AsperezaMin, 1, textura)
amin = par("AsperezaMin", 0.6, -500, 250)
um = MEL.create_material_expression(m, unreal.MaterialExpressionConstant, -500, 330); um.set_editor_property("r", 1.0)
lr = MEL.create_material_expression(m, unreal.MaterialExpressionLinearInterpolate, -300, 260)
MEL.connect_material_expressions(amin, "", lr, "A"); MEL.connect_material_expressions(um, "", lr, "B"); MEL.connect_material_expressions(ro, "R", lr, "Alpha")
MEL.connect_material_property(lr, "", unreal.MaterialProperty.MP_ROUGHNESS)
# metal = textura * Metal
fm = par("Metal", 0.25, -500, 520)
mm = MEL.create_material_expression(m, unreal.MaterialExpressionMultiply, -300, 500)
MEL.connect_material_expressions(mt, "R", mm, "A"); MEL.connect_material_expressions(fm, "", mm, "B")
MEL.connect_material_property(mm, "", unreal.MaterialProperty.MP_METALLIC)
# especular um pouco menor
sp = par("Especular", 0.35, -300, 650)
MEL.connect_material_property(sp, "", unreal.MaterialProperty.MP_SPECULAR)
MEL.recompile_material(m); EAL.save_loaded_asset(m)
mi = unreal.load_asset("/Game/Ruina/Materials/MI_Ruina")
MEL.update_material_instance(mi); EAL.save_loaded_asset(mi)
log("FIM OK")
