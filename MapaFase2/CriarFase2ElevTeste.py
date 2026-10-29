# Cria/atualiza /Game/Maps/Fase2_ElevTeste: mapa ISOLADO de teste (elevador + portas + barreiras seletivas).
# Nao toca no Fase2_Blockout nem na Fase 1. v1 (so elevador) esta no commit 9806769.
# Se o mapa existir, so e refeito com MapaFase2/RECRIAR_ELEV.txt. Medidas em cm: piso Z=0, andar de separacao Z=600,
# cabine 250x250 em X 0..250 (centro 125,0). Topo: corredor NORTE (+Y) com barreira ROXA (Ruina), SUL (-Y) com barreira CIANO (Neon).
import unreal, os
proj = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
DST = '/Game/Maps/Fase2_ElevTeste'
ell = unreal.EditorLevelLibrary; eal = unreal.EditorAssetLibrary; mel = unreal.MaterialEditingLibrary
force = os.path.exists(os.path.join(proj, 'MapaFase2', 'RECRIAR_ELEV.txt'))
if eal.does_asset_exist(DST) and not force:
    print('[ELEV] JA_EXISTE (nada feito)'); raise SystemExit
# material translucido provisório (cor/opacidade por parametro)
MP = '/Game/Fase2/Materiais/M_F2_Barreira'
if not eal.does_asset_exist(MP):
    at = unreal.AssetToolsHelpers.get_asset_tools()
    m = at.create_asset('M_F2_Barreira', '/Game/Fase2/Materiais', unreal.Material, unreal.MaterialFactoryNew())
    m.set_editor_property('blend_mode', unreal.BlendMode.BLEND_TRANSLUCENT)
    m.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    m.set_editor_property('two_sided', True)
    c = mel.create_material_expression(m, unreal.MaterialExpressionVectorParameter, -400, 0)
    c.set_editor_property('parameter_name', 'Cor'); c.set_editor_property('default_value', unreal.LinearColor(0.65, 0.15, 1, 1))
    o = mel.create_material_expression(m, unreal.MaterialExpressionScalarParameter, -400, 250)
    o.set_editor_property('parameter_name', 'Opac'); o.set_editor_property('default_value', 0.35)
    mel.connect_material_property(c, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(o, '', unreal.MaterialProperty.MP_OPACITY)
    mel.recompile_material(m); eal.save_loaded_asset(m)
    print('[ELEV] material criado')
CUBE = unreal.load_asset('/Engine/BasicShapes/Cube')
M = {k: unreal.load_asset('/Game/Fase2/Materiais/MI_F2_' + k) for k in ('A', 'B', 'guia', 'parede')}
if eal.does_asset_exist(DST):
    ell.load_level(DST)
    kill = ('StaticMeshActor', 'PlayerStart', 'DirectionalLight', 'PostProcessVolume', 'CoopElevator', 'CoopElevTest', 'CoopRoleBarrier')
    for a in ell.get_all_level_actors():
        if a.get_class().get_name() in kill: ell.destroy_actor(a)
else:
    ell.new_level(DST)
def box(name, x0, x1, y0, y1, z0, z1, m, folder, col=True):
    a = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector((x0+x1)/2, (y0+y1)/2, (z0+z1)/2))
    c = a.static_mesh_component; c.set_static_mesh(CUBE); c.set_material(0, M[m])
    a.set_actor_scale3d(unreal.Vector((x1-x0)/100.0, (y1-y0)/100.0, (z1-z0)/100.0))
    a.set_actor_label(name); a.set_folder_path(folder)
    if not col: c.set_collision_profile_name('NoCollision')
    return a
# nivel inferior (igual v1)
box('Piso_Oeste', -1200, 0, -600, 600, -40, 0, 'A', 'Piso')
box('Piso_Leste_Baixo', 260, 1000, -600, 600, -40, 0, 'A', 'Piso')
box('Piso_Norte_Cabine', 0, 260, 135, 600, -40, 0, 'A', 'Piso')
box('Piso_Sul_Cabine', 0, 260, -600, -135, -40, 0, 'A', 'Piso')
box('Fundo_Poco', 0, 250, -125, 125, -240, -200, 'parede', 'Piso')
for i, (x, y) in enumerate([(-10, -135), (260, -135), (-10, 135), (260, 135)]):
    box('Guia_%d' % i, x-8, x+8, y-8, y+8, -200, 950, 'guia', 'Guias', col=False)
# andar de separacao (Z=600): dois corredores provisorios, um por porta
for s, nome in ((1, 'Norte_Ruina'), (-1, 'Sul_Neon')):
    y0, y1 = sorted((s*135, s*800))
    box('Corredor_' + nome + '_Piso', 0, 250, y0, y1, 560, 600, 'B', 'Separacao')
    box('Corredor_' + nome + '_ParedeW', -10, 0, y0, y1, 600, 900, 'parede', 'Separacao')
    box('Corredor_' + nome + '_ParedeE', 250, 260, y0, y1, 600, 900, 'parede', 'Separacao')
    ye0, ye1 = sorted((s*800, s*810))
    box('Corredor_' + nome + '_Fundo', -10, 260, ye0, ye1, 600, 900, 'parede', 'Separacao')
BCLS = unreal.load_class(None, '/Script/JogoCoop.CoopRoleBarrier')
for s, role, nome in ((1, unreal.CoopBarrierRole.RUINA, 'Barreira_Roxa_Ruina'), (-1, unreal.CoopBarrierRole.NEON, 'Barreira_Ciano_Neon')):
    b = ell.spawn_actor_from_class(BCLS, unreal.Vector(125, s*225, 750))
    b.set_editor_property('allowed_role', role); b.set_editor_property('size', unreal.Vector(250, 20, 300))
    b.set_actor_label(nome); b.set_folder_path('Separacao')
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
print('[ELEV] CRIADO v2 ' + DST)
