# Cria /Game/Maps/Fase2_Funcional: copia EXPERIMENTAL e revisada do Poco dos Contrapesos (Tarefa 05). Nao toca no Fase2_Blockout
# nem na Fase 1. Refaz so com MapaFase2/RECRIAR_FUNC.txt. Medidas em cm; niveis A=0, B=600, C=1200, D=1800 (iguais ao blockout);
# poco central 500x500 mantido (X 950..1450, Y -250..250). Modulos visuais do Blender: MapaFase2/Modulos/*.fbx -> /Game/Fase2/Modulos.
import unreal, os, math
proj = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
DST = '/Game/Maps/Fase2_Funcional'
ell = unreal.EditorLevelLibrary; eal = unreal.EditorAssetLibrary
if eal.does_asset_exist(DST) and not os.path.exists(os.path.join(proj, 'MapaFase2', 'RECRIAR_FUNC.txt')):
    print('[F2F] JA_EXISTE'); raise SystemExit
# 1) modulos do Blender (pivo na base; colisao gerada na importacao)
MODS = {}
for n in ('F2M_Guarda_200', 'F2M_Trelica_400', 'F2M_Painel_200x300', 'F2M_Grade_200', 'F2M_Tubo_400', 'F2M_Pilar_600'):
    p = '/Game/Fase2/Modulos/' + n
    if not eal.does_asset_exist(p):
        t = unreal.AssetImportTask(); t.filename = os.path.join(proj, 'MapaFase2', 'Modulos', n + '.fbx')
        t.destination_path = '/Game/Fase2/Modulos'; t.automated = True; t.save = True; t.replace_existing = True
        o = unreal.FbxImportUI(); o.import_mesh = True; o.import_materials = True; o.import_textures = False; o.import_as_skeletal = False
        o.static_mesh_import_data.combine_meshes = True; o.static_mesh_import_data.auto_generate_collision = True
        t.options = o
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
    MODS[n] = unreal.load_asset(p)
print('[F2F] modulos', {k: bool(v) for k, v in MODS.items()})
CUBE = unreal.load_asset('/Engine/BasicShapes/Cube'); SPH = unreal.load_asset('/Engine/BasicShapes/Sphere')
M = {k: unreal.load_asset('/Game/Fase2/Materiais/MI_F2_' + k) for k in ('parede', 'trilho', 'A', 'B', 'Cc', 'D', 'guia', 'elev', 'contra', 'trava', 'suporte', 'console', 'mec', 'saida')}
GLASS = unreal.load_asset('/Game/Fase2/Materiais/M_F2_Barreira')
if eal.does_asset_exist(DST):
    ell.load_level(DST)
    for a in ell.get_all_level_actors():
        if a.get_class().get_name() not in ('WorldSettings', 'Brush', 'DefaultPhysicsVolume', 'WorldDataLayers'): ell.destroy_actor(a)
else:
    ell.new_level(DST)
N = [0]
def box(name, x0, x1, y0, y1, z0, z1, m, folder, col=True, mat=None):
    a = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector((x0+x1)/2, (y0+y1)/2, (z0+z1)/2))
    c = a.static_mesh_component; c.set_static_mesh(CUBE); c.set_material(0, mat or M[m])
    a.set_actor_scale3d(unreal.Vector((x1-x0)/100.0, (y1-y0)/100.0, (z1-z0)/100.0))
    a.set_actor_label(name); a.set_folder_path(folder); a.tags = [unreal.Name('F2F')]
    if not col: c.set_collision_profile_name('NoCollision')
    N[0] += 1
    return a
def mod(n, x, y, z, yaw=0, sx=1, folder='Modulos', col=True):
    a = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(roll=0, pitch=0, yaw=yaw))
    a.static_mesh_component.set_static_mesh(MODS[n]); a.set_actor_scale3d(unreal.Vector(sx, 1, 1))
    a.set_folder_path(folder); a.set_actor_label(n); a.tags = [unreal.Name('F2F'), unreal.Name('F2Modulo')]
    if not col: a.static_mesh_component.set_collision_profile_name('NoCollision')
    N[0] += 1
    return a
