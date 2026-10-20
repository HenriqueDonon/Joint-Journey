# Cria /Game/Maps/Fase1_Editavel a partir de MapaFase1/layout.json (gerado pelo jogo com -DumpLayout).
# - Decoracao com modelo (pasta "Decor" no Outliner): atores normais, pode mover/girar/duplicar/apagar.
# - Referencia (pasta "Referencia"): plataformas, paredes, teto, porta, coluna, ponte... travados, so aparecem no Editor.
#   Essas pecas continuam sendo criadas pelo codigo (tem logica de jogo).
# Se o mapa ja existir, NAO sobrescreve (para nao perder edicoes), a menos que exista MapaFase1/RECRIAR.txt.
import unreal, json, os
proj = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
LAYOUT = os.path.join(proj, 'MapaFase1', 'layout.json')
DST = '/Game/Maps/Fase1_Editavel'
def luz_editor(ell):
    # luz so para o Editor (no jogo a luz vem do codigo): 2 luzes direcionais sem sombra, enxergam por baixo do teto
    for a in ell.get_all_level_actors():
        if a.get_folder_path() and str(a.get_folder_path()).startswith('Referencia/Luz'): ell.destroy_actor(a)
    for pitch, yaw, lux in ((-50, 35, 4.0), (-35, 215, 2.5)):
        l = ell.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(750, 0, 2000), unreal.Rotator(roll=0, pitch=pitch, yaw=yaw))
        lc = l.light_component
        lc.set_editor_property('cast_shadows', False)
        lc.set_editor_property('intensity', lux)
        l.tags = [unreal.Name('F1Ref')]
        l.set_folder_path('Referencia/Luz_so_Editor')
        l.set_editor_property('is_editor_only_actor', True)
        l.set_actor_hidden_in_game(True)

def run():
    force = os.path.exists(os.path.join(proj, 'MapaFase1', 'RECRIAR.txt'))
    ell = unreal.EditorLevelLibrary
    data = json.load(open(LAYOUT, encoding='utf-8'))
    if unreal.EditorAssetLibrary.does_asset_exist(DST):
        ell.load_level(DST)
        mine = [a for a in ell.get_all_level_actors() if any(str(t) in ('F1Decor', 'F1Ref') for t in a.tags)]
        if mine and not force:
            print('[FASE1ED] JA_EXISTE (nada feito; crie MapaFase1/RECRIAR.txt para refazer)'); return
        for a in mine: ell.destroy_actor(a)
    else:
        ell.new_level(DST)
    ws = ell.get_editor_world().get_world_settings()
    ws.set_editor_property('default_game_mode', unreal.load_class(None, '/Script/JogoCoop.CoopFase1GameMode'))
    cache = {}
    nd = nr = 0
    for e in data:
        p = e['m'].split('.')[0]
        if p not in cache: cache[p] = unreal.load_asset(p)
        m = cache[p]
        if not m: continue
        rot = unreal.Rotator(roll=e['r'][2], pitch=e['r'][0], yaw=e['r'][1])
        a = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*e['l']), rot)
        c = a.static_mesh_component
        c.set_mobility(unreal.ComponentMobility.MOVABLE)
        c.set_static_mesh(m)
        a.set_actor_scale3d(unreal.Vector(*e['s']))
        a.set_actor_label(e['n'])
        a.set_folder_path(e['f'])
        if e['d']:
            a.tags = [unreal.Name('F1Decor')]
            c.set_collision_profile_name('BlockAll' if e['c'] else 'NoCollision')
            nd += 1
        else:
            a.tags = [unreal.Name('F1Ref')]
            c.set_collision_profile_name('NoCollision')
            a.set_actor_hidden_in_game(True)
            a.set_editor_property('is_editor_only_actor', True)
            nr += 1
    luz_editor(ell)
    ell.save_current_level()
    print('[FASE1ED] OK decor=%d ref=%d' % (nd, nr))
run()
