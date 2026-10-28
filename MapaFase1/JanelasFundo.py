# Fundo luminoso atras dos vidros das janelas (Painel_Parede) do mapa Fase1_Editavel.
# Lado inicial: brilho de fogo; lado da saida: luz fria. O fundo fica "preso" a janela (move junto no Editor).
import unreal
mel = unreal.MaterialEditingLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary
D = '/Game/Fase1/Janela'
M = D + '/M_JanelaFundo'
if not eal.does_asset_exist(M):
    mat = at.create_asset('M_JanelaFundo', D, unreal.Material, unreal.MaterialFactoryNew())
    cor = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -700, 0)
    cor.set_editor_property('parameter_name', 'Cor')
    cor.set_editor_property('default_value', unreal.LinearColor(1, 0.35, 0.06, 1))
    tc = mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -900, 250)
    mk = mel.create_material_expression(mat, unreal.MaterialExpressionComponentMask, -750, 250)
    mk.set_editor_property('r', False); mk.set_editor_property('g', True)
    ca = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -750, 350); ca.set_editor_property('r', 0.5)
    cb = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -750, 430); cb.set_editor_property('r', 2.2)
    lp = mel.create_material_expression(mat, unreal.MaterialExpressionLinearInterpolate, -550, 300)
    mel.connect_material_expressions(tc, '', mk, '')
    mel.connect_material_expressions(ca, '', lp, 'A'); mel.connect_material_expressions(cb, '', lp, 'B'); mel.connect_material_expressions(mk, '', lp, 'Alpha')
    mu = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 100)
    mel.connect_material_expressions(cor, '', mu, 'A'); mel.connect_material_expressions(lp, '', mu, 'B')
    mel.connect_material_property(mu, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    bk = mel.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -300, -120)
    bk.set_editor_property('constant', unreal.LinearColor(0.02, 0.02, 0.02, 1))
    mel.connect_material_property(bk, '', unreal.MaterialProperty.MP_BASE_COLOR)
    mel.recompile_material(mat); eal.save_asset(M)
base = unreal.load_asset(M)
def mi(name, col):
    p = D + '/' + name
    if eal.does_asset_exist(p): return unreal.load_asset(p)
    m = at.create_asset(name, D, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    mel.set_material_instance_parent(m, base)
    mel.set_material_instance_vector_parameter_value(m, 'Cor', col)
    eal.save_asset(p); return m
fogo = mi('MI_JanelaFogo', unreal.LinearColor(3.0, 1.0, 0.15, 1))
fria = mi('MI_JanelaFria', unreal.LinearColor(0.5, 0.9, 1.6, 1))
ell = unreal.EditorLevelLibrary
ell.load_level('/Game/Maps/Fase1_Editavel')
for a in ell.get_all_level_actors():
    if a.get_actor_label().startswith('JanelaFundo'): ell.destroy_actor(a)
cube = unreal.load_asset('/Engine/BasicShapes/Cube')
n = 0
for w in ell.get_all_level_actors():
    if not isinstance(w, unreal.StaticMeshActor): continue
    sm = w.static_mesh_component.static_mesh
    if not sm or sm.get_name() != 'SM_Painel_Parede' or 'F1Ref' in [str(t) for t in w.tags]: continue
    b = ell.spawn_actor_from_class(unreal.StaticMeshActor, w.get_actor_location(), w.get_actor_rotation())
    c = b.static_mesh_component
    c.set_mobility(unreal.ComponentMobility.MOVABLE)
    c.set_static_mesh(cube)
    c.set_material(0, fogo if w.get_actor_location().x < 1500 else fria)
    c.set_collision_profile_name('NoCollision')
    c.set_cast_shadow(False)
    b.attach_to_actor(w, unreal.Name(''), unreal.AttachmentRule.SNAP_TO_TARGET, unreal.AttachmentRule.SNAP_TO_TARGET, unreal.AttachmentRule.SNAP_TO_TARGET, False)
    b.root_component.set_relative_location(unreal.Vector(0, -3, 120), False, False)
    b.root_component.set_relative_scale3d(unreal.Vector(5.2, 0.02, 2.0))
    b.tags = [unreal.Name('F1Decor')]
    b.set_actor_label('JanelaFundo')
    b.set_folder_path('Decor/JanelaFundo')
    n += 1
ell.save_current_level()
print('[FASE1ED] JANELAS_OK %d' % n)
