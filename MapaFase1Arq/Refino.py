# Refino da Fase1_Arquitetura (sem recriar o mapa): secao do teto maior, camera de derrota e revestimento do tunel.
import unreal
ell = unreal.EditorLevelLibrary
ell.load_level('/Game/Maps/Fase1')
acts = ell.get_all_level_actors()
rig = [a for a in acts if a.get_class().get_name() == 'CoopCollapseRig'][0]
def v2(x, y): return unreal.Vector2D(x, y)
SY = -1.0 if rig.get_actor_location().y < 0 else 1.0
rig.set_editor_property('bay_min', v2(-250, -1400)); rig.set_editor_property('bay_max', v2(1240, 1400))
rig.set_editor_property('bay_z', 1380.0); rig.set_editor_property('truss_z', 1545.0)
rig.set_editor_property('lose_cam_pos', unreal.Vector(1750, 1100 * -SY, 900)); rig.set_editor_property('lose_cam_target', unreal.Vector(450, -100 * -SY, 200))
if not [a for a in acts if a.get_class().get_name() == 'CoopTunnelDressing']:
    door = [a for a in acts if unreal.Name('F1L_Porta') in a.tags][0]
    td = ell.spawn_actor_from_class(unreal.load_class(None, '/Script/JogoCoop.CoopTunnelDressing'), door.get_actor_location())
    td.set_actor_label('F1_TunelPosFase_Revestimento'); td.set_folder_path('Gameplay_Marcadores')
print('[REFINO] rig', rig.get_editor_property('bay_min'), rig.get_editor_property('bay_max'), 'SY', SY)
ell.save_current_level()
print('[REFINO] OK')
