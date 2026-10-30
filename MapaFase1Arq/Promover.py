# Promove a nova arquitetura a Fase 1 oficial (renomeacao pelo editor, que atualiza o registro de assets):
#   /Game/Maps/Fase1_Editavel   -> /Game/Maps/Backup/Fase1_Original_Editavel  (backup do mapa antigo, continua jogavel)
#   /Game/Maps/Fase1_Arquitetura -> /Game/Maps/Fase1                           (Fase 1 oficial)
# Reverter: MapaFase1Arq/ReverterPromocao.py
import unreal
eal = unreal.EditorAssetLibrary; ell = unreal.EditorLevelLibrary
src_old, dst_old = '/Game/Maps/Fase1_Editavel', '/Game/Maps/Backup/Fase1_Original_Editavel'
src_new, dst_new = '/Game/Maps/Fase1_Arquitetura', '/Game/Maps/Fase1'
ell.load_level('/Game/Maps/MainMenu')
# rename_asset abre um dialogo (cancelado no modo automatico) por haver nomes em textos/scripts; entao: duplica e, se a copia
# existir e carregar, remove o nome antigo (equivale a renomear; copia completa tambem em Backups/ e na tag git fase1-original-backup)
for s, d in ((src_old, dst_old), (src_new, dst_new)):
    if not eal.does_asset_exist(d):
        print('[PROMO] duplicar', s, '->', d, eal.duplicate_asset(s, d) is not None)
        eal.save_asset(d, only_if_is_dirty=False)
    if eal.does_asset_exist(d) and eal.does_asset_exist(s):
        print('[PROMO] remover nome antigo', s, eal.delete_asset(s))
for p in (dst_new, dst_old):
    ell.load_level(p)
    acts = ell.get_all_level_actors()
    cls = {}
    for a in acts: cls[a.get_class().get_name()] = cls.get(a.get_class().get_name(), 0) + 1
    gm = ell.get_editor_world().get_world_settings().get_editor_property('default_game_mode')
    print('[PROMO] mapa', p, 'atores', len(acts), 'GameMode', gm.get_name() if gm else None,
          'rig', cls.get('CoopCollapseRig', 0), 'tunel', cls.get('CoopTunnelDressing', 0), 'marcadores', sum(1 for a in acts if any(str(t).startswith('F1L_') for t in a.tags)))
print('[PROMO] antigo existe?', eal.does_asset_exist(src_old), '| arquitetura existe?', eal.does_asset_exist(src_new))
print('[PROMO] FIM')
