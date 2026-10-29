# Cria /Game/Maps/Fase2_ElevTeste: mapa ISOLADO de teste do elevador (nao toca no Fase2_Blockout nem na Fase 1).
# Medidas em cm. Piso inferior Z=0, patamar superior Z=600, cabine 250x250 em X 0..250 (centro 125,0).
import unreal, os
DST = '/Game/Maps/Fase2_ElevTeste'
ell = unreal.EditorLevelLibrary; eal = unreal.EditorAssetLibrary
if eal.does_asset_exist(DST):
    print('[ELEV] JA_EXISTE (nada feito)'); raise SystemExit
CUBE = unreal.load_asset('/Engine/BasicShapes/Cube')
M = {k: unreal.load_asset('/Game/Fase2/Materiais/MI_F2_' + k) for k in ('A', 'B', 'guia', 'parede')}
ell.new_level(DST)
def box(name, x0, x1, y0, y1, z0, z1, m, folder, col=True):
    a = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector((x0+x1)/2, (y0+y1)/2, (z0+z1)/2))
    c = a.static_mesh_component; c.set_static_mesh(CUBE); c.set_material(0, M[m])
    a.set_actor_scale3d(unreal.Vector((x1-x0)/100.0, (y1-y0)/100.0, (z1-z0)/100.0))
    a.set_actor_label(name); a.set_folder_path(folder)
    if not col: c.set_collision_profile_name('NoCollision')
    return a
box('Piso_Oeste', -1200, 0, -600, 600, -40, 0, 'A', 'Piso')
box('Piso_Leste_Baixo', 250, 1000, -600, 600, -40, 0, 'A', 'Piso')
box('Piso_Norte_Cabine', 0, 250, 125, 600, -40, 0, 'A', 'Piso')
box('Piso_Sul_Cabine', 0, 250, -600, -125, -40, 0, 'A', 'Piso')
box('Fundo_Poco', 0, 250, -125, 125, -240, -200, 'parede', 'Piso')
box('Patamar_Superior', 250, 1000, -600, 600, 560, 600, 'B', 'Piso')
for i, (x, y) in enumerate([(0, -125), (250, -125), (0, 125), (250, 125)]):
    box('Guia_%d' % i, x-8, x+8, y-8, y+8, -200, 900, 'guia', 'Guias', col=False)
for i, y in enumerate((-100, 100)):
    s = ell.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-600, y, 100)); s.set_actor_label('PlayerStart_%d' % (i+1))
elev = ell.spawn_actor_from_class(unreal.load_class(None, '/Script/JogoCoop.CoopElevator'), unreal.Vector(125, 0, 0))
elev.set_actor_label('Elevador')
ell.spawn_actor_from_class(unreal.load_class(None, '/Script/JogoCoop.CoopElevTest'), unreal.Vector(0, 0, 300)).set_actor_label('TesteAuto')
for pitch, yaw, lux, sh in ((-62, 28, 4.0, True), (-40, 208, 2.5, False), (-30, 100, 1.5, False)):
    d = ell.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 3000), unreal.Rotator(roll=0, pitch=pitch, yaw=yaw))
    d.light_component.set_editor_property('mobility', unreal.ComponentMobility.MOVABLE)
    d.light_component.set_editor_property('intensity', lux); d.light_component.set_editor_property('cast_shadows', sh)
pp = ell.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(0, 0, 300)); pp.set_editor_property('unbound', True)
st = pp.get_editor_property('settings')
st.set_editor_property('override_auto_exposure_min_brightness', True); st.set_editor_property('auto_exposure_min_brightness', 1.0)
st.set_editor_property('override_auto_exposure_max_brightness', True); st.set_editor_property('auto_exposure_max_brightness', 1.0)
st.set_editor_property('override_motion_blur_amount', True); st.set_editor_property('motion_blur_amount', 0.0)
pp.set_editor_property('settings', st)
ell.get_editor_world().get_world_settings().set_editor_property('default_game_mode', unreal.load_class(None, '/Script/JogoCoop.CoopFase2GameMode'))
ell.save_current_level()
print('[ELEV] CRIADO ' + DST)
