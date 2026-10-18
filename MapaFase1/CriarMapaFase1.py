# Cria o mapa de teste /Game/Maps/Fase1_Teste (GameMode = CoopFase1GameMode). NAO mexe no Sala1.
import unreal
unreal.EditorAssetLibrary.make_directory('/Game/Maps')
if not unreal.EditorAssetLibrary.does_asset_exist('/Game/Maps/Fase1_Teste'):
    unreal.EditorLevelLibrary.new_level('/Game/Maps/Fase1_Teste')
else:
    unreal.EditorLevelLibrary.load_level('/Game/Maps/Fase1_Teste')
ws = unreal.EditorLevelLibrary.get_editor_world().get_world_settings()
ws.set_editor_property('default_game_mode', unreal.load_class(None, '/Script/JogoCoop.CoopFase1GameMode'))
unreal.EditorLevelLibrary.save_current_level()
print('FASE1_MAPA_OK')
