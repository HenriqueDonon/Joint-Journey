import unreal
EAL = unreal.EditorAssetLibrary
def log(*a): unreal.log("[NEON] " + " ".join(str(x) for x in a))
manny = unreal.load_asset("/Game/Characters/Mannequins/Meshes/SKM_Manny")
sk = unreal.load_asset("/Game/Neon/Mesh/SK_Neon")
rtg = unreal.load_asset("/Game/Neon/Rig/RTG_Manny_para_Neon")
rc = unreal.IKRetargeterController.get_controller(rtg)
# pes e cabeca: desfaz o alinhamento automatico (gira o pe/cabeca para a direcao do Manny)
try:
    T = unreal.RetargetSourceOrTarget.TARGET
    pn = rc.get_current_retarget_pose_name(T)
    rc.reset_retarget_pose(pn, ["foot_l","foot_r","ball_l","ball_r","neck_01","neck_02","head"], T)
    log("pose de pes/cabeca restaurada:", pn)
except Exception as e: log("reset:", e)
try:
    rs = rc.get_root_settings(); rs.set_editor_property("scale_horizontal", 0.0); rc.set_root_settings(rs)
    log("raiz: deslocamento horizontal removido")
except Exception as e: log("root:", e)
EAL.save_loaded_asset(rtg)
for n in ("NEO_Idle","NEO_Walk_InPlace","NEO_Run_Fwd","NEO_Jump","NEO_Fall_Loop","NEO_Land"):
    if EAL.does_asset_exist("/Game/Neon/Anims/" + n): EAL.delete_asset("/Game/Neon/Anims/" + n)
names = ("MM_Idle","MM_Walk_InPlace","MM_Run_Fwd","MM_Jump","MM_Fall_Loop","MM_Land")
ad = [EAL.find_asset_data("/Game/Characters/Mannequins/Animations/Manny/" + n) for n in names]
res = unreal.IKRetargetBatchOperation.duplicate_and_retarget(ad, manny, sk, rtg, "MM_", "NEO_", "", "", False)
ok = []
for r in res:
    p = str(r.package_name) if hasattr(r, "package_name") else r.get_path_name().split(".")[0]
    dst = "/Game/Neon/Anims/" + p.split("/")[-1]
    if p != dst:
        if EAL.does_asset_exist(dst): EAL.delete_asset(dst)
        EAL.rename_asset(p, dst)
    a = unreal.load_asset(dst)
    if isinstance(a, unreal.AnimSequence):
        a.set_editor_property("enable_root_motion", False); a.set_editor_property("force_root_lock", True)
        EAL.save_loaded_asset(a); ok.append(dst)
log("animacoes:", ok)
log("FIM OK")