def rail(x0, y0, x1, y1, z):   # guarda-corpo modular (modulo de 200 cm, esticado para fechar o trecho)
    L = math.hypot(x1-x0, y1-y0); k = max(1, round(L / 200.0)); yaw = math.degrees(math.atan2(y1-y0, x1-x0))
    for i in range(k):
        f = (i + 0.5) / k
        mod('F2M_Guarda_200', x0 + (x1-x0)*f, y0 + (y1-y0)*f, z, yaw, (L / k) / 200.0, 'Guardas')
def stairs(name, x0, x1, ys, ydir, z0, folder, n=20, run=50.0, rise=30.0, m='guia'):
    for i in range(n):
        ya, yb = ys + ydir*i*run, ys + ydir*(i+1)*run
        box('%s_degrau%02d' % (name, i), x0, x1, min(ya, yb), max(ya, yb), z0, z0 + (i+1)*rise, m, folder)
def text(label, loc, yaw, size=55, color=(1, 1, 1)):
    t = ell.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(*loc), unreal.Rotator(roll=0, pitch=0, yaw=yaw))
    t.text_render.set_editor_property('text', label); t.text_render.set_editor_property('world_size', size)
    t.text_render.set_editor_property('text_render_color', unreal.Color(int(color[0]*255), int(color[1]*255), int(color[2]*255), 255))
    t.text_render.set_editor_property('horizontal_alignment', unreal.HorizTextAligment.EHTA_CENTER)
    t.set_folder_path('Rotulos'); t.set_actor_label('Rotulo')
def mark(tag, x, y, z, folder='Marcadores'):
    t = ell.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(x, y, z)); t.tags = [unreal.Name(tag)]; t.set_actor_label(tag); t.set_folder_path(folder)
    return t
def light(x, y, z, lm, rgb=(0.75, 0.85, 1.0), r=1800):
    l = ell.spawn_actor_from_class(unreal.PointLight, unreal.Vector(x, y, z)); c = l.point_light_component
    c.set_editor_property('mobility', unreal.ComponentMobility.MOVABLE); c.set_editor_property('intensity_units', unreal.LightUnits.LUMENS)
    c.set_editor_property('intensity', lm); c.set_editor_property('light_color', unreal.Color(int(rgb[0]*255), int(rgb[1]*255), int(rgb[2]*255), 255))
    c.set_editor_property('attenuation_radius', r); c.set_editor_property('cast_shadows', False); l.set_folder_path('Luz')

# ===== ENVOLTORIO: X -600..2800, Y -1250..1250, Z 0..2600 (aberto em cima) =====
box('A_Piso', -600, 2800, -1250, 1250, -40, 0, 'A', 'Envoltorio')
box('Parede_Oeste', -640, -600, -1290, 1290, -40, 2600, 'parede', 'Envoltorio')
box('Parede_Norte', -640, 2840, 1250, 1290, -40, 2600, 'parede', 'Envoltorio')
box('Parede_Sul', -640, 2840, -1290, -1250, -40, 2600, 'parede', 'Envoltorio').tags = [unreal.Name('F2F'), unreal.Name('CorteSul')]
# parede leste com a PORTA DE SAIDA (D): vao Y -800..-400, Z 1800..2250
box('Parede_Leste_S', 2800, 2840, -1290, -800, -40, 2600, 'parede', 'Envoltorio')
box('Parede_Leste_N', 2800, 2840, -400, 1290, -40, 2600, 'parede', 'Envoltorio')
box('Parede_Leste_Baixo', 2800, 2840, -800, -400, -40, 1800, 'parede', 'Envoltorio')
box('Parede_Leste_Topo', 2800, 2840, -800, -400, 2250, 2600, 'parede', 'Envoltorio')

