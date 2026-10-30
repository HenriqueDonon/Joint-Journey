# Cria /Game/Maps/Fase1_Arquitetura: Fase 1 sobre a arquitetura do experimento Higgsfield (3D Jutsu, rev.10, editada no Blender:
# Higgsfield_Fase1/jj_fase1_layout2.glb). Gameplay = CoopFase1GameMode em "layout 2": posicoes dos atores interativos vem dos
# marcadores F1L_* deste mapa. Nao toca nos mapas Fase1_Editavel / Fase1_Experimental / Fase2.
import unreal, os, json
proj = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
DST = '/Game/Maps/Fase1'   # Fase 1 oficial (promovida em 29/10/2026; antes /Game/Maps/Fase1_Arquitetura); DIR = '/Game/Fase1Arq/HF'
ell = unreal.EditorLevelLibrary; eal = unreal.EditorAssetLibrary
if eal.does_asset_exist(DST) and not os.path.exists(os.path.join(proj, 'MapaFase1Arq', 'RECRIAR.txt')):
    print('[F1A] JA_EXISTE'); raise SystemExit
# 1. importa a cena (glTF/Interchange): uma malha por objeto, vertices ja em coordenadas do mundo
if not eal.list_assets(DIR, recursive=True):
    t = unreal.AssetImportTask(); t.filename = os.path.join(proj, 'Higgsfield_Fase1', 'jj_fase1_layout2.glb')
    t.destination_path = DIR; t.automated = True; t.save = True; t.replace_existing = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
meshes = []
for p in eal.list_assets(DIR, recursive=True):
    a = unreal.load_asset(p)
    if isinstance(a, unreal.StaticMesh) and not a.get_name().startswith(('HF_', 'SM_HF_')): meshes.append(a)   # HF_*: copias soltas na origem (descartadas)
print('[F1A] malhas importadas:', len(meshes))
NOCOL = ('HAZ_embers', 'DET_hazard_stripes', 'DOOR_emblem')
for m in meshes:
    bs = m.get_editor_property('body_setup')
    if bs:
        bs.set_editor_property('collision_trace_flag', unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        eal.save_loaded_asset(m)
# 2. mapa novo
if eal.does_asset_exist(DST): ell.load_level(DST); [ell.destroy_actor(a) for a in ell.get_all_level_actors() if a.get_class().get_name() not in ('WorldSettings', 'Brush', 'DefaultPhysicsVolume', 'WorldDataLayers')]
else: ell.new_level(DST)
bb = {}
for m in meshes:
    a = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, 0))
    c = a.static_mesh_component; c.set_mobility(unreal.ComponentMobility.STATIC); c.set_static_mesh(m)
    nm = m.get_name().replace('SM_', '')
    a.set_actor_label('HF_' + nm); a.set_folder_path('Higgsfield/' + nm.split('_')[0])
    a.tags = [unreal.Name('F1Decor'), unreal.Name('F1Arq')]
    if any(k in nm for k in NOCOL): c.set_collision_profile_name('NoCollision')
    o, e = a.get_actor_bounds(False); bb[nm] = [round(o.x-e.x), round(o.x+e.x), round(o.y-e.y), round(o.y+e.y), round(o.z-e.z), round(o.z+e.z)]
print('[F1A] BOUNDS ' + json.dumps(bb))
# eixo Y: o pier (catwalk) fica em Y positivo no Blender; se veio negativo, a importacao espelhou Y
cw = [v for k, v in bb.items() if 'catwalk_end' in k]
SY = -1.0 if (cw and cw[0][3] < 0) else 1.0
print('[F1A] sinal Y', SY)
def mark(tag, x, y, z, sx=1, sy=1, sz=1):
    t = ell.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(x, y * SY, z))
    t.set_actor_scale3d(unreal.Vector(sx, sy, sz)); t.tags = [unreal.Name(tag)]; t.set_actor_label(tag); t.set_folder_path('Gameplay_Marcadores')
