# Importa os fragmentos da coluna (ImportFragmentos/SM_ColunaFrag.fbx, 10 pecas cortadas no Blender a partir do SM_Coluna_Inteira)
# para /Game/Fase1/ColunaFrag/SM_ColFrag_01..10. Pivo = origem da coluna (centro da base). Material aplicado no jogo (o mesmo da coluna).
import unreal, os, re
DIR = os.path.dirname(os.path.abspath(__file__))
AT = unreal.AssetToolsHelpers.get_asset_tools(); EAL = unreal.EditorAssetLibrary
DST = '/Game/Fase1/ColunaFrag'
ui = unreal.FbxImportUI()
ui.import_mesh = True; ui.import_as_skeletal = False; ui.import_materials = False; ui.import_textures = False
ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
sd = ui.static_mesh_import_data
sd.set_editor_property('auto_generate_collision', True); sd.set_editor_property('combine_meshes', False)
t = unreal.AssetImportTask(); t.filename = os.path.join(DIR, 'SM_ColunaFrag.fbx'); t.destination_path = DST
t.automated = True; t.replace_existing = True; t.save = True; t.options = ui
AT.import_asset_tasks([t])
n = 0
for p in EAL.list_assets(DST, recursive=False):
    a = unreal.load_asset(p)
    if not isinstance(a, unreal.StaticMesh): continue
    m = re.search(r'ColFrag_(\d+)', a.get_name())
    if not m: continue
    want = 'SM_ColFrag_%02d' % int(m.group(1))
    if a.get_name() != want:
        EAL.rename_asset(p.split('.')[0], DST + '/' + want)
    n += 1
SML = unreal.EditorStaticMeshLibrary
for p in EAL.list_assets(DST, recursive=False):
    a = unreal.load_asset(p)
    if isinstance(a, unreal.StaticMesh) and SML.get_simple_collision_count(a) == 0:
        SML.add_simple_collisions(a, unreal.ScriptingCollisionShapeType.NDOP26)
        EAL.save_loaded_asset(a)
for p in EAL.list_assets(DST, recursive=False):
    a = unreal.load_asset(p)
    if isinstance(a, unreal.StaticMesh):
        unreal.log('[FRAG] %s bounds=%s colisao=%d' % (a.get_name(), a.get_bounding_box(), unreal.EditorStaticMeshLibrary.get_simple_collision_count(a)))
unreal.log('[FRAG] FIM %d' % n)