# ===== A (Z=0): saguao de inicio X -600..450 + corredor do poco X 450..1050 (Y -300..300) ate o elevador =====
for s in (1, -1):
    y0, y1 = sorted((s*300, s*1250))
    box('Sala_B_ParedeOeste_' + ('N' if s > 0 else 'S'), 450, 470, y0, y1, 0, 1200, 'parede', 'B_Salas')
    # paredes internas |Y|=300 (limitam o corredor do poco em A e separam as salas de B): vaos do corredor da cabine e janelas do hub
    yi0, yi1 = sorted((s*300, s*320))
    box('ParedeInterna_%s_A' % ('N' if s > 0 else 'S'), 450, 1950, yi0, yi1, 0, 600, 'parede', 'B_Salas')
    box('ParedeInterna_%s_B1' % ('N' if s > 0 else 'S'), 450, 1075, yi0, yi1, 600, 1200, 'parede', 'B_Salas')
    box('ParedeInterna_%s_B2' % ('N' if s > 0 else 'S'), 1325, 1550, yi0, yi1, 600, 1200, 'parede', 'B_Salas')
    box('ParedeInterna_%s_B3' % ('N' if s > 0 else 'S'), 1850, 1950, yi0, yi1, 600, 1200, 'parede', 'B_Salas')
    box('ParedeInterna_%s_VergaCorredor' % ('N' if s > 0 else 'S'), 1075, 1325, yi0, yi1, 900, 1200, 'parede', 'B_Salas')
    box('ParedeInterna_%s_JanelaBaixo' % ('N' if s > 0 else 'S'), 1550, 1850, yi0, yi1, 600, 800, 'parede', 'B_Salas')
    box('ParedeInterna_%s_JanelaTopo' % ('N' if s > 0 else 'S'), 1550, 1850, yi0, yi1, 1100, 1200, 'parede', 'B_Salas')
    box('P2_Janela_Observacao_' + ('N' if s > 0 else 'S'), 1550, 1850, yi0 + 5, yi1 - 5, 800, 1100, 'guia', 'Placeholders_P2', mat=GLASS)
# poco de servico do elevador em A (cabine 250x250 centrada em X 1200, Y 0): paredes + bloco do contrapeso a leste
box('Poco_A_ParedeOeste_N', 1040, 1050, 130, 170, 0, 900, 'parede', 'Elevador')
box('Poco_A_ParedeOeste_S', 1040, 1050, -170, -130, 0, 900, 'parede', 'Elevador')
box('Poco_A_VergaOeste', 1040, 1050, -130, 130, 260, 900, 'parede', 'Elevador')
for s in (1, -1):
    y0, y1 = sorted((s*150, s*170))
    box('Poco_ParedeLado_%s_A' % ('N' if s > 0 else 'S'), 1050, 1450, y0, y1, 0, 600, 'parede', 'Elevador')
    box('Poco_ParedeLado_%s_B1' % ('N' if s > 0 else 'S'), 1050, 1075, y0, y1, 600, 900, 'parede', 'Elevador')
    box('Poco_ParedeLado_%s_B2' % ('N' if s > 0 else 'S'), 1325, 1450, y0, y1, 600, 900, 'parede', 'Elevador')
    # fechamento entre o poco e a parede interna (nao sobra fresta em A)
    yy0, yy1 = sorted((s*170, s*300))
    box('Poco_Fecho_%s' % ('N' if s > 0 else 'S'), 1370, 1450, yy0, yy1, 0, 600, 'parede', 'Elevador')
box('P1_Contrapeso_Elevador', 1360, 1450, -150, 150, 0, 900, 'contra', 'Placeholders_P1')
box('Hub_ParedeOeste', 1450, 1470, -300, 300, 0, 1200, 'parede', 'B_Hub')
# ===== B (Z=600): sala NORTE (Ruina) e sala SUL (Neon) X 450..1950, |Y| 320..1250; corredores da cabine; hub; reencontro =====
for s, nome in ((1, 'Norte_Ruina'), (-1, 'Sul_Neon')):
    y0, y1 = sorted((s*320, s*1250))
    box('B_Piso_' + nome, 470, 1950, y0, y1, 560, 600, 'B', 'B_Salas')
    yc0, yc1 = sorted((s*147, s*300))
    box('B_Corredor_Piso_' + nome, 1075, 1325, yc0, yc1, 560, 600, 'B', 'B_Salas')
    box('B_Corredor_ParedeW_' + nome, 1065, 1075, yc0, yc1, 600, 900, 'parede', 'B_Salas')
    box('B_Corredor_ParedeE_' + nome, 1325, 1335, yc0, yc1, 600, 900, 'parede', 'B_Salas')
    box('B_Corredor_Teto_' + nome, 1065, 1335, yc0, yc1, 900, 920, 'parede', 'B_Salas')
