# Desfaz a promocao: recria os nomes originais a partir das copias e remove /Game/Maps/Fase1 e a pasta Backup.
import unreal
eal = unreal.EditorAssetLibrary; unreal.EditorLevelLibrary.load_level('/Game/Maps/MainMenu')
for s, d in (('/Game/Maps/Fase1', '/Game/Maps/Fase1_Arquitetura'), ('/Game/Maps/Backup/Fase1_Original_Editavel', '/Game/Maps/Fase1_Editavel')):
    if not eal.does_asset_exist(d): print('[REVERTE] duplicar', s, d, eal.duplicate_asset(s, d) is not None); eal.save_asset(d, only_if_is_dirty=False)
    if eal.does_asset_exist(d) and eal.does_asset_exist(s): print('[REVERTE] remover', s, eal.delete_asset(s))
# alternativa sem editor: git checkout fase1-original-backup -- Content/Maps/Fase1_Editavel.umap Content/Maps/Fase1_Arquitetura.umap
