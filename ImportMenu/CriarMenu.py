# Cria o mapa /Game/Maps/MainMenu com o GameMode do menu principal (CoopFrontEndGameMode).
import unreal

unreal.EditorAssetLibrary.make_directory('/Game/Maps')
if not unreal.EditorAssetLibrary.does_asset_exist('/Game/Maps/MainMenu'):
    unreal.EditorLevelLibrary.new_level('/Game/Maps/MainMenu')
else:
    unreal.EditorLevelLibrary.load_level('/Game/Maps/MainMenu')

world = unreal.EditorLevelLibrary.get_editor_world()
ws = world.get_world_settings()
ws.set_editor_property('game_mode_override', unreal.load_class(None, '/Script/JogoCoop.CoopFrontEndGameMode'))

unreal.EditorLevelLibrary.save_current_level()
print('MAINMENU_OK')
