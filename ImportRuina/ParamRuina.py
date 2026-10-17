import unreal, sys
MEL = unreal.MaterialEditingLibrary
mi = unreal.load_asset("/Game/Ruina/Materials/MI_Ruina")
for n, v in (("Metal", 0.7), ("AsperezaMin", 0.45), ("Especular", 0.35)):
    MEL.set_material_instance_scalar_parameter_value(mi, n, v)
unreal.EditorAssetLibrary.save_loaded_asset(mi)
unreal.log("[RUINA] params ok FIM OK")