mark('F1L_Coluna', 1000, 600, 0)
mark('F1L_Ponte', 2000, 0, 0, 16.0)
mark('F1L_Porta', 4176, 0, 300)
for x, y in ((300, -550), (300, -400), (450, -475)): mark('F1L_Inicio', x, y, 100)
mark('F1L_Barreira', 2000, 0, 500, 0.3, 28.0, 11.0)
# estrutura sustentada pela coluna + colapso progressivo (substitui o deck simples F1L_Teto)
rig = ell.spawn_actor_from_class(unreal.load_class(None, '/Script/JogoCoop.CoopCollapseRig'), unreal.Vector(1000, 600 * SY, 0))
rig.set_actor_label('F1_EstruturaColuna_Colapso'); rig.set_folder_path('Gameplay_Marcadores')
def v2(x, y): return unreal.Vector2D(x, y)
ys = sorted((330 * SY, 1410 * SY)); rig.set_editor_property('deck_min', v2(0, ys[0])); rig.set_editor_property('deck_max', v2(1167, ys[1]))
# secao do teto = cobertura de toda a area de inicio (parede a parede), logo abaixo das trelicas
rig.set_editor_property('bay_min', v2(-250, -1400)); rig.set_editor_property('bay_max', v2(1240, 1400))
rig.set_editor_property('wall_y', 1400 * SY); rig.set_editor_property('bay_z', 1380.0); rig.set_editor_property('truss_z', 1545.0)
rig.set_editor_property('lose_cam_pos', unreal.Vector(1750, 1100 * SY * -1, 900)); rig.set_editor_property('lose_cam_target', unreal.Vector(450, -100 * SY * -1, 200))
# revestimento do tunel pos-fase (na posicao do marcador F1L_Porta)
td = ell.spawn_actor_from_class(unreal.load_class(None, '/Script/JogoCoop.CoopTunnelDressing'), unreal.Vector(4176, 0, 300))
td.set_actor_label('F1_TunelPosFase_Revestimento'); td.set_folder_path('Gameplay_Marcadores')
mark('F1L_Rachaduras', 600, 1396, 420, 12.0, 1.0, 5.0)
# 3. luzes do experimento (mesmas posicoes/cores), refeitas como luzes nativas
L = json.load(open(os.path.join(proj, 'Higgsfield_Fase1', 'luzes_layout2.json')))
for l in L:
    x, y, z = [v * 100 for v in l['p']]; col = l['c']
    if l['t'] == 'SPOT':
        d = l['dir']; rot = unreal.MathLibrary.make_rot_from_x(unreal.Vector(d[0], d[1] * SY, d[2]))
        a = ell.spawn_actor_from_class(unreal.SpotLight, unreal.Vector(x, y * SY, z), rot); lc = a.spot_light_component
        lc.set_editor_property('outer_cone_angle', min(80.0, l['cone'] * 0.5 + 5)); lc.set_editor_property('inner_cone_angle', l['cone'] * 0.3)
        lm = 9000.0; rad = 3200.0; sh = True
    else:
        a = ell.spawn_actor_from_class(unreal.PointLight, unreal.Vector(x, y * SY, z)); lc = a.point_light_component
        n = l['n']
        lm, rad = (60000.0, 2300.0) if 'fire' in n else (8000.0, 1400.0) if 'door' in n else (3000.0, 700.0) if 'spark' in n else (1500.0, 500.0)
        sh = False
    lc.set_editor_property('mobility', unreal.ComponentMobility.MOVABLE)
    lc.set_editor_property('intensity_units', unreal.LightUnits.LUMENS); lc.set_editor_property('intensity', lm)
    lc.set_editor_property('light_color', unreal.Color(int(min(1, col[2]) * 255), int(min(1, col[1]) * 255), int(min(1, col[0]) * 255), 255))
    lc.set_editor_property('attenuation_radius', rad); lc.set_editor_property('cast_shadows', sh)
    a.set_actor_label('HFLuz_' + l['n']); a.set_folder_path('Luzes_Higgsfield'); a.tags = [unreal.Name('F1Decor'), unreal.Name('F1Arq')]
print('[F1A] luzes', len(L))
# 4. modelos do Blender da Fase 1 que reforcam a catastrofe (fora do percurso, sem colisao para nao criar degraus)
def fase1(slot, x, y, z, yaw=0, s=1.0, col=False):
    m = unreal.load_asset('/Game/Fase1/%s/SM_%s' % (slot, slot))
    if not m: return
    a = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y * SY, z), unreal.Rotator(roll=0, pitch=0, yaw=yaw))
    c = a.static_mesh_component; c.set_static_mesh(m); a.set_actor_scale3d(unreal.Vector(s, s, s))
    c.set_collision_profile_name('BlockAll' if col else 'NoCollision')
    a.set_actor_label('F1_' + slot); a.set_folder_path('Modelos_Fase1'); a.tags = [unreal.Name('F1Decor'), unreal.Name('F1Arq')]
fase1('Pilha_Escombro', 700, 1150, 0, 20); fase1('Pilha_Escombro', 1050, -1150, 0, 160); fase1('Pilha_Escombro', 3300, -1150, 0, 75)
fase1('Viga_Quebrada', 500, 1250, 0, 35); fase1('Viga_Quebrada', 3500, 1150, 0, -20)
fase1('Estrutura_Aco', -100, -1250, 0, 0, 1.2, True); fase1('Estrutura_Aco', -100, 1250, 0, 0, 1.2, True)
fase1('Cabos', 1500, 1300, 0, 0); fase1('Pilar_Ruido', 3000, 1250, 0, 0, 1.0, True); fase1('Fachada_Queimada', 2900, -1390, 0, 0)
# 5. pos-processamento (frio x lava), mapa usa o GameMode da Fase 1
pp = ell.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(2000, 0, 500)); pp.set_editor_property('unbound', True)
st = pp.get_editor_property('settings')
for k, v in (('auto_exposure_bias', -1.0), ('vignette_intensity', 0.5), ('bloom_intensity', 0.6), ('ambient_occlusion_intensity', 0.7),
             ('color_contrast', unreal.Vector4(1.1, 1.1, 1.1, 1.0)), ('white_temp', 7200.0)):
    st.set_editor_property('override_' + k, True); st.set_editor_property(k, v)
pp.set_editor_property('settings', st); pp.tags = [unreal.Name('F1Decor')]
ell.get_editor_world().get_world_settings().set_editor_property('default_game_mode', unreal.load_class(None, '/Script/JogoCoop.CoopFase1GameMode'))
ell.save_current_level()
print('[F1A] CRIADO ' + DST)
