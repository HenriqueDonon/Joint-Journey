import unreal
ell = unreal.EditorLevelLibrary
ell.load_level('/Game/Maps/Fase1_Experimental')
for a in ell.get_all_level_actors():
    l = a.get_actor_label()
    if any(k in l for k in ('Plataforma', 'Lava', 'Parede_Lateral', 'Ponte_Quebrada', 'Block')):
        o, e = a.get_actor_bounds(False)
        if e.x > 1: print('[MED]', l, 'X %.0f..%.0f Y %.0f..%.0f Z %.0f..%.0f' % (o.x-e.x, o.x+e.x, o.y-e.y, o.y+e.y, o.z-e.z, o.z+e.z))
