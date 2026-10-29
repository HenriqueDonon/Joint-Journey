import unreal, json
eal = unreal.EditorAssetLibrary
out = {}
for p in eal.list_assets('/Game/Fase1Arq/HF', recursive=True):
    a = unreal.load_asset(p)
    if isinstance(a, unreal.MaterialInstanceConstant):
        d = {'parent': a.get_editor_property('parent').get_name() if a.get_editor_property('parent') else None}
        for v in a.get_editor_property('vector_parameter_values'): d[str(v.parameter_info.name)] = [round(v.parameter_value.r,2), round(v.parameter_value.g,2), round(v.parameter_value.b,2)]
        for v in a.get_editor_property('scalar_parameter_values'): d[str(v.parameter_info.name)] = round(v.parameter_value,2)
        out[a.get_name()] = d
print('[MAT] ' + json.dumps(out))
