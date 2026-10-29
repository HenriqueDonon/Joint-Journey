# Cria /Game/Maps/Fase2_Blockout (blockout do Poco dos Contrapesos). So cubos + materiais basicos. NAO sobrescreve se ja existir
# (crie MapaFase2/RECRIAR.txt para refazer). Medidas em cm. Niveis (superficie): A=0, B=600, C=1200, D=1800.
import unreal, os
proj = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
DST = '/Game/Maps/Fase2_Blockout'
ell = unreal.EditorLevelLibrary
eal = unreal.EditorAssetLibrary
if eal.does_asset_exist(DST) and not os.path.exists(os.path.join(proj, 'MapaFase2', 'RECRIAR.txt')):
    print('[FASE2] JA_EXISTE (nada feito)'); raise SystemExit
CUBE = unreal.load_asset('/Engine/BasicShapes/Cube')
BASE = unreal.load_asset('/Engine/BasicShapes/BasicShapeMaterial')
MATS = {}
def mat(name, r, g, b):
    if name in MATS: return MATS[name]
    path = '/Game/Fase2/Materiais/MI_F2_' + name
    if eal.does_asset_exist(path): m = unreal.load_asset(path)
    else:
        at = unreal.AssetToolsHelpers.get_asset_tools()
        m = at.create_asset('MI_F2_' + name, '/Game/Fase2/Materiais', unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        m.set_editor_property('parent', BASE)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(m, 'Color', unreal.LinearColor(r, g, b, 1))
    unreal.MaterialEditingLibrary.update_material_instance(m)
    eal.save_loaded_asset(m)
    MATS[name] = m
    return m
C = {'parede': (.42, .43, .46), 'trilho': (.12, .12, .13), 'A': (.28, .50, .32), 'B': (.62, .52, .18), 'Cc': (.20, .36, .62),
     'D': (.45, .25, .60), 'guia': (.85, .85, .85), 'elev': (.95, .45, .10), 'contra': (.75, .20, .55), 'trava': (.85, .10, .10),
     'suporte': (.10, .30, .90), 'console': (.95, .85, .15), 'mec': (.15, .70, .75), 'saida': (.95, .95, .95)}
for k, v in C.items(): mat(k, *v)

if eal.does_asset_exist(DST):
    ell.load_level(DST)
    for a in ell.get_all_level_actors():
        if any(str(t) == 'F2Blockout' for t in a.tags): ell.destroy_actor(a)
else:
    ell.new_level(DST)

def box(name, x0, x1, y0, y1, z0, z1, m, folder, col=True, tags=()):
    a = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector((x0+x1)/2, (y0+y1)/2, (z0+z1)/2))
    c = a.static_mesh_component
    c.set_static_mesh(CUBE)
    c.set_material(0, MATS[m])
    a.set_actor_scale3d(unreal.Vector((x1-x0)/100.0, (y1-y0)/100.0, (z1-z0)/100.0))
    a.set_actor_label(name)
    a.set_folder_path(folder)
    a.tags = [unreal.Name('F2Blockout')] + [unreal.Name(t) for t in tags]
    if not col: c.set_collision_profile_name('NoCollision')
    return a
def slab(name, x0, x1, y0, y1, ztop, m, folder='Niveis'): return box(name, x0, x1, y0, y1, ztop-40, ztop, m, folder)
def rail(name, x0, y0, x1, y1, z):   # guarda provisoria: 10cm de espessura, 110cm de altura
    if x0 == x1: x0, x1 = x0-5, x1+5
    if y0 == y1: y0, y1 = y0-5, y1+5
    box(name, x0, x1, y0, y1, z, z+110, 'trilho', 'Guardas')
def text(label, loc, yaw, size=70, color=(1, 1, 1)):
    t = ell.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(*loc), unreal.Rotator(roll=0, pitch=0, yaw=yaw))
    t.text_render.set_editor_property('text', label)
    t.text_render.set_editor_property('world_size', size)
    t.text_render.set_editor_property('text_render_color', unreal.Color(int(color[0]*255), int(color[1]*255), int(color[2]*255), 255))
    t.text_render.set_editor_property('horizontal_alignment', unreal.HorizTextAligment.EHTA_CENTER)
    t.tags = [unreal.Name('F2Blockout')]; t.set_folder_path('Rotulos'); t.set_actor_label('Rotulo')