for s in (1, -1):
    yA, yB = (320, 600) if s > 0 else (-600, -320)
    yC, yD = (900, 1250) if s > 0 else (-1250, -900)
    yg0, yg1 = (600, 900) if s > 0 else (-900, -600)
    box('B_ParedeLeste_%s_a' % ('N' if s > 0 else 'S'), 1950, 1970, yA, yB, 0, 1200, 'parede', 'B_Salas')
    box('B_ParedeLeste_%s_b' % ('N' if s > 0 else 'S'), 1950, 1970, yC, yD, 0, 1200, 'parede', 'B_Salas')
    box('B_ParedeLeste_%s_baixo' % ('N' if s > 0 else 'S'), 1950, 1970, yg0, yg1, 0, 600, 'parede', 'B_Salas')
    box('B_ParedeLeste_%s_verga' % ('N' if s > 0 else 'S'), 1950, 1970, yg0, yg1, 1000, 1200, 'parede', 'B_Salas')
# hub de contrapesos (entre as salas, visto pelas janelas): fechado
box('Hub_ParedeLeste', 1930, 1950, -300, 300, 0, 1200, 'parede', 'B_Hub')
box('Hub_Piso', 1470, 1930, -300, 300, 560, 600, 'B', 'B_Hub')
box('P2_Contrapeso_Central', 1620, 1780, -100, 100, 600, 1100, 'contra', 'Placeholders_P2')
# tetos de B = piso de C
box('C_Faixa_Norte', 450, 2800, 250, 1250, 1160, 1200, 'Cc', 'C')
box('C_Cobertura_Sul', 450, 1950, -1250, -300, 1160, 1200, 'Cc', 'C')
box('C_TopoHub', 1450, 1950, -300, 250, 1160, 1200, 'Cc', 'C')
# reencontro (B leste) e escada B->C
box('B_Reencontro_Piso', 1970, 2800, -1250, 1250, 560, 600, 'B', 'B_Reencontro')
stairs('Escada_B_C', 2250, 2500, -900, 1, 600, 'B_Reencontro')
box('Escada_B_C_Patamar', 2250, 2500, 100, 250, 600, 1200, 'guia', 'B_Reencontro')
# ===== D (Z=1800): plataforma de saida X 2200..2800, Y -1250..150; escada C->D com portao P3 =====
box('D_Plataforma', 2200, 2800, -1250, 150, 1760, 1800, 'D', 'D')
stairs('Escada_C_D', 2550, 2800, 1150, -1, 1200, 'D')
box('Escada_C_D_ParedeLateral', 2530, 2550, 150, 1170, 1200, 2200, 'parede', 'D')
# porta de saida (placeholder, sem logica): moldura, folha e letreiro
box('D_Saida_Moldura_S', 2780, 2800, -850, -800, 1800, 2300, 'saida', 'D')
box('D_Saida_Moldura_N', 2780, 2800, -400, -350, 1800, 2300, 'saida', 'D')
box('D_Saida_Moldura_Topo', 2780, 2800, -850, -350, 2250, 2300, 'saida', 'D')
box('D_Saida_Folha', 2800, 2810, -800, -400, 1800, 2250, 'D', 'D')
box('D_Saida_Letreiro', 2775, 2785, -820, -380, 2320, 2450, 'saida', 'D', col=False)
# ===== GUARDA-CORPOS (modulos) =====
rail(450, 250, 1450, 250, 1200); rail(450, -300, 1450, -300, 1200)          # bordas do poco em C
rail(450, 250, 450, 1250, 1200); rail(450, -300, 450, -1250, 1200)          # bordas de C sobre o saguao
rail(1450, -300, 1450, 250, 1200)                                            # topo do hub sobre o poco
rail(1950, 250, 2250, 250, 1200); rail(2500, 250, 2530, 250, 1200)           # C sobre o reencontro (vao da escada B->C)
rail(1950, -300, 1950, 250, 1200); rail(1950, -1250, 1950, -300, 1200)
rail(2200, -1250, 2200, 150, 1800); rail(2200, 150, 2530, 150, 1800)         # D
rail(2250, -900, 2250, 100, 600); rail(2500, -900, 2500, 100, 600)           # laterais da escada B->C (lado do reencontro)
# ===== ELEVADOR (A<->B), consoles, barreiras, portoes =====
E = ell.spawn_actor_from_class(unreal.load_class(None, '/Script/JogoCoop.CoopElevator'), unreal.Vector(1200, 0, 0))
E.set_actor_label('Elevador_A_B'); E.set_folder_path('Elevador')
for k, v in (('rise', 600.0), ('explicit_control', True), ('landing_doors', True), ('locked_p1', True)): E.set_editor_property(k, v)
CC = unreal.load_class(None, '/Script/JogoCoop.CoopConsole')
def console(label, x, y, z, yaw, action, attach=False):
    c = ell.spawn_actor_from_class(CC, unreal.Vector(x, y, z), unreal.Rotator(roll=0, pitch=0, yaw=yaw))
    c.set_editor_property('action', action); c.set_editor_property('elevator', E); c.set_editor_property('attach_to_elevator', attach)
    c.set_actor_label(label); c.set_folder_path('Elevador')
