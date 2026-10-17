import unreal
EAL = unreal.EditorAssetLibrary
def log(*a): unreal.log("[VERTICE] " + " ".join(str(x) for x in a))
manny = unreal.load_asset("/Game/Characters/Mannequins/Meshes/SKM_Manny")
sk = unreal.load_asset("/Game/Vertice/Mesh/SK_Vertice")
rtg = unreal.load_asset("/Game/Vertice/Rig/RTG_Manny_para_Vertice")
# cabeça/capuz acompanha o tronco (o capuz é grande e preso aos ombros): sem animação de pescoço/cabeça nesta versão
rc = unreal.IKRetargeterController.get_controller(rtg)
for ch in ("Head", "Neck"):
    try: rc.set_source_chain("None", ch); log("cadeia desligada:", ch)
    except Exception as e: log("set_source_chain", ch, e)
# animações no lugar: remove o deslocamento horizontal do quadril vindo do root motion do Manny (o personagem é movido pelo CharacterMovement)
try:
    rs = rc.get_root_settings()
    rs.set_editor_property("scale_horizontal", 0.0)
    rc.set_root_settings(rs); log("raiz: deslocamento horizontal removido")
except Exception as e: log("root settings:", e)
# pés: desfaz o alinhamento automático do pé/dedo (girava o pé do Vértice para a direção do pé do Manny -> ponta dos pés no repouso)
try:
    T = unreal.RetargetSourceOrTarget.TARGET
    pn = rc.get_current_retarget_pose_name(T)
    rc.reset_retarget_pose(pn, ["foot_l","foot_r","ball_l","ball_r","neck_01","neck_02","head"], T); log("pose dos pes restaurada:", pn)
except Exception as e: log("reset pes:", e)
EAL.save_loaded_asset(rtg)
for n in ("VRT_Idle","VRT_Walk_InPlace","VRT_Run_Fwd","VRT_Jump","VRT_Fall_Loop","VRT_Land"):
    if EAL.does_asset_exist("/Game/Vertice/Anims/" + n): EAL.delete_asset("/Game/Vertice/Anims/" + n)
names = ("MM_Idle","MM_Walk_InPlace","MM_Run_Fwd","MM_Jump","MM_Fall_Loop","MM_Land")
ad = [EAL.find_asset_data("/Game/Characters/Mannequins/Animations/Manny/" + n) for n in names]
res = unreal.IKRetargetBatchOperation.duplicate_and_retarget(ad, manny, sk, rtg, "MM_", "VRT_", "", "", False)
log("gerou:", [str(r.package_name) if hasattr(r, "package_name") else str(r) for r in res])
ok = []
for r in res:
    p = str(r.package_name) if hasattr(r, "package_name") else r.get_path_name().split(".")[0]
    nm = p.split("/")[-1]
    dst = "/Game/Vertice/Anims/" + nm
    if p != dst:
        if EAL.does_asset_exist(dst): EAL.delete_asset(dst)
        EAL.rename_asset(p, dst)
    a = unreal.load_asset(dst)
    if isinstance(a, unreal.AnimSequence):
        a.set_editor_property("enable_root_motion", False); a.set_editor_property("force_root_lock", True)
        EAL.save_loaded_asset(a); ok.append(dst)
log("animacoes:", ok)
log("FIM OK")
