# Cria o mapa vazio /Game/Maps/Sala1 e o material base /Game/Materials/M_JJ (cor, emissivo, rugosidade, metal).
import unreal

unreal.EditorAssetLibrary.make_directory('/Game/Maps')
if not unreal.EditorAssetLibrary.does_asset_exist('/Game/Maps/Sala1'):
    unreal.EditorLevelLibrary.new_level('/Game/Maps/Sala1')
    unreal.EditorLevelLibrary.save_current_level()

unreal.EditorAssetLibrary.make_directory('/Game/Materials')
if not unreal.EditorAssetLibrary.does_asset_exist('/Game/Materials/M_JJ'):
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    mel = unreal.MaterialEditingLibrary
    mat = tools.create_asset('M_JJ', '/Game/Materials', unreal.Material, unreal.MaterialFactoryNew())

    def vparam(name, default, y):
        e = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -500, y)
        e.set_editor_property('parameter_name', name)
        e.set_editor_property('default_value', default)
        return e

    def sparam(name, default, y):
        e = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -500, y)
        e.set_editor_property('parameter_name', name)
        e.set_editor_property('default_value', default)
        return e

    c = vparam('Color', unreal.LinearColor(0.5, 0.5, 0.5, 1.0), 0)
    em = vparam('Emissive', unreal.LinearColor(0.0, 0.0, 0.0, 1.0), 200)
    ro = sparam('Roughness', 0.8, 400)
    me = sparam('Metallic', 0.0, 500)
    mel.connect_material_property(c, '', unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(em, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(ro, '', unreal.MaterialProperty.MP_ROUGHNESS)
    mel.connect_material_property(me, '', unreal.MaterialProperty.MP_METALLIC)
    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset('/Game/Materials/M_JJ')
    unreal.log('M_JJ criado')
