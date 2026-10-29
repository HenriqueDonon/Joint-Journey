# Fase 1 EXPERIMENTAL (copia segura): duplica /Game/Maps/Fase1_Editavel -> /Game/Maps/Fase1_Experimental e acrescenta
# melhorias visuais. O mapa oficial nao e alterado. A jogabilidade (coluna, ponte, plataformas, porta, colapso) continua
# criada pelo codigo (CoopFase1GameMode) igual ao oficial. Atores novos: tags F1Decor + F1Exp, pasta "Experimental".
import unreal, os
proj = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
SRC, DST = '/Game/Maps/Fase1_Editavel', '/Game/Maps/Fase1_Experimental'
ell = unreal.EditorLevelLibrary; eal = unreal.EditorAssetLibrary; mel = unreal.MaterialEditingLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
if eal.does_asset_exist(DST) and not os.path.exists(os.path.join(proj, 'MapaFase1X', 'RECRIAR.txt')):
    print('[F1X] JA_EXISTE (nada feito)'); raise SystemExit
# ---- 1. material de cor por vertice (props do catalogo Higgsfield nao tem UV/textura) ----
MP = '/Game/Fase1X/M_F1X_CorVertice'
if not eal.does_asset_exist(MP):
    m = at.create_asset('M_F1X_CorVertice', '/Game/Fase1X', unreal.Material, unreal.MaterialFactoryNew())
    vc = mel.create_material_expression(m, unreal.MaterialExpressionVertexColor, -500, 0)
    mul = mel.create_material_expression(m, unreal.MaterialExpressionMultiply, -250, 0)
    k = mel.create_material_expression(m, unreal.MaterialExpressionConstant, -500, 150); k.set_editor_property('r', 0.75)
    mel.connect_material_expressions(vc, '', mul, 'A'); mel.connect_material_expressions(k, '', mul, 'B')
    mel.connect_material_property(mul, '', unreal.MaterialProperty.MP_BASE_COLOR)
    r = mel.create_material_expression(m, unreal.MaterialExpressionConstant, -500, 300); r.set_editor_property('r', 0.62)
    mt = mel.create_material_expression(m, unreal.MaterialExpressionConstant, -500, 400); mt.set_editor_property('r', 0.35)
    mel.connect_material_property(r, '', unreal.MaterialProperty.MP_ROUGHNESS)
    mel.connect_material_property(mt, '', unreal.MaterialProperty.MP_METALLIC)
    mel.recompile_material(m); eal.save_loaded_asset(m)
MAT = unreal.load_asset(MP)
# ---- 2. importa os props do Higgsfield (FBX exportados do .blend rev.10) ----
props = {}
for nm in ('HF_Console', 'HF_Gerador', 'HF_Transformador', 'HF_Barris', 'HF_Sinalizador'):
    path = '/Game/Fase1X/HF/' + nm + '/SM_' + nm
    if not eal.does_asset_exist(path):
        t = unreal.AssetImportTask()
        t.filename = os.path.join(proj, 'ImportHiggsfield', nm + '.fbx'); t.destination_path = '/Game/Fase1X/HF/' + nm
        t.destination_name = 'SM_' + nm; t.automated = True; t.save = True; t.replace_existing = True
        ui = unreal.FbxImportUI(); ui.import_materials = False; ui.import_textures = False; ui.import_as_skeletal = False
        ui.static_mesh_import_data.combine_meshes = True; ui.static_mesh_import_data.auto_generate_collision = True
        ui.static_mesh_import_data.vertex_color_import_option = unreal.VertexColorImportOption.REPLACE
        t.options = ui
        at.import_asset_tasks([t])
    sm = unreal.load_asset(path)
    if sm:
        sm.set_material(0, MAT); eal.save_loaded_asset(sm)
        props[nm] = sm
    print('[F1X] prop', nm, bool(sm))
# ---- 3. copia o mapa e carrega a copia ----
if not eal.does_asset_exist(DST):
    ell.load_level(SRC)                                    # abre o oficial so para "Salvar como" a copia (o oficial nao e salvo)
    unreal.EditorLoadingAndSavingUtils.save_map(ell.get_editor_world(), DST)
    print('[F1X] copia salva', eal.does_asset_exist(DST))
ell.load_level(DST)
for a in ell.get_all_level_actors():
    if any(str(t) == 'F1Exp' for t in a.tags): ell.destroy_actor(a)
def tag(a, folder, label):
    a.tags = list(a.tags) + [unreal.Name('F1Decor'), unreal.Name('F1Exp')]
    a.set_folder_path('Experimental/' + folder); a.set_actor_label(label)
    return a
def prop(nm, x, y, z, yaw, col=True):
    a = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(roll=0, pitch=0, yaw=yaw))
    c = a.static_mesh_component; c.set_mobility(unreal.ComponentMobility.MOVABLE); c.set_static_mesh(props[nm])
    c.set_collision_profile_name('BlockAll' if col else 'NoCollision')
    return tag(a, 'Higgsfield', nm)
