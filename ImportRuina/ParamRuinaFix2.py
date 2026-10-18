# A tentativa anterior (ParamRuina.py: Metal=0.7) deixou a Ruina ainda metalica/escura
# demais pra luz fraca da sala (skylight realtime capture intensidade 0.5).
# Reduz bem mais o metal e a aspereza minima pra sair do "preto fosco".
import unreal
MEL = unreal.MaterialEditingLibrary
mi = unreal.load_asset("/Game/Ruina/Materials/MI_Ruina")
for n, v in (("Metal", 0.08), ("AsperezaMin", 0.6), ("Especular", 0.3)):
    MEL.set_material_instance_scalar_parameter_value(mi, n, v)
unreal.EditorAssetLibrary.save_loaded_asset(mi)
unreal.log("[RUINA] params fix2 ok FIM OK")
print("PARAM_RUINA_FIX2_OK")