# ---- envelope: X 0..2400, Y -900..900, Z 0..2200 (aberto em cima para a luz e para a vista superior) ----
slab('A_Piso_Nivel0', 0, 2400, -900, 900, 0, 'A')
box('Parede_Sul', 0, 2400, -940, -900, -40, 2200, 'parede', 'Paredes', tags=('CorteSul',))
box('Parede_Norte', 0, 2400, 900, 940, -40, 2200, 'parede', 'Paredes')
box('Parede_Oeste', -40, 0, -940, 940, -40, 2200, 'parede', 'Paredes')
# parede leste com abertura provisoria da saida (Y -150..150, Z 1800..2100)
box('Parede_Leste_S', 2400, 2440, -940, -150, -40, 2200, 'parede', 'Paredes')
box('Parede_Leste_N', 2400, 2440, 150, 940, -40, 2200, 'parede', 'Paredes')
box('Parede_Leste_Baixo', 2400, 2440, -150, 150, -40, 1800, 'parede', 'Paredes')
box('Parede_Leste_Topo', 2400, 2440, -150, 150, 2100, 2200, 'parede', 'Paredes')
# ---- B: nivel intermediario Z=600 (duas plataformas separadas pelo poco + passarela provisoria) ----
slab('B1_PlataformaOeste', 150, 950, -400, 400, 600, 'B', 'Niveis')
slab('B2_PlataformaLeste', 1450, 2250, -400, 400, 600, 'B', 'Niveis')
slab('B_Passarela', 850, 1550, 300, 500, 600, 'B', 'Niveis')
rail('B1_S', 150, -400, 950, -400, 600); rail('B1_N', 150, 400, 850, 400, 600); rail('B1_O', 150, -400, 150, 400, 600)
rail('B1_L1', 950, -400, 950, -150, 600); rail('B1_L2', 950, 150, 950, 300, 600)
rail('B2_S', 1450, -400, 2250, -400, 600); rail('B2_N', 1550, 400, 2250, 400, 600); rail('B2_L', 2250, -400, 2250, 400, 600)
rail('B2_O1', 1450, -400, 1450, -150, 600); rail('B2_O2', 1450, 150, 1450, 300, 600)
rail('BP_S', 950, 300, 1450, 300, 600); rail('BP_N', 850, 500, 1550, 500, 600)
# ---- C: nivel superior Z=1200 (plataforma de manutencao, borda do elevador principal) ----
slab('C_PlataformaManutencao', 150, 950, -600, 600, 1200, 'Cc', 'Niveis')
rail('C_S', 150, -600, 950, -600, 1200); rail('C_N', 150, 600, 950, 600, 1200); rail('C_O', 150, -600, 150, 600, 1200)
rail('C_L1', 950, -600, 950, -150, 1200); rail('C_L2', 950, 150, 950, 600, 1200)
# ---- D: saida Z=1800 ----
slab('D_PlataformaSaida', 1450, 2400, -600, 600, 1800, 'D', 'Niveis')
slab('D_Soleira', 2400, 2600, -150, 150, 1800, 'D', 'Niveis')
rail('D_S', 1450, -600, 2400, -600, 1800); rail('D_N', 1450, 600, 2400, 600, 1800); rail('D_O1', 1450, -600, 1450, -150, 1800); rail('D_O2', 1450, 150, 1450, 600, 1800)
rail('D_Soleira_S', 2400, -150, 2600, -150, 1800); rail('D_Soleira_N', 2400, 150, 2600, 150, 1800)
# ---- guias do poco central 500x500 (X 950..1450, Y -250..250): 4 prumos sem colisao, vao livre ate Z=2200 ----
for i, (x, y) in enumerate([(950, -250), (1450, -250), (950, 250), (1450, 250)]):
    box('Poco_Guia_%d' % i, x-10, x+10, y-10, y+10, 0, 2200, 'guia', 'Poco', col=False)
