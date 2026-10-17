import unreal
def log(*a): unreal.log("[VERTICE] " + " ".join(str(x) for x in a))
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem); EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LES.load_level("/Game/Vertice/Maps/Teste_Vertice")
for a in EAS.get_all_level_actors():
    if isinstance(a, unreal.SkyLight): a.light_component.set_intensity(2.0); log("luz ambiente 2.0")
LES.save_current_level(); log("FIM OK")