console('Console_Cabine', 1300, 85, 60, 0, unreal.CoopConsoleAction.ELEVATOR_TOGGLE, True)
console('Console_Chamada_A', 985, -230, 60, 0, unreal.CoopConsoleAction.ELEVATOR_CALL_BOTTOM)
console('Console_Chamada_B_Norte', 1400, 380, 660, 0, unreal.CoopConsoleAction.ELEVATOR_CALL_TOP)
console('Console_Chamada_B_Sul', 1400, -380, 660, 0, unreal.CoopConsoleAction.ELEVATOR_CALL_TOP)
BC = unreal.load_class(None, '/Script/JogoCoop.CoopRoleBarrier')
for s, role, nome in ((1, unreal.CoopBarrierRole.RUINA, 'Barreira_Roxa_Ruina'), (-1, unreal.CoopBarrierRole.NEON, 'Barreira_Ciano_Neon')):
    b = ell.spawn_actor_from_class(BC, unreal.Vector(1200, s*230, 750)); b.set_editor_property('allowed_role', role)
    b.set_editor_property('size', unreal.Vector(250, 20, 300)); b.set_actor_label(nome); b.set_folder_path('B_Salas')
GC = unreal.load_class(None, '/Script/JogoCoop.CoopGate')
def gate(gid, x, y, z, size):
    g = ell.spawn_actor_from_class(GC, unreal.Vector(x, y, z)); g.set_editor_property('gate_id', gid); g.set_editor_property('size', unreal.Vector(*size))
    g.set_actor_label('Portao_' + gid); g.set_folder_path('Portoes')
gate('P2_Norte', 1960, 750, 600, (20, 300, 400)); gate('P2_Sul', 1960, -750, 600, (20, 300, 400))
gate('P3_Saida', 2675, 1185, 1200, (250, 20, 400))
# ===== PLACEHOLDERS dos puzzles (sem logica) =====
box('P1_Trava_Destrutivel', 1100, 1300, 170, 230, 80, 260, 'trava', 'Placeholders_P1')
box('P1_Suporte_Reconstruivel', 1100, 1300, -230, -170, 0, 200, 'suporte', 'Placeholders_P1', col=False)
box('P1_Mecanismo_Tambor', 1000, 1040, -120, 120, 300, 480, 'mec', 'Placeholders_P1')
box('P1_Area_Aproximacao', 470, 1040, -290, 290, 0, 4, 'saida', 'Placeholders_P1', col=False)
for s, nome, cor in ((1, 'Norte_Ruina', 'trava'), (-1, 'Sul_Neon', 'suporte')):
    y = s*750
    box('P2_PlataformaSuspensa_' + nome, 1250, 1650, y-200, y+200, 640, 680, 'contra', 'Placeholders_P2')
    for dx, dy in ((-190, -190), (190, -190), (-190, 190), (190, 190)):
        box('P2_Cabo_' + nome, 1450+dx-3, 1450+dx+3, y+dy-3, y+dy+3, 680, 1160, 'trilho', 'Placeholders_P2', col=False)
    box('P2_Carga_' + nome, 1400, 1500, y-50, y+50, 680, 800, 'mec', 'Placeholders_P2')