# ---- placeholders (SEM logica/movimento) ----
box('P1_ElevadorCarga', 1075, 1325, -125, 125, 0, 300, 'elev', 'Placeholders_P1')
box('P1_Contrapeso_Reserva', 1360, 1440, 160, 240, 0, 600, 'contra', 'Placeholders_P1')
box('P1_Trava_Destrutivel', 750, 850, -100, 100, 0, 150, 'trava', 'Placeholders_P1')
box('P1_Suporte_Reconstruivel', 750, 850, 300, 400, 0, 100, 'suporte', 'Placeholders_P1')
box('P1_Entrada_Marca', 100, 500, -800, -400, 0, 6, 'saida', 'Placeholders_P1', col=False)
box('P2_Mecanismo_Contrapeso', 1100, 1300, 650, 850, 0, 1100, 'contra', 'Placeholders_P2')
box('P2_Console_B1', 300, 400, -50, 50, 600, 750, 'console', 'Placeholders_P2')
box('P2_Console_B2', 2000, 2100, -50, 50, 600, 750, 'console', 'Placeholders_P2')
box('P3_Mecanismo_Final', 250, 450, 400, 560, 1200, 1450, 'mec', 'Placeholders_P3')
box('P3_ElevadorPrincipal_Reserva', 1075, 1325, -125, 125, 1200, 1500, 'mec', 'Placeholders_P3', col=False)
box('D_Saida_Marca', 2300, 2400, -150, 150, 1800, 1806, 'saida', 'Placeholders_D', col=False)
# rotulos (frente para +X na parede oeste, e para -Y... ver capturas)
text('A | NIVEL INFERIOR - PUZZLE 1 (elevador travado)', (20, -600, 250), 0, 55, (.6, 1, .6))
text('B | NIVEL INTERMEDIARIO - PUZZLE 2 (balanca)', (20, -300, 850), 0, 55, (1, .9, .4))
text('C | NIVEL SUPERIOR - PUZZLE 3 (ultima subida)', (20, 0, 1450), 0, 55, (.5, .7, 1))
text('D | SAIDA SUPERIOR', (2380, 0, 1990), 180, 55, (1, .8, 1))
text('POCO CENTRAL 500x500', (1200, 0, 900), 180, 70)
# ---- inicio, luz e configuracao do mapa ----
for i, y in enumerate((-650, -550)):
    s = ell.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(300, y, 100))
    s.tags = [unreal.Name('F2Blockout')]; s.set_actor_label('PlayerStart_%d' % (i+1)); s.set_folder_path('Inicio')
d = ell.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 3000), unreal.Rotator(roll=0, pitch=-62, yaw=28))
d.light_component.set_editor_property('mobility', unreal.ComponentMobility.MOVABLE)
d.light_component.set_editor_property('intensity', 4.0)
d.tags = [unreal.Name('F2Blockout')]; d.set_folder_path('Luz')
d2 = ell.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 3000), unreal.Rotator(roll=0, pitch=-40, yaw=208))
d2.light_component.set_editor_property('mobility', unreal.ComponentMobility.MOVABLE)
d2.light_component.set_editor_property('intensity', 2.5)
d2.light_component.set_editor_property('cast_shadows', False)
d2.tags = [unreal.Name('F2Blockout')]; d2.set_folder_path('Luz')
d3 = ell.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 3000), unreal.Rotator(roll=0, pitch=-30, yaw=100))
d3.light_component.set_editor_property('mobility', unreal.ComponentMobility.MOVABLE)
d3.light_component.set_editor_property('intensity', 1.5)
d3.light_component.set_editor_property('cast_shadows', False)
d3.tags = [unreal.Name('F2Blockout')]; d3.set_folder_path('Luz')
pp = ell.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(1200, 0, 1100))
pp.set_editor_property('unbound', True)
st = pp.get_editor_property('settings')
st.set_editor_property('override_auto_exposure_min_brightness', True); st.set_editor_property('auto_exposure_min_brightness', 1.0)
st.set_editor_property('override_auto_exposure_max_brightness', True); st.set_editor_property('auto_exposure_max_brightness', 1.0)
st.set_editor_property('override_motion_blur_amount', True); st.set_editor_property('motion_blur_amount', 0.0)
st.set_editor_property('override_bloom_intensity', True); st.set_editor_property('bloom_intensity', 0.0)
pp.set_editor_property('settings', st)
pp.tags = [unreal.Name('F2Blockout')]; pp.set_folder_path('Luz')
ws = ell.get_editor_world().get_world_settings()
ws.set_editor_property('default_game_mode', unreal.load_class(None, '/Script/JogoCoop.CoopFase2GameMode'))
unreal.EditorLevelLibrary.save_current_level()
print('[FASE2] CRIADO ' + DST)