# 3a. props do catalogo Higgsfield: so junto as paredes, fora do percurso (centro Y +-650; pecas ate 1,12 m de fundo)
prop('HF_Gerador', -750, 655, 0, 180)
prop('HF_Console', -700, -655, 0, 90)
prop('HF_Transformador', -1800, -670, 0, 0); prop('HF_Transformador', -1680, -670, 0, 0)
prop('HF_Barris', -1560, -640, 0, 0)
prop('HF_Gerador', 2800, 655, 44, 180)
prop('HF_Console', 2800, -655, 44, 90)
prop('HF_Transformador', 1660, 670, 44, 180)
for y in (-300, 300): prop('HF_Sinalizador', 3585, y, 400, 0, col=False)
# 3b. faces do vao de lava (antes lisas/claras): placas industriais verticais, mesma orientacao das placas da parede de fundo
ref = None
for a in ell.get_all_level_actors():
    if a.get_actor_label().startswith('Placa_Industrial') and a.get_actor_location().x < -2050 and abs(a.get_actor_location().y) < 400:
        ref = a; break
n_t = 0
if ref:
    up = ref.get_actor_up_vector()               # normal da placa (lado visivel) na parede de fundo, que olha para +X
    sg = 1.0 if up.x > 0 else -1.0
    print('[F1X] normal da placa de referencia:', up)
    for face_x, top, nx in ((3.0, 0.0, 1.0), (1497.0, 44.0, -1.0)):
        bot = -395.0; h = top - bot
        rot = unreal.MathLibrary.make_rot_from_xz(unreal.Vector(0, 0, 1), unreal.Vector(nx * sg, 0, 0))   # comprimento na vertical, face para o vao
        for i in range(11):
            y = -700 + i * 140
            a = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(face_x, y, (top + bot) / 2), rot)
            c = a.static_mesh_component; c.set_mobility(unreal.ComponentMobility.MOVABLE); c.set_static_mesh(ref.static_mesh_component.static_mesh)
            a.set_actor_scale3d(unreal.Vector(h / 300.0, 1.0, 1.0))
            c.set_collision_profile_name('NoCollision')
            tag(a, 'FaceVao', 'Placa_FaceVao'); n_t += 1
    # paredes laterais do vao (Y=+-750, X 0..1500, abaixo do piso): eram a faixa clara/lisa vista sobre a lava
    for wy, ny in ((746.0, -1.0), (-746.0, 1.0)):
        bot, top = -395.0, 5.0; h = top - bot
        rot = unreal.MathLibrary.make_rot_from_xz(unreal.Vector(0, 0, 1), unreal.Vector(0, ny * sg, 0))
        for i in range(11):
            x = 70 + i * 140
            a = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, wy, (top + bot) / 2), rot)
            c = a.static_mesh_component; c.set_mobility(unreal.ComponentMobility.MOVABLE); c.set_static_mesh(ref.static_mesh_component.static_mesh)
            a.set_actor_scale3d(unreal.Vector(h / 300.0, 1.0, 1.0))
            c.set_collision_profile_name('NoCollision')
            tag(a, 'ParedeVao', 'Placa_ParedeVao'); n_t += 1
print('[F1X] placas no vao:', n_t, 'ref:', ref.get_actor_label() if ref else None, ref.get_actor_rotation() if ref else '', ref.get_actor_scale3d() if ref else '')
# 3c. iluminacao: brilho quente da lava nas faces do vao, acento ciano na porta de saida, sinalizadores vermelhos
def lamp(x, y, z, col, lm, rad, label):
    l = ell.spawn_actor_from_class(unreal.PointLight, unreal.Vector(x, y, z))
    lc = l.point_light_component; lc.set_editor_property('mobility', unreal.ComponentMobility.MOVABLE)
    lc.set_editor_property('intensity_units', unreal.LightUnits.LUMENS); lc.set_editor_property('intensity', lm)
    lc.set_editor_property('light_color', unreal.Color(int(col[2]*255), int(col[1]*255), int(col[0]*255), 255))
    lc.set_editor_property('attenuation_radius', rad); lc.set_editor_property('cast_shadows', False)
    return tag(l, 'Luz', label)
for y in (-450, 0, 450):
    lamp(120, y, -230, (1.0, 0.38, 0.1), 2500, 650, 'Luz_Lava_FaceInicio')
    lamp(1380, y, -200, (1.0, 0.38, 0.1), 2500, 650, 'Luz_Lava_FaceSaida')
lamp(3470, 0, 250, (0.35, 0.85, 1.0), 2200, 800, 'Luz_Porta_Ciano')
for y in (-300, 300): lamp(3540, y, 420, (1.0, 0.08, 0.05), 900, 450, 'Luz_Sinalizador')
# 3d. pos-processamento: exposicao controlada (cena estava estourada), vinheta, contraste leve, AO
pp = ell.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(750, 0, 200))
pp.set_editor_property('unbound', True); pp.set_editor_property('priority', 5.0)
st = pp.get_editor_property('settings')
for k, v in (('auto_exposure_bias', -1.0),
             ('vignette_intensity', 0.45), ('bloom_intensity', 0.45), ('ambient_occlusion_intensity', 0.7),
             ('color_contrast', unreal.Vector4(1.08, 1.08, 1.08, 1.0)), ('color_saturation', unreal.Vector4(1.06, 1.04, 1.0, 1.0))):
    st.set_editor_property('override_' + k, True); st.set_editor_property(k, v)
pp.set_editor_property('settings', st)
tag(pp, 'Luz', 'PosProcesso_Experimental')
ell.save_current_level()
print('[F1X] CRIADO ' + DST)