if True:
    box('P2_Freio_1_Ruina', 700, 820, 900, 1020, 600, 760, 'trava', 'Placeholders_P2')
    box('P2_Freio_2_Ruina', 700, 820, 500, 620, 600, 760, 'trava', 'Placeholders_P2')
    box('P2_TravaLiberacao_Ruina', 1800, 1900, 1100, 1200, 600, 900, 'trava', 'Placeholders_P2')
    box('P2_Guia_Reconstruivel_Neon', 700, 760, -1100, -500, 600, 1160, 'suporte', 'Placeholders_P2', col=False)
    box('P2_Suporte_Transferencia_Neon', 1800, 1900, -1200, -1100, 600, 900, 'suporte', 'Placeholders_P2', col=False)
    box('P2_Rota_Reencontro', 1970, 2800, -100, 100, 600, 604, 'saida', 'Placeholders_P2', col=False)
# P3: trajetoria da esfera na cobertura sul de C (X 460..1950, Y ~-800), 3 trechos + lancador + sensor + coletor.
# (a faixa norte fica livre para circulacao e para o portao P3 da escada C->D)
box('P3_Lancador', 1800, 1940, -920, -680, 1200, 1450, 'mec', 'Placeholders_P3')
for i, (xa, xb, z) in enumerate(((1350, 1800, 1380), (900, 1350, 1350), (550, 900, 1320))):
    box('P3_Trecho%d_Construivel' % (i+1), xa, xb, -850, -750, z, z+20, ('suporte', 'guia', 'trava')[i], 'Placeholders_P3')
    for x in (xa + 50, xb - 50): box('P3_Apoio', x-10, x+10, -810, -790, 1200, z, 'trilho', 'Placeholders_P3')
s = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(1870, -800, 1510)); s.static_mesh_component.set_static_mesh(SPH)
s.static_mesh_component.set_material(0, M['console']); s.set_actor_scale3d(unreal.Vector(1.2, 1.2, 1.2)); s.set_actor_label('P3_Esfera'); s.set_folder_path('Placeholders_P3')
box('P3_Sensor_Final', 460, 550, -860, -740, 1200, 1340, 'console', 'Placeholders_P3')
box('P3_Coletor_Seguranca', 550, 1800, -1080, -980, 1200, 1215, 'saida', 'Placeholders_P3', col=False)
# ===== MODULOS VISUAIS (Blender): paineis no saguao, trelicas sobre o poco, pilares, grades, tubos =====
for i in range(12):
    y = -1100 + i * 200
    for z in (0, 300): mod('F2M_Painel_200x300', -596, y, z, 90, folder='Modulos_Parede', col=False)
for x in (550, 950, 1450): mod('F2M_Trelica_400', x, 0, 2450, 90, 1.5, 'Modulos_Trelica', col=False)
for x in (-200, 200, 600, 1000, 1400, 1800, 2200, 2600): mod('F2M_Trelica_400', x, 0, 2540, 0, 1.0, 'Modulos_Trelica', col=False)
for (x, y) in ((450, 300), (450, -300), (1450, 300), (1450, -300), (1950, 300), (1950, -300)): mod('F2M_Pilar_600', x, y, 1200, 0, folder='Modulos_Pilar', col=False)
mod('F2M_Grade_200', 2375, 175, 1200.5, 0, folder='Modulos_Grade', col=False)
for y in (-500, -1000): mod('F2M_Grade_200', 2500, y, 1800.5, 0, folder='Modulos_Grade', col=False)
for (x, y, z) in ((700, 290, 420), (700, -290, 420), (2400, 1235, 900), (2400, -1235, 900)): mod('F2M_Tubo_400', x, y, z, 0, 1.0, 'Modulos_Tubo', col=False)
# ===== ROTULOS =====
text('A | INICIO - PUZZLE 1: ELEVADOR DE EMERGENCIA', (-590, 0, 420), 0, 60, (.6, 1, .6))
text('TRAVA (Ruina)', (1200, 240, 300), 270, 35, (1, .4, .4)); text('SUPORTE (Neon)', (1200, -240, 230), 90, 35, (.4, .6, 1))
text('B NORTE - RUINA (freios/travas)', (1300, 1240, 1050), 270, 45, (1, .85, .4)); text('B SUL - NEON (guias/suportes)', (1300, -1240, 1050), 90, 45, (.4, 1, 1))
text('REENCONTRO (apos P2)', (2790, 0, 900), 180, 50, (1, 1, 1))
text('C - PUZZLE 3: NUCLEO EM MOVIMENTO', (1200, -1240, 1550), 90, 50, (.5, .7, 1))
text('SAIDA', (2770, -600, 2350), 180, 90, (1, .8, .2))
text('POCO CENTRAL 500x500', (1200, 245, 1700), 270, 50)
# ===== marcadores de layout, retorno de queda, inicio, apresentacao, luzes =====
mark('F2X_Layout', 0, 0, 50)
for tag, x, y, z in (('F2X_Check_A', -250, 0, 120), ('F2X_Check_B_Ruina', 1200, 520, 720), ('F2X_Check_B_Neon', 1200, -520, 720),
                     ('F2X_Check_C', 2375, 380, 1320), ('F2X_Check_D', 2500, -600, 1920)): mark(tag, x, y, z)
