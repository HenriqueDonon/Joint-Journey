#include "CoopF2Test.h"
#include "CoopCharacter.h"
#include "CoopElevator.h"
#include "CoopGate.h"
#include "CoopFase2GameMode.h"
#include "CoopDestructible.h"
#include "CoopConsole.h"
#include "CoopBalanceMachine.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"

void ACoopF2Flow::Go(int32 S, const FString& Why)
{
	const AGameStateBase* GS = GetWorld()->GetGameState();
	Log += FString::Printf(TEXT("EVT t=%.2f srvT=%.2f passo %d->%d %s\n"), Total, GS ? GS->GetServerWorldTimeSeconds() : 0.f, St, S, *Why);
	St = S; StT = 0.f; Wp = 0; Rec = 0.f; bFlag = false; Save();
}
void ACoopF2Flow::Save() { FFileHelper::SaveStringToFile(Log, *(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/f2fluxo/log_%s.txt"), *Tag))); }

void ACoopF2Flow::Tick(float Dt)
{
	Super::Tick(Dt);
	UWorld* W = GetWorld();
	if (Tag.IsEmpty()) { FParse::Value(FCommandLine::Get(), TEXT("FluxoTag="), Tag); if (Tag.IsEmpty()) Tag = TEXT("solo"); }
	APlayerController* PC = W->GetFirstPlayerController();
	ACoopCharacter* Me = PC ? Cast<ACoopCharacter>(PC->GetPawn()) : nullptr;
	AGameStateBase* GS = W->GetGameState();
	ACoopElevator* E = nullptr; for (TActorIterator<ACoopElevator> It(W); It; ++It) { E = *It; break; }
	if (!Me || !GS || !E) { Rec += Dt; if (Rec > 20.f && !bDone) { bDone = true; GEngine->Exec(W, TEXT("quit")); } return; }   // sem jogo (ex.: host saiu): fecha sozinho
	if (bDone) return;
	ACoopCharacter* O = nullptr; int32 NP = 0;
	for (TActorIterator<ACoopCharacter> It(W); It; ++It) { ++NP; if (*It != Me) O = *It; }
	ACoopFase2GameMode* GM = W->GetAuthGameMode<ACoopFase2GameMode>();   // so no host
	StT += Dt; ShotT += Dt; LogT += Dt; Total += Dt;
	const bool bNeon = Me->CoopRole == ECoopRole::Neon;
	const float Sg = bNeon ? -1.f : 1.f;            // Neon = sul (-Y), Ruina = norte (+Y)
	const FVector L = Me->GetActorLocation();
	const FVector Home = E->GetHome();
	const bool bTop = E->IsParkedAt(true) && E->SideDoorAlpha() > 0.95f, bBot = E->IsParkedAt(false) && E->FrontGateAlpha() > 0.95f;
	auto Walk = [&](FVector2D Tg, bool bJump = false) -> float
	{
		const FVector2D D = Tg - FVector2D(L.X, L.Y);
		if (D.Size() > 22.f) { Me->AddMovementInput(FVector(D.X, D.Y, 0).GetSafeNormal(), 1.f); PC->SetControlRotation(FRotator(-12.f, FMath::RadiansToDegrees(FMath::Atan2(D.Y, D.X)), 0)); }
		if (bJump && !Me->GetCharacterMovement()->IsFalling()) Me->Jump();
		return D.Size();
	};
	auto Path = [&](std::initializer_list<FVector2D> P, bool bJump = false) -> bool   // segue pontos; true no fim
	{
		const int32 Nn = (int32)P.size(); if (Wp >= Nn) return true;
		if (Walk(*(P.begin() + Wp), bJump) < 30.f) ++Wp;
		return Wp >= Nn;
	};
	auto InCab = [&](const FVector& P) { return FMath::Abs(P.X - E->GetActorLocation().X) < 95.f && FMath::Abs(P.Y - E->GetActorLocation().Y) < 95.f && P.Z - E->GetActorLocation().Z > 0.f && P.Z - E->GetActorLocation().Z < 250.f; };
	const bool bBoth = O && InCab(L) && InCab(O->GetActorLocation());
	auto Use = [&](const TCHAR* Why) { if (UFunction* F = Me->FindFunction(TEXT("ServerInteract"))) Me->ProcessEvent(F, nullptr); Log += FString::Printf(TEXT("ACAO t=%.2f tecla E (%s)\n"), Total, Why); };
	auto Res = [&](const FString& S) { Log += TEXT("RESULTADO ") + S + TEXT("\n"); Save(); };
	if (St >= 1 && ShotT >= 0.25f) { ShotT = 0.f; FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/f2fluxo/%s/f_%05d.jpg"), *Tag, Shots++), false, false); }
	if (LogT >= 0.5f)
	{
		LogT = 0.f;
		Log += FString::Printf(TEXT("t=%.2f passo=%d %s pos=(%.0f,%.0f,%.0f) chao=%d elevZ=%.1f topo=%d portasLat=%.2f portao=%.2f travado=%d vista=%s\n"), Total, St, bNeon ? TEXT("Neon") : TEXT("Ruina"),
			L.X, L.Y, L.Z, (int32)Me->GetCharacterMovement()->IsMovingOnGround(), E->CurrentOffsetZ(), (int32)E->IsTargetTop(), E->SideDoorAlpha(), E->FrontGateAlpha(), (int32)E->bLockedP1,
			PC->GetViewTarget() == Me ? TEXT("personagem") : *PC->GetViewTarget()->GetClass()->GetName());
	}
	if (Total > 260.f && St < 98) { Res(TEXT("TEMPO TOTAL ESGOTADO (260 s)")); Go(98, TEXT("tempo total")); }   // nunca fica preso
	const float TO = 30.f;   // tempo limite por passo
	if (St > 0 && St < 90 && StT > TO) { Res(FString::Printf(TEXT("TIMEOUT no passo %d pos=(%.0f,%.0f,%.0f)"), St, L.X, L.Y, L.Z)); Go(98, TEXT("timeout")); }
	switch (St)
	{
	case 0:   // apresentacao: Ruina pula com E aos 1,5 s; Neon assiste ate o fim
		if (NP < 2) break;
		if (PC->GetViewTarget() != Me && !bFlag) { bFlag = true; Rec = Total; Log += FString::Printf(TEXT("INTRO comecou t=%.2f (vista=%s)\n"), Total, *PC->GetViewTarget()->GetClass()->GetName()); }
		if (bFlag && !bNeon && Total - Rec > 1.5f && Wp == 0) { Use(TEXT("pular apresentacao")); Wp = 1; }
		if (StT > 60.f) { Res(TEXT("INTRO nao observada em 60 s")); Go(1, TEXT("sem intro")); break; }
		if (bFlag && PC->GetViewTarget() == Me) { Res(FString::Printf(TEXT("INTRO %s apos %.2f s (vista devolvida ao personagem)"), bNeon ? TEXT("completa") : TEXT("pulada"), Total - Rec)); Go(1, TEXT("controle devolvido")); }
		break;
	case 1: if (Path({ FVector2D(700, Sg * -45.f), FVector2D(1200, Sg * -45.f) }) && bBoth && StT > 2.f) Go(2, TEXT("os dois na cabine (A)")); break;
	case 2:   // elevador travado (P1 nao resolvido): pedido deve ser recusado
		if (bNeon && StT > 0.5f && !bFlag) { bFlag = true; Use(TEXT("cabine com trava do P1")); }
		if (StT > 3.f) { Res(FString::Printf(TEXT("TRAVA P1: pedido com elevador travado -> %s"), E->IsMoving() || E->IsTargetTop() ? TEXT("PARTIU (FALHA)") : TEXT("recusado, ficou parado (OK)"))); Go(3, TEXT("teste da trava")); }
		break;
	case 3:   // DEPURACAO: destrava so o elevador (host) e pede a subida pelo console
		if (GM && !bFlag && StT > 0.5f) { bFlag = true; E->ServerSetLocked(false); Log += TEXT("DEPURACAO host: trava do elevador liberada para teste (NAO e o Puzzle 1)\n"); }
		if (bNeon && StT > 1.5f && StT < 1.5f + Dt * 1.5f) Use(TEXT("subir"));
		if (E->IsMoving() && Rec == 0.f) { Rec = 1.f; Log += FString::Printf(TEXT("ELEV partiu t=%.2f\n"), Total); }
		if (E->IsMoving() && InCab(L)) Rec = FMath::Max(Rec, 1.f + FMath::Abs((L.Z - E->GetActorLocation().Z) - 90.f));
		if (bTop && StT > 3.f) { Res(FString::Printf(TEXT("SUBIDA A->B: chegou, eu a bordo=%d, desvio altura rel=%.1f cm, portas laterais abertas"), (int32)InCab(L), Rec - 1.f)); Go(4, TEXT("topo")); }
		break;
	case 4: if (Path({ FVector2D(1200, Sg * 40.f), FVector2D(1200, Sg * 470.f) })) { Res(FString::Printf(TEXT("BARREIRA PROPRIA (%s): atravessou, Y=%.0f"), bNeon ? TEXT("ciano") : TEXT("roxa"), L.Y)); Go(5, TEXT("na sala propria")); } break;
	case 5:   // tenta a barreira do outro (andando e pulando), depois volta para a propria sala
		if (!bFlag && Path({ FVector2D(1200, Sg * 40.f) })) { StT = 0.f; bFlag = true; }
		if (bFlag)
		{
			Walk(FVector2D(1200, -Sg * 520.f), true); Rec = FMath::Max(Rec, -Sg * L.Y);
			if (StT > 6.f) { Res(FString::Printf(TEXT("BARREIRA DO OUTRO: avancou ate |Y|=%.0f (barreira em 220..240) -> %s"), Rec, Rec < 215.f ? TEXT("BLOQUEADO (OK)") : TEXT("PASSOU (FALHA)"))); Go(6, TEXT("bloqueio testado")); }
		}
		break;
	case 6: if (bNeon) { if (Path({ FVector2D(1200, -40.f) }) && StT > 1.f && O && O->GetActorLocation().Y > 380.f) Go(7, TEXT("Neon na cabine, Ruina na sala norte")); }
		else if (Path({ FVector2D(1200, 470.f) }) && StT > 1.f) Go(7, TEXT("Ruina aguarda na sala norte")); break;
	case 7:   // Neon desce sozinho; Ruina chama o elevador pelo console da sala norte
		if (bNeon)
		{
			if (!bFlag && StT > 1.f) { bFlag = true; Use(TEXT("descer sozinho")); }
			if (bFlag && bBot && E->CurrentOffsetZ() < 1.f && Path({ FVector2D(800, 0) })) Go(8, TEXT("Neon desembarcou em A"));
		}
		else
		{
			if (!bFlag && E->IsParkedAt(false) && E->FrontGateAlpha() > 0.95f && O && O->GetActorLocation().X < 1000.f && Path({ FVector2D(1250, 470) })) { bFlag = true; Use(TEXT("chamar elevador (B norte)")); }
			if (bFlag && bTop && Path({ FVector2D(1200, 470), FVector2D(1200, 40) }) && StT > 1.f) { Use(TEXT("descer com o elevador")); Go(8, TEXT("Ruina desce")); }
		}
		break;
	case 8:   // reembarque em A e subida juntos (Ruina pede)
		if (bNeon) { if ((bFlag || (E->IsParkedAt(false) && E->FrontGateAlpha() > 0.95f && O && O->GetActorLocation().Z < 300.f)) && Path({ FVector2D(1200, -45) })) bFlag = true; if (bFlag && (bBoth || (E->IsMoving() && InCab(L)))) Go(9, TEXT("os dois na cabine de novo")); }
		else { if (bBot && Path({ FVector2D(1200, 45) }) && bBoth && StT > 2.f) { Use(TEXT("subir de novo")); Go(9, TEXT("Ruina pediu subida")); } }
		break;
	case 9:
		if (!bNeon && !E->IsMoving() && !E->IsTargetTop() && StT > 2.f && FMath::Fmod(StT, 2.f) < Dt * 1.01f) Use(TEXT("subir de novo (pedido repetido: o anterior foi recusado)"));
		if (bTop && StT > 1.f) { Res(TEXT("RETORNO/CHAMADA: Neon desceu sozinho, Ruina chamou de B, desceu, os dois subiram de novo (OK)")); Go(10, TEXT("topo de novo")); }
		break;
	case 10:   // cada um vai ate o portao P2 da propria sala (fechado): deve bloquear
		if (Path({ FVector2D(1200, Sg * 470.f), FVector2D(1900, Sg * 470.f), FVector2D(1900, Sg * 750.f), FVector2D(2150, Sg * 750.f) }, StT > 9.f)) {}
		Rec = FMath::Max(Rec, L.X);
		if (StT > 12.f) { Res(FString::Printf(TEXT("PORTAO P2 FECHADO: X max=%.0f (portao em 1950..1970) -> %s"), Rec, Rec < 1950.f ? TEXT("BLOQUEADO (OK)") : TEXT("PASSOU (FALHA)"))); Go(11, TEXT("portao testado")); }
		break;
	case 11:   // DEPURACAO (host): abre os portoes (P2/P3 ainda nao existem) -> rota de reencontro
		if (GM && !GM->bDebugUnlocked && StT > 1.f) { GM->DebugUnlockAll(); Log += TEXT("DEPURACAO host: F10 portoes abertos (NAO e solucao de puzzle)\n"); }
		if (StT > 2.5f && Path({ FVector2D(2150, Sg * 750.f), FVector2D(2150, Sg * 300.f) }) && O && FVector::Dist2D(L, O->GetActorLocation()) < 700.f && StT > 4.f)
		{ Res(FString::Printf(TEXT("REENCONTRO: os dois no reencontro, distancia %.0f cm"), FVector::Dist2D(L, O->GetActorLocation()))); Go(12, TEXT("reencontro")); }
		break;
	case 12: if ((bNeon || StT > 3.f) && Path({ FVector2D(bNeon ? 2195.f : 2110.f, -1080), FVector2D(2375, -1080), FVector2D(2375, 300), FVector2D(2375, Sg > 0 ? 520.f : 420.f) })) { Res(FString::Printf(TEXT("ESCADA B->C: chegou em C, Z=%.0f"), L.Z)); Go(13, TEXT("em C")); } break;
	case 13:   // Neon: salto proposital por cima do guarda-corpo para dentro do poco -> retorno ao ponto seguro de C
		if (bNeon)
		{
			if (!bFlag) { if (Path({ FVector2D(1700, 420), FVector2D(700, 420), FVector2D(700, 330) })) { bFlag = true; Rec = L.Z; } }
			else { Walk(FVector2D(700, 100), true); Rec = FMath::Min(Rec, L.Z); if (Rec < 900.f && L.Z > 1250.f && !Me->GetCharacterMovement()->IsFalling()) { Res(FString::Printf(TEXT("QUEDA no poco: caiu ate Z=%.0f e voltou ao ponto seguro de C (%.0f,%.0f,%.0f)"), Rec, L.X, L.Y, L.Z)); Go(14, TEXT("recuperado")); } }
		}
		else if (StT > 2.f) Go(14, TEXT("Ruina segue"));
		break;
	case 14: if (Path({ FVector2D(2375, 700), FVector2D(2375, 1210), FVector2D(bNeon ? 2625.f : 2725.f, 1210), FVector2D(bNeon ? 2625.f : 2725.f, 120), FVector2D(2500, Sg > 0 ? -450.f : -750.f) })) { Res(FString::Printf(TEXT("ESCADA C->D: chegou em D, Z=%.0f (porta de saida a %.0f cm)"), L.Z, FVector::Dist(L, FVector(2800, -600, 2000)))); Go(15, TEXT("em D")); } break;
	case 15: PC->SetControlRotation(FRotator(5.f, 0.f, 0.f)); if (StT > 3.f) { Res(TEXT("FIM: percurso A->B->reencontro->C->D concluido")); Go(99, TEXT("fim")); } break;
	case 98: case 99: if (StT > (Tag == TEXT("host") ? 8.f : 4.f)) { Save(); bDone = true; GEngine->Exec(W, TEXT("quit")); } break;
	}
}

// ---------------- vistas fixas do mapa (capturas para o relatorio) ----------------
void ACoopF2Views::Tick(float Dt)
{
	Super::Tick(Dt);
	UWorld* W = GetWorld();
	APlayerController* PC = W->GetFirstPlayerController();
	if (!PC || !PC->GetPawn()) return;
	T += Dt;
	struct FV { const TCHAR* Name; FVector P, Tg; float Fov; float HideAbove; bool bCut; };
	static const FV V[] = {
		{ TEXT("01_vista_geral_poco"), FVector(-500, -1150, 2500), FVector(1300, 300, 600), 85, 1e9f, false },
		{ TEXT("02_planta_A_z0"), FVector(1100, 0, 9000), FVector(1100, 1, 0), 22, 560, false },
		{ TEXT("03_planta_B_z600"), FVector(1100, 0, 9000), FVector(1100, 1, 600), 22, 1150, false },
		{ TEXT("04_planta_C_z1200"), FVector(1100, 0, 9000), FVector(1100, 1, 1200), 22, 1750, false },
		{ TEXT("05_planta_D_z1800"), FVector(1100, 0, 9000), FVector(1100, 1, 1800), 22, 1e9f, false },
		{ TEXT("06_corte_lateral_alturas"), FVector(1100, -9000, 1200), FVector(1100, 0, 1200), 22, 1e9f, true },
		{ TEXT("07_elevador_A"), FVector(300, -200, 420), FVector(1150, 0, 150), 75, 1e9f, false },
		{ TEXT("08_B_portas_barreiras"), FVector(700, 750, 1900), FVector(1200, 0, 650), 80, 1150, false },
		{ TEXT("09_P1_area"), FVector(300, 0, 500), FVector(1150, 0, 150), 75, 1e9f, false },
		{ TEXT("10_P2_sala_norte_ruina"), FVector(500, 1150, 1100), FVector(1500, 600, 650), 85, 1150, false },
		{ TEXT("11_P2_sala_sul_neon"), FVector(1900, -1200, 1100), FVector(900, -600, 650), 85, 1150, false },
		{ TEXT("12_P3_trajetoria_esfera"), FVector(300, -300, 1800), FVector(1200, -800, 1300), 80, 1e9f, false },
		{ TEXT("13_D_saida"), FVector(1500, 600, 2150), FVector(2800, -600, 2000), 75, 1e9f, false },
		{ TEXT("14_reencontro_escadas"), FVector(2000, 1100, 1450), FVector(2450, -600, 700), 85, 1750, false },
	};
	static const FV V2[] = {
		{ TEXT("P2_01_sala_norte_ruina"), FVector(1000, 1190, 1120), FVector(1560, 560, 860), 88, 1e9f, false },
		{ TEXT("P2_02_sala_sul_neon"), FVector(1000, -1190, 1120), FVector(1560, -560, 860), 88, 1e9f, false },
		{ TEXT("P2_03_hub_pela_janela_norte"), FVector(1700, 700, 960), FVector(1700, 0, 940), 70, 1e9f, false },
		{ TEXT("P2_04_freio_e_bracadeiras"), FVector(1450, 360, 900), FVector(1450, 800, 800), 85, 1e9f, false },
		{ TEXT("P2_05_calha_e_pino"), FVector(1000, -420, 1000), FVector(1450, -700, 900), 85, 1e9f, false },
		{ TEXT("P2_06_planta_B"), FVector(1300, 0, 9000), FVector(1300, 1, 600), 20, 1150, false },
	};
	static const bool bP2Views = FParse::Param(FCommandLine::Get(), TEXT("F2VistasP2"));
	const FV* VV = bP2Views ? V2 : V;
	const int32 NV = bP2Views ? UE_ARRAY_COUNT(V2) : UE_ARRAY_COUNT(V);
	static const bool bIntroRec = FParse::Param(FCommandLine::Get(), TEXT("F2Intro"));
	if (bIntroRec)   // grava a apresentacao quadro a quadro (com -usefixedtimestep -fps=30) e o retorno ao personagem
	{
		if (T > 2.5f && T < 10.f) FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/f2intro/f_%05d.jpg"), S++), false, false);
		if (T > 10.5f) GEngine->Exec(W, TEXT("quit"));
		return;
	}
	if (T < 4.f) return;
	const int32 I = (int32)((T - 4.f) / 1.5f);
	const float Ph = FMath::Fmod(T - 4.f, 1.5f);
	if (I >= NV) { if (T > 4.f + NV * 1.5f + 1.f) GEngine->Exec(W, TEXT("quit")); return; }
	if (I != S - 1 || !Cam)
	{
		if (!Cam) Cam = W->SpawnActor<ACameraActor>();
		S = I + 1;
		for (TActorIterator<AStaticMeshActor> It(W); It; ++It)
		{
			FVector O, Ext; It->GetActorBounds(false, O, Ext);
			It->SetActorHiddenInGame((O.Z - Ext.Z) > VV[I].HideAbove || (VV[I].bCut && It->ActorHasTag(TEXT("CorteSul"))));
		}
		Cam->SetActorLocationAndRotation(VV[I].P, (VV[I].Tg - VV[I].P).Rotation());
		Cam->GetCameraComponent()->SetFieldOfView(VV[I].Fov);
		PC->SetViewTargetWithBlend(Cam, 0.f);
	}
	if (Ph > 0.9f && Ph < 0.9f + Dt * 1.5f) FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/f2vistas/%s.png"), VV[I].Name), false, false);
}

// =================== PUZZLE 1 ===================
void ACoopF2P1Flow::Go(int32 S, const FString& Why)
{
	const AGameStateBase* GS = GetWorld()->GetGameState();
	Log += FString::Printf(TEXT("EVT t=%.2f srvT=%.2f passo %d->%d %s\n"), Total, GS ? GS->GetServerWorldTimeSeconds() : 0.f, St, S, *Why);
	St = S; StT = 0.f; Wp = 0; Sub = 0; SubT = 0.f; Save();
}
void ACoopF2P1Flow::Save() { FFileHelper::SaveStringToFile(Log, *(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/f2p1/log_%s.txt"), *Tag))); }

void ACoopF2P1Flow::Tick(float Dt)
{
	Super::Tick(Dt);
	UWorld* W = GetWorld();
	if (Tag.IsEmpty()) { FParse::Value(FCommandLine::Get(), TEXT("FluxoTag="), Tag); if (Tag.IsEmpty()) Tag = TEXT("solo"); }
	APlayerController* PC = W->GetFirstPlayerController();
	ACoopCharacter* Me = PC ? Cast<ACoopCharacter>(PC->GetPawn()) : nullptr;
	AGameStateBase* GS = W->GetGameState();
	ACoopElevator* E = nullptr; for (TActorIterator<ACoopElevator> It(W); It; ++It) { E = *It; break; }
	ACoopDestructible *Tr = nullptr, *Su = nullptr;
	for (TActorIterator<ACoopDestructible> It(W); It; ++It)
		if (FMath::Abs(It->GetActorLocation().X - 1200.f) < 10.f && It->GetActorLocation().Z < 400.f) { if (It->GetActorLocation().Y > 0.f) Tr = *It; else Su = *It; }
	if (!Me || !GS || !E || !Tr || !Su) { NoGame += Dt; if (NoGame > 25.f && !bDone) { bDone = true; GEngine->Exec(W, TEXT("quit")); } return; }
	if (bDone) return;
	static const bool bSwap = FParse::Param(FCommandLine::Get(), TEXT("F2Swap"));
	if (!bRoleSet && Total > 1.f)
	{
		bRoleSet = true;
		if (bSwap)   // RPC de teste ja existente (ServerSetRole): host vira Ruina, cliente vira Neon
			if (UFunction* F = Me->FindFunction(TEXT("ServerSetRole"))) { uint8 R = (uint8)(Tag == TEXT("host") ? ECoopRole::Ruina : ECoopRole::Neon); Me->ProcessEvent(F, &R); }
	}
	ACoopCharacter* O = nullptr; int32 NP = 0;
	for (TActorIterator<ACoopCharacter> It(W); It; ++It) { ++NP; if (*It != Me) O = *It; }
	StT += Dt; SubT += Dt; ShotT += Dt; LogT += Dt; Total += Dt;
	const bool bNeon = Me->CoopRole == ECoopRole::Neon;
	const FVector L = Me->GetActorLocation();
	const float Srv = GS->GetServerWorldTimeSeconds();
	auto Walk = [&](FVector2D Tg) -> float
	{
		const FVector2D D = Tg - FVector2D(L.X, L.Y);
		if (D.Size() > 22.f) { Me->AddMovementInput(FVector(D.X, D.Y, 0).GetSafeNormal(), 1.f); PC->SetControlRotation(FRotator(-12.f, FMath::RadiansToDegrees(FMath::Atan2(D.Y, D.X)), 0)); }
		return D.Size();
	};
	auto At = [&](FVector2D Tg) { return Walk(Tg) < 30.f; };
	auto Face = [&](const FVector& P) { const FVector D = P - L; PC->SetControlRotation(FRotator(-10.f, D.Rotation().Yaw, 0)); };
	auto Use = [&](const TCHAR* Why) { if (UFunction* F = Me->FindFunction(TEXT("ServerInteract"))) Me->ProcessEvent(F, nullptr); Log += FString::Printf(TEXT("ACAO t=%.2f tecla E (%s)\n"), Total, Why); };
	auto Fire = [&](ACoopDestructible* T, const TCHAR* Why)
	{
		Face(T->GetActorLocation());
		if (UFunction* F = Me->FindFunction(TEXT("ServerPowerShot")))
		{
			struct { ACoopDestructible* Target; FVector_NetQuantize10 End; } Parm{ T, FVector_NetQuantize10(T->GetActorLocation()) };
			Me->ProcessEvent(F, &Parm);
		}
		Log += FString::Printf(TEXT("ACAO t=%.2f %s atira o poder em %s (%s)\n"), Total, bNeon ? TEXT("Neon") : TEXT("Ruina"), T == Tr ? TEXT("TRAVA") : TEXT("SUPORTE"), Why);
	};
	auto InCab = [&](const FVector& P) { return FMath::Abs(P.X - E->GetActorLocation().X) < 95.f && FMath::Abs(P.Y - E->GetActorLocation().Y) < 95.f && P.Z - E->GetActorLocation().Z > 0.f && P.Z - E->GetActorLocation().Z < 250.f; };
	auto Res = [&](const FString& S) { Log += TEXT("RESULTADO ") + S + TEXT("\n"); Save(); };
	auto PromptHere = [&]() -> FString
	{
		const ACoopConsole* B = nullptr; float BD = 1e9f;
		for (TActorIterator<ACoopConsole> It(W); It; ++It) { const float D = FVector::Dist(It->GetActorLocation(), L); if (D < It->UseRadius && D < BD) { BD = D; B = *It; } }
		if (!B) return FString(TEXT("(nenhum console ao alcance)"));
		int32 S = 1; const FString P = B->Prompt(S); return P + (S == 2 ? TEXT(" [verde: disponivel]") : S == 0 ? TEXT(" [vermelho: bloqueado]") : TEXT(" [cinza: sem efeito]"));
	};
	// camera de inspecao no host (-InspP1): mostra a trava, o suporte e a cabine nos momentos certos (so para o video)
	static const bool bInsp = FParse::Param(FCommandLine::Get(), TEXT("InspP1"));
	if (bInsp && Tag == TEXT("host") && St >= 1)
	{
		static ACameraActor* IC = nullptr;
		if (!IC) { IC = W->SpawnActor<ACameraActor>(); IC->GetCameraComponent()->SetFieldOfView(80.f); }
		FVector CP(420, -60, 420), CT(1150, 0, 180);
		if (St == 3) { CP = FVector(640, 285, 330); CT = FVector(1200, 205, 160); }
		else if (St == 4) { CP = FVector(640, -285, 330); CT = FVector(1200, -200, 120); }
		else if (St == 6) { CP = FVector(560, -40, 650); CT = FVector(1200, 0, 450); }
		IC->SetActorLocationAndRotation(CP, (CT - CP).Rotation());
		if (St >= 7) { if (PC->GetViewTarget() != Me) PC->SetViewTargetWithBlend(Me, 0.f); }
		else if (PC->GetViewTarget() != IC && PC->GetViewTarget() == Me) PC->SetViewTargetWithBlend(IC, 0.f);
	}
	if (St >= 1 && ShotT >= 0.2f) { ShotT = 0.f; FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/f2p1/%s/f_%05d.jpg"), *Tag, Shots++), false, false); }
	if (LogT >= 0.5f)
	{
		LogT = 0.f;
		Log += FString::Printf(TEXT("t=%.2f passo=%d %s pos=(%.0f,%.0f,%.0f) trava=%s suporte=%s elevTravado=%d elevZ=%.0f portasLat=%.2f\n"), Total, St, bNeon ? TEXT("Neon") : TEXT("Ruina"), L.X, L.Y, L.Z,
			Tr->IsDestroyed() ? TEXT("DESTRUIDA") : TEXT("inteira"), Su->IsDestroyed() ? TEXT("ausente") : TEXT("CONSTRUIDO"), (int32)E->bLockedP1, E->CurrentOffsetZ(), E->SideDoorAlpha());
	}
	if (Total > 205.f && St < 98) { Res(TEXT("TEMPO TOTAL ESGOTADO")); Go(98, TEXT("tempo total")); }
	if (St > 0 && St < 90 && StT > 40.f) { Res(FString::Printf(TEXT("TIMEOUT no passo %d sub %d pos=(%.0f,%.0f,%.0f)"), St, Sub, L.X, L.Y, L.Z)); Go(98, TEXT("timeout")); }
	const FVector2D NeonSpot(900, -255), RuinaSpot(900, 255), Wait(680, 120), NeonWait(680, -120);
	switch (St)
	{
	case 0:   // papel e apresentacao (os dois pulam com E)
		if (NP < 2 || Total < 3.f) break;
		if (PC->GetViewTarget() != Me && Sub == 0 && SubT > 1.f) { Use(TEXT("pular apresentacao")); Sub = 1; }
		if (PC->GetViewTarget() == Me && StT > 4.f && E->bLockedP1)
		{ Res(FString::Printf(TEXT("A. INICIO: papel=%s, elevador TRAVADO=%d, trava=%s, suporte=%s"), bNeon ? TEXT("Neon") : TEXT("Ruina"), (int32)E->bLockedP1, Tr->IsDestroyed() ? TEXT("destruida") : TEXT("inteira"), Su->IsDestroyed() ? TEXT("ausente") : TEXT("construido"))); Go(1, TEXT("controle")); }
		break;
	case 1: if (At(bNeon ? NeonSpot : Wait)) Go(2, TEXT("posicionado")); break;
	case 2:   // Neon: console travado (B), tentar reconstruir sem material (D), tentar destruir a trava (C)
		if (!bNeon) { if (O && O->GetActorLocation().Y < -150.f && Tr->IsDestroyed() == false && StT > 1.f && Sub == 0) Sub = 0; Walk(Wait); if (O && FVector::Dist2D(O->GetActorLocation(), FVector(NeonWait.X, NeonWait.Y, 0)) < 60.f && StT > 3.f) Go(3, TEXT("Neon terminou as tentativas")); break; }
		if (Sub == 0) { Res(TEXT("INDICADOR console A: ") + PromptHere()); Use(TEXT("console de chamada com o elevador travado")); Sub = 1; SubT = 0.f; }
		else if (Sub == 1 && SubT > 2.5f) { Res(FString::Printf(TEXT("B. CONSOLE ANTES DO PUZZLE: elevador %s"), E->IsMoving() || E->CurrentOffsetZ() > 1.f ? TEXT("PARTIU (FALHA)") : TEXT("nao partiu (OK, travado)"))); Fire(Su, TEXT("reconstruir sem material")); Sub = 2; SubT = 0.f; }
		else if (Sub == 2 && SubT > 1.5f) { Res(FString::Printf(TEXT("D. NEON RECONSTRUIR SEM MATERIAL: suporte %s"), Su->IsDestroyed() ? TEXT("continua ausente (BLOQUEADO, OK)") : TEXT("CONSTRUIDO (FALHA)"))); Sub = 3; SubT = 0.f; }
		else if (Sub == 3) { if (At(RuinaSpot)) { Fire(Tr, TEXT("Neon tenta destruir a trava")); Sub = 4; SubT = 0.f; } }
		else if (Sub == 4 && SubT > 1.5f) { Res(FString::Printf(TEXT("C. NEON DESTRUIR A TRAVA: trava %s"), Tr->IsDestroyed() ? TEXT("DESTRUIDA (FALHA)") : TEXT("intacta (BLOQUEADO, OK)"))); Sub = 5; }
		else if (Sub == 5 && At(NeonWait)) Go(3, TEXT("Neon aguarda"));
		break;
	case 3:   // Ruina: tenta reconstruir o suporte (nao pode) e destroi a trava
		if (bNeon) { Walk(NeonWait); if (Tr->IsDestroyed() && StT > 1.f) { T0 = Total; Go(4, TEXT("trava destruida (visto pelo Neon)")); } break; }
		if (Sub == 0) { if (At(NeonSpot)) { Fire(Su, TEXT("Ruina tenta reconstruir o suporte")); Sub = 1; SubT = 0.f; } }
		else if (Sub == 1 && SubT > 1.5f) { Res(FString::Printf(TEXT("RUINA NO SUPORTE: %s"), Su->IsDestroyed() ? TEXT("continua ausente (Ruina nao reconstroi, OK)") : TEXT("CONSTRUIDO (FALHA)"))); Sub = 2; }
		else if (Sub == 2) { if (At(RuinaSpot)) { Fire(Tr, TEXT("Ruina destroi a trava")); Sub = 3; SubT = 0.f; } }
		else if (Sub == 3 && SubT > 1.f) { Res(FString::Printf(TEXT("E. RUINA DESTROI A TRAVA: %s"), Tr->IsDestroyed() ? TEXT("DESTRUIDA (OK)") : TEXT("intacta (FALHA)"))); Sub = 4; }
		else if (Sub == 4 && At(Wait)) { T0 = Total; Go(4, TEXT("Ruina aguarda")); }
		break;
	case 4:   // F: elevador continua travado; Neon tenta o console de novo e depois reconstroi o suporte
		if (!bNeon) { Walk(Wait); if (!E->bLockedP1) { Res(FString::Printf(TEXT("I. (Ruina ve) elevador LIBERADO %.2f s depois do suporte comecar"), Total - T0)); Go(5, TEXT("liberado")); } else if (!Su->IsDestroyed() && T0 > 0.f && Sub == 0) { T0 = Total; Sub = 1; } break; }
		if (Sub == 0) { if (StT > 2.f && At(NeonSpot)) { Res(FString::Printf(TEXT("F. APOS DESTRUIR A TRAVA: elevador travado=%d (esperado 1)"), (int32)E->bLockedP1)); Res(TEXT("INDICADOR console A: ") + PromptHere()); Use(TEXT("console com trava destruida e sem suporte")); Sub = 1; SubT = 0.f; } }
		else if (Sub == 1 && SubT > 2.f) { Res(FString::Printf(TEXT("F2. CONSOLE COM SO A TRAVA DESTRUIDA: elevador %s"), E->IsMoving() || E->CurrentOffsetZ() > 1.f ? TEXT("PARTIU (FALHA)") : TEXT("nao partiu (OK)"))); Fire(Su, TEXT("Neon reconstroi o suporte com o material")); Sub = 2; SubT = 0.f; T0 = Total; }
		else if (Sub == 2 && SubT > 0.7f && SubT < 0.7f + Dt * 1.5f) Res(FString::Printf(TEXT("H. DURANTE A MONTAGEM (+0,7 s): suporte=%s, elevador travado=%d (esperado 1)"), Su->IsDestroyed() ? TEXT("ausente") : TEXT("construido"), (int32)E->bLockedP1));
		else if (Sub == 2 && !E->bLockedP1) { Res(FString::Printf(TEXT("G/I. SUPORTE RECONSTRUIDO e elevador LIBERADO %.2f s depois do tiro (montagem ~1,44 s)"), Total - T0)); Res(TEXT("INDICADOR console A: ") + PromptHere()); Sub = 3; SubT = 0.f; }
		else if (Sub == 3 && At(NeonWait)) Go(5, TEXT("Neon aguarda"));
		break;
	case 5:   // L: nada de consumo duplicado: Ruina tenta destruir o suporte pronto; Neon tenta reconstruir a trava
		if (Sub == 0 && StT > (bNeon ? 1.f : 4.5f)) { if (At(bNeon ? RuinaSpot : NeonSpot)) { Fire(bNeon ? Tr : Su, bNeon ? TEXT("Neon tenta reconstruir a trava") : TEXT("Ruina tenta destruir o suporte pronto")); Sub = 1; SubT = 0.f; } }
		else if (Sub == 1 && SubT > 1.5f) { Res(FString::Printf(TEXT("L. SEM DUPLICIDADE: trava=%s suporte=%s elevTravado=%d (esperado: destruida/construido/0)"), Tr->IsDestroyed() ? TEXT("destruida") : TEXT("INTEIRA"), Su->IsDestroyed() ? TEXT("AUSENTE") : TEXT("construido"), (int32)E->bLockedP1)); Sub = 2; }
		else if (Sub == 2 && At(FVector2D(800, bNeon ? -60.f : 60.f)) && StT > 9.f) Go(6, TEXT("rumo a cabine"));
		break;
	case 6:   // J: embarque e subida juntos (Ruina aciona; repete se recusado)
		if (At(FVector2D(1200, bNeon ? -45.f : 45.f)) && O && InCab(L) && InCab(O->GetActorLocation()))
		{
			if (!bNeon && !E->IsMoving() && !E->IsTargetTop() && SubT > 2.f) { Use(TEXT("subir")); SubT = 0.f; }
		}
		if (E->IsMoving() && Sub == 0) { Sub = 1; Res(TEXT("INDICADOR cabine: ") + PromptHere()); }
		if (E->IsParkedAt(true) && E->SideDoorAlpha() > 0.95f) { Res(FString::Printf(TEXT("J. SUBIDA JUNTOS: chegou em B, eu a bordo=%d, Z=%.0f"), (int32)InCab(L), L.Z)); Go(7, TEXT("em B")); }
		break;
	case 7: { const float Sg = bNeon ? -1.f : 1.f;   // K: barreiras do setor B
		if (Sub == 0) { if (At(FVector2D(1200, Sg * 40.f)) && At(FVector2D(1200, Sg * 40.f))) Sub = 1; }
		else if (Sub == 1) { if (At(FVector2D(1200, Sg * 470.f))) { Res(FString::Printf(TEXT("K. BARREIRA PROPRIA: atravessou (Y=%.0f)"), L.Y)); Sub = 2; } }
		else if (Sub == 2) { if (At(FVector2D(1200, Sg * 40.f))) { Sub = 3; SubT = 0.f; T0 = 0.f; } }
		else if (Sub == 3) { Walk(FVector2D(1200, -Sg * 520.f)); if (!Me->GetCharacterMovement()->IsFalling() && SubT > 1.f) Me->Jump(); T0 = FMath::Max(T0, -Sg * L.Y); if (SubT > 6.f) { Res(FString::Printf(TEXT("K. BARREIRA DO OUTRO: |Y| max %.0f -> %s"), T0, T0 < 215.f ? TEXT("BLOQUEADO (OK)") : TEXT("PASSOU (FALHA)"))); Go(8, TEXT("barreiras")); } }
		break; }
	case 8: if (At(FVector2D(1200, (bNeon ? -1.f : 1.f) * 470.f)) && StT > 2.f) { Res(TEXT("FIM P1: puzzle concluido em cooperacao e setor B funcionando")); Go(99, TEXT("fim")); } break;
	case 98: case 99: if (StT > (Tag == TEXT("host") ? 8.f : 4.f)) { Save(); bDone = true; GEngine->Exec(W, TEXT("quit")); } break;
	}
}

// =================== PUZZLE 2 ===================
void ACoopF2P2Flow::Go(int32 S, const FString& Why)
{
	const AGameStateBase* GS = GetWorld()->GetGameState();
	Log += FString::Printf(TEXT("EVT t=%.2f srvT=%.2f passo %d->%d %s\n"), Total, GS ? GS->GetServerWorldTimeSeconds() : 0.f, St, S, *Why);
	St = S; StT = 0.f; Wp = 0; Sub = 0; SubT = 0.f; Save();
}
void ACoopF2P2Flow::Save() { FFileHelper::SaveStringToFile(Log, *(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/f2p2/log_%s.txt"), *Tag))); }

void ACoopF2P2Flow::Tick(float Dt)
{
	Super::Tick(Dt);
	UWorld* W = GetWorld();
	if (Tag.IsEmpty()) { FParse::Value(FCommandLine::Get(), TEXT("FluxoTag="), Tag); if (Tag.IsEmpty()) Tag = TEXT("solo"); }
	APlayerController* PC = W->GetFirstPlayerController();
	ACoopCharacter* Me = PC ? Cast<ACoopCharacter>(PC->GetPawn()) : nullptr;
	AGameStateBase* GS = W->GetGameState();
	ACoopElevator* E = nullptr; for (TActorIterator<ACoopElevator> It(W); It; ++It) { E = *It; break; }
	ACoopBalanceMachine* M = nullptr; for (TActorIterator<ACoopBalanceMachine> It(W); It; ++It) { M = *It; break; }
	ACoopDestructible *Tr = nullptr, *Su = nullptr;
	for (TActorIterator<ACoopDestructible> It(W); It; ++It)
		if (FMath::Abs(It->GetActorLocation().X - 1200.f) < 10.f && It->GetActorLocation().Z < 400.f) { if (It->GetActorLocation().Y > 0.f) Tr = *It; else Su = *It; }
	if (!Me || !GS || !E || !M || !Tr || !Su || !M->Brake || !M->Chute || !M->Pin || !M->Clamp0) { NoGame += Dt; if (NoGame > 25.f && !bDone) { bDone = true; GEngine->Exec(W, TEXT("quit")); } return; }
	if (bDone) return;
	static const bool bSwap = FParse::Param(FCommandLine::Get(), TEXT("F2Swap"));
	if (!bRoleSet && Total > 1.f)
	{
		bRoleSet = true;
		if (bSwap) if (UFunction* F = Me->FindFunction(TEXT("ServerSetRole"))) { uint8 R = (uint8)(Tag == TEXT("host") ? ECoopRole::Ruina : ECoopRole::Neon); Me->ProcessEvent(F, &R); }
		int32 Lag = 0;
		if (Tag == TEXT("client") && FParse::Value(FCommandLine::Get(), TEXT("F2Lag="), Lag) && Lag > 0) { GEngine->Exec(W, *FString::Printf(TEXT("Net PktLag=%d"), Lag)); Log += FString::Printf(TEXT("LATENCIA simulada no cliente: %d ms\n"), Lag); }
	}
	ACoopCharacter* O = nullptr; int32 NP = 0;
	for (TActorIterator<ACoopCharacter> It(W); It; ++It) { ++NP; if (*It != Me) O = *It; }
	StT += Dt; SubT += Dt; ShotT += Dt; LogT += Dt; Total += Dt;
	const bool bNeon = Me->CoopRole == ECoopRole::Neon;
	const float Sg = bNeon ? -1.f : 1.f;
	const FVector L = Me->GetActorLocation();
	const FBalState& B = M->State();
	const float P = M->PlatformOffset();
	auto Walk = [&](FVector2D Tg) -> float
	{
		const FVector2D D = Tg - FVector2D(L.X, L.Y);
		if (D.Size() > 22.f) { Me->AddMovementInput(FVector(D.X, D.Y, 0).GetSafeNormal(), 1.f); PC->SetControlRotation(FRotator(-12.f, FMath::RadiansToDegrees(FMath::Atan2(D.Y, D.X)), 0)); }
		return D.Size();
	};
	auto At = [&](FVector2D Tg) { return Walk(Tg) < 30.f; };
	auto Path = [&](std::initializer_list<FVector2D> Pt) -> bool { const int32 N = (int32)Pt.size(); if (Wp >= N) return true; if (Walk(*(Pt.begin() + Wp)) < 30.f) ++Wp; return Wp >= N; };
	auto Use = [&](const TCHAR* Why) { if (UFunction* F = Me->FindFunction(TEXT("ServerInteract"))) Me->ProcessEvent(F, nullptr); Log += FString::Printf(TEXT("ACAO t=%.2f tecla E (%s)\n"), Total, Why); };
	auto Fire = [&](ACoopDestructible* T, const TCHAR* Why)
	{
		const FVector D = T->GetActorLocation() - L; PC->SetControlRotation(FRotator(FMath::Clamp(D.Rotation().Pitch, -60.f, 60.f), D.Rotation().Yaw, 0));
		if (UFunction* F = Me->FindFunction(TEXT("ServerPowerShot"))) { struct { ACoopDestructible* Target; FVector_NetQuantize10 End; } Parm{ T, FVector_NetQuantize10(T->GetActorLocation()) }; Me->ProcessEvent(F, &Parm); }
		Log += FString::Printf(TEXT("ACAO t=%.2f %s atira em %s\n"), Total, bNeon ? TEXT("Neon") : TEXT("Ruina"), Why);
	};
	auto Res = [&](const FString& S) { Log += TEXT("RESULTADO ") + S + TEXT("\n"); Save(); };
	auto InCab = [&](const FVector& Pp) { return FMath::Abs(Pp.X - E->GetActorLocation().X) < 95.f && FMath::Abs(Pp.Y - E->GetActorLocation().Y) < 95.f && Pp.Z - E->GetActorLocation().Z > 0.f && Pp.Z - E->GetActorLocation().Z < 250.f; };
	auto GatesOpen = [&]() { int32 N = 0; for (TActorIterator<ACoopGate> It(W); It; ++It) if (It->GateId.ToString().StartsWith(TEXT("P2_")) && It->bOpen) ++N; return N; };
	auto Mach = [&]() { return FString::Printf(TEXT("P=%.0f alvo=%.0f massaN=%d massaS=%d freioLivre=%d travado=%d concluido=%d pilhas=%d lingotes=[%d,%d,%d] calha=%s pino=%s portoesP2abertos=%d rearmes=%d"),
		P, B.Target, M->MassNorth(), M->MassSouth(), (int32)B.bBrakeFree, (int32)B.bLocked, (int32)B.bDone, HasAuthority() ? M->MaterialPiles() : -1, B.L0, B.L1, B.L2,
		M->Chute->IsDestroyed() ? TEXT("ausente") : TEXT("CONSTRUIDA"), M->Pin->IsDestroyed() ? TEXT("ausente") : TEXT("ENCAIXADO"), GatesOpen(), B.Resets); };
	// camera fixa em cada sala durante a maquina (so para o video)
	static const bool bInsp = FParse::Param(FCommandLine::Get(), TEXT("InspP2"));
	if (bInsp && St >= 5 && St <= 9)
	{
		static ACameraActor* IC = nullptr;
		if (!IC) { IC = W->SpawnActor<ACameraActor>(); IC->GetCameraComponent()->SetFieldOfView(88.f); }
		const FVector CP(1000.f, Sg * 1190.f, 1120.f), CT(1560.f, Sg * 560.f, 860.f);
		IC->SetActorLocationAndRotation(CP, (CT - CP).Rotation());
		if (PC->GetViewTarget() != IC) PC->SetViewTargetWithBlend(IC, 0.f);
	}
	else if (bInsp && St > 9 && PC->GetViewTarget() != Me) PC->SetViewTargetWithBlend(Me, 0.f);
	if (St >= 1 && ShotT >= 0.2f) { ShotT = 0.f; FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/f2p2/%s/f_%05d.jpg"), *Tag, Shots++), false, false); }
	if (LogT >= 0.5f) { LogT = 0.f; Log += FString::Printf(TEXT("t=%.2f passo=%d.%d %s pos=(%.0f,%.0f,%.0f) %s\n"), Total, St, Sub, bNeon ? TEXT("Neon") : TEXT("Ruina"), L.X, L.Y, L.Z, *Mach()); }
	if (Total > 235.f && St < 98) { Res(TEXT("TEMPO TOTAL ESGOTADO")); Go(98, TEXT("tempo total")); }
	if (St > 0 && St < 90 && StT > 45.f) { Res(FString::Printf(TEXT("TIMEOUT no passo %d.%d pos=(%.0f,%.0f,%.0f) %s"), St, Sub, L.X, L.Y, L.Z, *Mach())); Go(98, TEXT("timeout")); }
	const FVector2D NeonSpotA(900, -255), RuinaSpotA(900, 255);
	const FVector2D RoomSpot(1300.f, Sg * 470.f), MachSpot(1450.f, Sg * 470.f), PinSpot(1050.f, -650.f), ResetSpot(1750.f, 450.f * Sg);
	switch (St)
	{
	case 0:
		if (NP < 2 || Total < 3.f) break;
		if (PC->GetViewTarget() != Me && Sub == 0 && SubT > 1.f) { Use(TEXT("pular apresentacao")); Sub = 1; }
		if (PC->GetViewTarget() == Me && StT > 4.f) { Res(FString::Printf(TEXT("INICIO papel=%s | maquina: %s"), bNeon ? TEXT("Neon") : TEXT("Ruina"), *Mach())); Go(1, TEXT("controle")); }
		break;
	// ---------------- P1 (rapido, caminho real) ----------------
	case 1: if (At(bNeon ? NeonSpotA : RuinaSpotA)) Go(2, TEXT("P1 posicionado")); break;
	case 2:
		if (!bNeon && Sub == 0 && StT > 1.f) { Fire(Tr, TEXT("TRAVA (P1)")); Sub = 1; }
		if (bNeon && Sub == 0 && Tr->IsDestroyed() && StT > 2.f) { Fire(Su, TEXT("SUPORTE (P1)")); Sub = 1; }
		if (!E->bLockedP1) { Res(TEXT("A. P1 resolvido (elevador liberado)")); Go(3, TEXT("P1 ok")); }
		break;
	case 3:
		if (At(FVector2D(1200, Sg * 45.f)) && O && InCab(L) && InCab(O->GetActorLocation()) && !bNeon && !E->IsMoving() && !E->IsTargetTop() && SubT > 2.f) { Use(TEXT("subir")); SubT = 0.f; }
		if (E->IsParkedAt(true) && E->SideDoorAlpha() > 0.95f) { Res(FString::Printf(TEXT("B. ELEVADOR levou os dois ate B (eu a bordo=%d, Z=%.0f)"), (int32)InCab(L), L.Z)); Go(4, TEXT("em B")); }
		break;
	case 4: if (Path({ FVector2D(1200, Sg * 40.f), FVector2D(1200, Sg * 470.f), RoomSpot })) { Res(FString::Printf(TEXT("C. SEPARACAO: atravessei a barreira %s, estou na sala %s (Y=%.0f)"), bNeon ? TEXT("ciano") : TEXT("roxa"), bNeon ? TEXT("SUL") : TEXT("NORTE"), L.Y)); Go(5, TEXT("na sala")); } break;
	// ---------------- P2 ----------------
	case 5:   // D: interacoes nao autorizadas / sem efeito
		if (bNeon)
		{
			if (Sub == 0 && At(MachSpot)) { Fire(M->Chute, TEXT("CALHA sem material")); Sub = 1; SubT = 0.f; }
			else if (Sub == 1 && SubT > 1.5f) { Res(FString::Printf(TEXT("D1. NEON reconstruir a calha SEM material: calha %s"), M->Chute->IsDestroyed() ? TEXT("continua ausente (OK)") : TEXT("CONSTRUIDA (FALHA)"))); Sub = 2; }
			else if (Sub == 2 && At(PinSpot)) { Fire(M->Pin, TEXT("PINO com a maquina emperrada")); Sub = 3; SubT = 0.f; }
			else if (Sub == 3 && SubT > 1.5f) { Res(FString::Printf(TEXT("D2. NEON pino com a maquina desalinhada: pino %s"), M->Pin->IsDestroyed() ? TEXT("nao encaixou (OK)") : TEXT("ENCAIXOU (FALHA)"))); Go(6, TEXT("Neon testou")); }
		}
		else
		{
			if (Sub == 0 && At(MachSpot)) { Fire(M->Chute, TEXT("CALHA da sala do Neon (atraves da parede/janela)")); Sub = 1; SubT = 0.f; }
			else if (Sub == 1 && SubT > 1.5f) { Res(FString::Printf(TEXT("D3. RUINA na peca do outro lado: calha %s"), M->Chute->IsDestroyed() ? TEXT("sem efeito (OK)") : TEXT("ALTERADA (FALHA)"))); Sub = 2; }
			else if (Sub == 2 && At(RoomSpot) && O && FVector::Dist2D(O->GetActorLocation(), FVector(PinSpot.X, PinSpot.Y, 0)) < 80.f && StT > 6.f) Go(6, TEXT("Ruina pronta"));
		}
		break;
	case 6:   // E/F: Ruina solta o freio -> plataformas se movem (Neon observa do outro lado)
		if (!bNeon && Sub == 0 && StT > 1.f) { Fire(M->Brake, TEXT("FREIO emperrado")); Sub = 1; }
		if (B.bBrakeFree && Sub < 2) { Res(FString::Printf(TEXT("%s. FREIO solto (visto por %s): alvo P=%.0f (norte mais pesada: 3 x 1)"), bNeon ? TEXT("F") : TEXT("E"), bNeon ? TEXT("Neon") : TEXT("Ruina"), B.Target)); Sub = 2; }
		if (Sub == 2 && FMath::Abs(P - B.Target) < 3.f) { Res(FString::Printf(TEXT("E. PLATAFORMAS pararam em P=%.0f (norte embaixo, sul em cima)"), P)); Go(7, TEXT("maquina livre")); }
		break;
	case 7:   // ERRO demonstrado (parte 1): bracadeira antes da calha -> lingote fica no berco; Neon monta a calha -> passa para o sul
		if (!bNeon && Sub == 0 && At(MachSpot)) { Fire(M->Clamp0, TEXT("BRACADEIRA 1")); Sub = 1; }
		if (B.L0 == 1 && Sub < 2) { Res(FString::Printf(TEXT("E2. lingote 1 no BERCO (calha ausente): massas %d x %d, alvo P=%.0f"), M->MassNorth(), M->MassSouth(), B.Target)); Sub = 2; }
		if (bNeon && Sub == 2 && At(MachSpot)) { Fire(M->Chute, TEXT("CALHA com o material do freio")); Sub = 3; }
		if (B.L0 == 2 && Sub >= 2 && Sub < 4) { Res(FString::Printf(TEXT("G. CALHA construida -> lingote passou ao SUL: massas %d x %d, alvo P=%.0f"), M->MassNorth(), M->MassSouth(), B.Target)); Sub = 4; }
		if (Sub == 4 && FMath::Abs(P) < 3.f) { Res(FString::Printf(TEXT("H1. PLATAFORMAS NIVELADAS (P=%.1f) - mas aqui a Ruina vai cometer um erro"), P)); Go(8, TEXT("nivelado")); }
		break;
	case 8:   // ERRO (parte 2): Ruina solta mais uma -> excesso no sul; Neon tenta o pino (nao encaixa); Ruina rearma
		if (!bNeon && Sub == 0 && StT > 1.f) { Fire(M->Clamp1, TEXT("BRACADEIRA 2 (erro: transferencia a mais)")); Sub = 1; }
		if (Sub < 2 && B.L1 == 2) { Res(FString::Printf(TEXT("ERRO: lingote 2 tambem passou: massas %d x %d, alvo P=%.0f (sul agora mais pesada)"), M->MassNorth(), M->MassSouth(), B.Target)); Sub = 2; }
		if (bNeon && Sub == 2 && P > 150.f && At(PinSpot)) { Fire(M->Pin, TEXT("PINO desalinhado")); Sub = 3; SubT = 0.f; }
		if (bNeon && Sub == 3 && SubT > 1.5f) { Res(FString::Printf(TEXT("H2. PINO com a maquina desequilibrada: %s"), M->Pin->IsDestroyed() ? TEXT("nao encaixou (OK)") : TEXT("ENCAIXOU (FALHA)"))); Sub = 4; }
		if (!bNeon && Sub == 2 && P > 150.f && StT > 6.f && At(ResetSpot)) { Use(TEXT("rearmar mecanismo")); Sub = 3; }
		if (B.Resets >= 1 && Sub >= 2) { Res(FString::Printf(TEXT("K. REARME: %s"), *Mach())); Go(9, TEXT("rearmado")); }
		break;
	case 9:   // SOLUCAO: freio -> calha -> UMA bracadeira -> nivel -> pino
		if (!bNeon)
		{
			if (Sub == 0 && At(RoomSpot) && StT > 2.f) { Fire(M->Brake, TEXT("FREIO")); Sub = 1; }
			else if (Sub == 1 && !M->Chute->IsDestroyed() && At(MachSpot)) { Fire(M->Clamp0, TEXT("BRACADEIRA 1 (unica)")); Sub = 2; }
		}
		else
		{
			if (Sub == 0 && B.bBrakeFree && At(MachSpot)) { Fire(M->Chute, TEXT("CALHA")); Sub = 1; }
			else if (Sub == 1 && !M->Chute->IsDestroyed() && At(PinSpot)) { Sub = 2; }
			else if (Sub == 2 && B.L0 == 2 && FMath::Abs(P) < 2.f) { Fire(M->Pin, TEXT("PINO no nivel")); Sub = 3; }
		}
		if (B.bDone && GatesOpen() == 2) { Res(FString::Printf(TEXT("I. CONCLUIDO: equilibrio travado e portoes abertos | %s"), *Mach())); Go(10, TEXT("concluido")); }
		else if (StT > 2.f && FMath::Fmod(StT, 5.f) < Dt && GatesOpen() > 0) Res(TEXT("FALHA: portao aberto antes da conclusao"));
		break;
	case 10: if (Path({ FVector2D(1850, Sg * 470.f), FVector2D(1850, Sg * 750.f), FVector2D(2150, Sg * 750.f), FVector2D(2150, Sg * 300.f) }) && O && FVector::Dist2D(L, O->GetActorLocation()) < 700.f)
		{ Res(FString::Printf(TEXT("J. REENCONTRO: distancia %.0f cm | caminho ate a escada B->C livre"), FVector::Dist2D(L, O->GetActorLocation()))); Go(11, TEXT("reencontro")); } break;
	case 11: if (StT > 3.f) { Res(FString::Printf(TEXT("FIM P2 | %s"), *Mach())); Go(99, TEXT("fim")); } break;
	case 98: case 99: if (StT > (Tag == TEXT("host") ? 8.f : 4.f)) { Save(); bDone = true; GEngine->Exec(W, TEXT("quit")); } break;
	}
}
