#include "CoopF2Test.h"
#include "CoopCharacter.h"
#include "CoopElevator.h"
#include "CoopGate.h"
#include "CoopFase2GameMode.h"
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
	const int32 NV = UE_ARRAY_COUNT(V);
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
			It->SetActorHiddenInGame((O.Z - Ext.Z) > V[I].HideAbove || (V[I].bCut && It->ActorHasTag(TEXT("CorteSul"))));
		}
		Cam->SetActorLocationAndRotation(V[I].P, (V[I].Tg - V[I].P).Rotation());
		Cam->GetCameraComponent()->SetFieldOfView(V[I].Fov);
		PC->SetViewTargetWithBlend(Cam, 0.f);
	}
	if (Ph > 0.9f && Ph < 0.9f + Dt * 1.5f) FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/f2vistas/%s.png"), V[I].Name), false, false);
}