for i, y in enumerate((-150, 150)):
    p = ell.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-300, y, 100)); p.set_actor_label('PlayerStart_%d' % (i+1)); p.set_folder_path('Inicio')
ic = ell.spawn_actor_from_class(unreal.load_class(None, '/Script/JogoCoop.CoopIntroCam'), unreal.Vector(0, 0, 300)); ic.set_actor_label('Apresentacao_Camera'); ic.set_folder_path('Inicio')
ic.set_editor_property('pos', [unreal.Vector(-520, -60, 260), unreal.Vector(250, 0, 600), unreal.Vector(1000, 0, 1500), unreal.Vector(1700, -300, 2150)])
ic.set_editor_property('look', [unreal.Vector(-300, 0, 150), unreal.Vector(1200, 0, 900), unreal.Vector(2200, -400, 1900), unreal.Vector(2800, -600, 2050)])
for pitch, yaw, lux, sh in ((-62, 28, 2.2, True), (-40, 208, 1.0, False), (-30, 100, 0.6, False)):
    d = ell.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 3000), unreal.Rotator(roll=0, pitch=pitch, yaw=yaw))
    d.light_component.set_editor_property('mobility', unreal.ComponentMobility.MOVABLE); d.light_component.set_editor_property('intensity', lux)
    d.light_component.set_editor_property('cast_shadows', sh); d.set_folder_path('Luz')
for (x, y, z, lm) in ((900, 800, 1050, 2200), (1600, 800, 1050, 2200), (900, -800, 1050, 2200), (1600, -800, 1050, 2200), (1700, 0, 1050, 1500),
                      (2400, 700, 1050, 2500), (2400, -700, 1100, 2500), (800, 0, 400, 2000), (2500, -600, 2200, 2000), (-200, 0, 600, 2500)): light(x, y, z, lm)
pp = ell.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(1200, 0, 1100)); pp.set_editor_property('unbound', True)
st = pp.get_editor_property('settings')
st.set_editor_property('override_auto_exposure_min_brightness', True); st.set_editor_property('auto_exposure_min_brightness', 1.0)
st.set_editor_property('override_auto_exposure_max_brightness', True); st.set_editor_property('auto_exposure_max_brightness', 1.0)
st.set_editor_property('override_motion_blur_amount', True); st.set_editor_property('motion_blur_amount', 0.0)
st.set_editor_property('override_auto_exposure_bias', True); st.set_editor_property('auto_exposure_bias', -1.0)
pp.set_editor_property('settings', st); pp.set_folder_path('Luz')
ell.get_editor_world().get_world_settings().set_editor_property('default_game_mode', unreal.load_class(None, '/Script/JogoCoop.CoopFase2GameMode'))
ell.save_current_level()
print('[F2F] CRIADO', DST, 'atores', len(ell.get_all_level_actors()), 'caixas/modulos', N[0])
