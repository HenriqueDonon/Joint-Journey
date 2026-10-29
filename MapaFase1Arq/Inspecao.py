import unreal
eal = unreal.EditorAssetLibrary
for d in ('/Niagara/DefaultAssets/Templates', '/Niagara/DefaultAssets', '/Engine/BasicShapes'):
    try: print('[INS]', d, [p.split('/')[-1] for p in eal.list_assets(d, recursive=True)][:60])
    except Exception as e: print('[INS] erro', d, e)
print('[INS] mats', [p for p in eal.list_assets('/Game/Fase1Arq/HF', recursive=True) if 'M_' in p.split('/')[-1]][:40])
print('[INS] sounds', [p for p in eal.list_assets('/Engine', recursive=True) if '/Sound' in p or 'Audio' in p][:20])
