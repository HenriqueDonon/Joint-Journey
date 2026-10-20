# Refaz so a luz do Editor no mapa Fase1_Editavel (nao mexe nos modelos)
import unreal, importlib, sys, os
sys.path.append(os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), 'MapaFase1'))
ell = unreal.EditorLevelLibrary
ell.load_level('/Game/Maps/Fase1_Editavel')
src = open(os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), 'MapaFase1', 'CriarFase1Editavel.py'), encoding='utf-8').read()
ns = {}
exec(src.split('def run():')[0], ns)
ns['luz_editor'](ell)
ell.save_current_level()
print('[FASE1ED] LUZ_OK')
