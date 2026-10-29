#include "CoopTestSubsystem.h"
#include "CoopCharacter.h"
#include "CoopDestructible.h"
#include "Camera/CameraActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"

void UCoopTestSubsystem::OnWorldBeginPlay(UWorld& W)
{
	Super::OnWorldBeginPlay(W);
	if (!W.IsGameWorld()) return;
	if (FParse::Param(FCommandLine::Get(), TEXT("F1Tour"))) W.SpawnActor<ACoopF1Tour>();
	if (FParse::Param(FCommandLine::Get(), TEXT("F1Fluxo"))) W.SpawnActor<ACoopF1Flow>();
}

// ---------------- passeio de camera (mesmo trajeto em qualquer mapa da Fase 1) ----------------
struct FTourKey { FVector P; float Yaw, Pitch; };
static const FTourKey GTour[] = {
	{ FVector(-1950, 0, 260), 0, -8 }, { FVector(-1100, -250, 230), 12, -6 }, { FVector(-500, 250, 250), -35, -10 },
	{ FVector(-50, 0, 330), -5, -22 }, { FVector(650, 0, 300), 0, -18 }, { FVector(1400, 0, 250), 5, -8 },
	{ FVector(2400, -250, 220), 15, -5 }, { FVector(3150, 0, 190), 0, -2 }, { FVector(3420, 0, 170), 0, 0 } };

// trajeto para o layout 2 (Fase1_Arquitetura, coordenadas da UE)
static const FTourKey GTour2[] = {
	{ FVector(0, 350, 420), -15, -10 }, { FVector(850, 450, 520), -55, -14 }, { FVector(1150, 0, 700), 0, -32 },
	{ FVector(2000, 0, 950), 0, -42 }, { FVector(2900, 650, 520), -12, -10 }, { FVector(3400, 0, 470), 0, -5 },
	{ FVector(3900, 0, 540), 0, -3 }, { FVector(3650, 900, 700), 195, -12 }, { FVector(2700, -950, 650), 168, -9 } };

// trajeto "estrutura" (coluna + o que ela sustenta): posicao -> alvo
struct FLookKey { FVector P, Tg; };
static const FLookKey GTourEst[] = {
	{ FVector(2700, 800, 700), FVector(900, -700, 800) }, { FVector(1900, -100, 520), FVector(1000, -600, 520) },
	{ FVector(1380, -330, 260), FVector(1000, -600, 640) }, { FVector(1950, -1150, 1050), FVector(1000, -650, 1150) },
	{ FVector(2300, 300, 1150), FVector(900, -800, 1200) }, { FVector(-100, 450, 650), FVector(900, -800, 900) } };

void ACoopF1Tour::Tick(float Dt)
{
	Super::Tick(Dt);
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn()) return;
	FParse::Value(FCommandLine::Get(), TEXT("TourTag="), Tag);
	if (Tag.IsEmpty()) Tag = TEXT("tour");
	T += Dt;
	if (T < 4.f) return;   // carrega texturas/luz
	if (!bStarted) { Cam = GetWorld()->SpawnActor<ACameraActor>(FVector::ZeroVector, FRotator::ZeroRotator); Cam->GetCameraComponent()->SetFieldOfView(85.f); PC->SetViewTargetWithBlend(Cam, 0.f); bStarted = true; return; }
	bool bL2 = false; for (TActorIterator<AActor> It(GetWorld()); It; ++It) if (It->Tags.Contains(FName("F1L_Ponte"))) { bL2 = true; break; }
	const FTourKey* Keys = bL2 ? GTour2 : GTour;
	const int32 NK = bL2 ? UE_ARRAY_COUNT(GTour2) : UE_ARRAY_COUNT(GTour); const float Dur = bL2 ? 22.f : 18.f;
	const float U = FMath::Clamp((T - 4.f) / Dur, 0.f, 1.f) * (NK - 1);
	const int32 I = FMath::Min((int32)U, NK - 2); const float A = U - I;
	auto K = [&](int32 i) { return Keys[FMath::Clamp(i, 0, NK - 1)]; };
	if (Tag == TEXT("estrutura"))
	{
		const int32 NE = UE_ARRAY_COUNT(GTourEst); const float DE = 20.f;
		const float UE_ = FMath::Clamp((T - 4.f) / DE, 0.f, 1.f) * (NE - 1); const int32 J = FMath::Min((int32)UE_, NE - 2); const float B = UE_ - J;
		auto E = [&](int32 i) { return GTourEst[FMath::Clamp(i, 0, NE - 1)]; };
		const FVector Pe = FMath::CubicCRSplineInterp(E(J - 1).P, E(J).P, E(J + 1).P, E(J + 2).P, 0.f, 1.f, 2.f, 3.f, 1.f + B);
		const FVector Te = FMath::CubicCRSplineInterp(E(J - 1).Tg, E(J).Tg, E(J + 1).Tg, E(J + 2).Tg, 0.f, 1.f, 2.f, 3.f, 1.f + B);
		Cam->SetActorLocationAndRotation(Pe, FRotationMatrix::MakeFromX(Te - Pe).Rotator());
		FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/%s/f_%05d.jpg"), *Tag, Frame++), false, false);
		if (T > 4.f + DE + 0.2f) GEngine->Exec(GetWorld(), TEXT("quit"));
		return;
	}
	const FVector P = FMath::CubicCRSplineInterp(K(I - 1).P, K(I).P, K(I + 1).P, K(I + 2).P, 0.f, 1.f, 2.f, 3.f, 1.f + A);
	const float S = A * A * (3 - 2 * A);
	Cam->SetActorLocationAndRotation(P, FRotator(FMath::Lerp(K(I).Pitch, K(I + 1).Pitch, S), FMath::Lerp(K(I).Yaw, K(I + 1).Yaw, S), 0));
	FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/%s/f_%05d.jpg"), *Tag, Frame++), false, false);
	if (T > 4.f + Dur + 0.2f) GEngine->Exec(GetWorld(), TEXT("quit"));
}

// ---------------- roteiro do puzzle (host + cliente, cada um controla o proprio personagem) ----------------
void ACoopF1Flow::SetPh(int32 P, const FString& Why)
{
	const AGameStateBase* GS = GetWorld()->GetGameState();
	Log += FString::Printf(TEXT("EVT t=%.2f srvT=%.2f fase %d->%d %s\n"), Total, GS ? GS->GetServerWorldTimeSeconds() : 0.f, Ph, P, *Why);
	Ph = P; PhT = 0.f;
}
void ACoopF1Flow::Save() { FFileHelper::SaveStringToFile(Log, *(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/f1fluxo/log_%s.txt"), *Tag))); }

void ACoopF1Flow::Fire(ACoopCharacter* Me, ACoopDestructible* T)
{
	// caminho REAL do jogo: RPC ServerPowerShot (o servidor valida papel, alcance, linha de visao e estado)
	if (UFunction* F = Me->FindFunction(TEXT("ServerPowerShot")))
	{
		struct { ACoopDestructible* Target; FVector_NetQuantize10 End; } Parm{ T, FVector_NetQuantize10(T->GetActorLocation()) };
		Me->ProcessEvent(F, &Parm);
	}
}

void ACoopF1Flow::Tick(float Dt)
{
	Super::Tick(Dt);
	UWorld* W = GetWorld();
	APlayerController* PC = W->GetFirstPlayerController();
	ACoopCharacter* Me = PC ? Cast<ACoopCharacter>(PC->GetPawn()) : nullptr;
	AGameStateBase* GS = W->GetGameState();
	if (Tag.IsEmpty()) { FParse::Value(FCommandLine::Get(), TEXT("FluxoTag="), Tag); if (Tag.IsEmpty()) Tag = TEXT("solo"); }
	if (!Me || !GS) return;
	PhT += Dt; ShotT += Dt; SaveT += Dt; Total += Dt;
	ACoopDestructible *Col = nullptr, *Br = nullptr;
	for (TActorIterator<ACoopDestructible> It(W); It; ++It) { if (It->Model == FName("Coluna_Inteira")) Col = *It; else if (It->Model == FName("Ponte_Inteira")) Br = *It; }
	if (!Col || !Br) return;
	// pontos do roteiro (padrao = layout original; com marcadores F1L_* = nova arquitetura)
	float EdgeX = -130.f, AcrossX = 1750.f, BeyondX = 1600.f, DoorApX = 3300.f, DoorPassX = 3760.f, DoorY = 0.f, TunnelX = 5150.f, ExitX = 5000.f, PassX = 3720.f;
	{
		AActor *MBr = nullptr, *MDoor = nullptr;
		for (TActorIterator<AActor> It(W); It; ++It) { if (It->Tags.Contains(FName("F1L_Ponte"))) MBr = *It; if (It->Tags.Contains(FName("F1L_Porta"))) MDoor = *It; }
		if (MBr && MDoor)
		{
			const float BX = MBr->GetActorLocation().X, HL = MBr->GetActorScale3D().X * 50.f, DX = MDoor->GetActorLocation().X;
			EdgeX = BX - HL - 130.f; AcrossX = BX + HL + 250.f; BeyondX = BX + HL - 150.f;
			DoorApX = DX - 380.f; DoorPassX = DX + 184.f; PassX = DX + 144.f; DoorY = MDoor->GetActorLocation().Y; TunnelX = DX + 1574.f; ExitX = DX + 1424.f;
		}
	}
	const bool bNeon = Me->CoopRole == ECoopRole::Neon;
	const float Oy = bNeon ? 100.f : -100.f;
	int32 NP = 0, NBeyond = 0, NExit = 0;
	for (TActorIterator<ACoopCharacter> It(W); It; ++It) { ++NP; if (It->GetActorLocation().X > BeyondX) ++NBeyond; if (It->GetActorLocation().X > ExitX) ++NExit; }
	auto Walk = [&](FVector2D Tg)->float
	{
		const FVector L = Me->GetActorLocation(); const FVector2D D = Tg - FVector2D(L.X, L.Y);
		if (D.Size() > 25.f) { Me->AddMovementInput(FVector(D.X, D.Y, 0).GetSafeNormal(), 1.f); PC->SetControlRotation(FRotator(-10.f, FMath::RadiansToDegrees(FMath::Atan2(D.Y, D.X)), 0)); }
		return D.Size();
	};
	static const bool bInsp = FParse::Param(FCommandLine::Get(), TEXT("InspCam"));
	if (bInsp && Tag == TEXT("host") && Ph >= 1)
	{
		static ACameraActor* IC = nullptr;
		if (!IC) { const FVector CP(2150, 450, 640), CT(850, -750, 520); IC = W->SpawnActor<ACameraActor>(CP, FRotationMatrix::MakeFromX(CT - CP).Rotator()); IC->GetCameraComponent()->SetFieldOfView(72.f); }
		if (PC->GetViewTarget() != IC) PC->SetViewTargetWithBlend(IC, 0.f);
	}
	if (Ph >= 1 && ShotT >= ((bInsp && Tag == TEXT("host")) ? 0.1f : 0.25f)) { ShotT = 0.f; FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/f1fluxo/%s/f_%05d.jpg"), *Tag, Shots++), false, false); }
	if (Ph >= 1 && SaveT > 0.5f)
	{
		SaveT = 0.f; const FVector L = Me->GetActorLocation();
		Log += FString::Printf(TEXT("t=%.2f ph=%d %s pos=(%.0f,%.0f,%.0f) coluna=%s ponte=%s\n"), Total, Ph, bNeon ? TEXT("Neon") : TEXT("Ruina"), L.X, L.Y, L.Z,
			Col->IsDestroyed() ? TEXT("DESTRUIDA") : TEXT("inteira"), Br->IsDestroyed() ? TEXT("DESTRUIDA") : TEXT("inteira"));
		Save();
	}
	// ---- variantes: -F1Teste=colapso (coluna nao e reconstruida -> teto desaba e a fase reinicia) | atalho (tentativas de pulo pelo vao) ----
	const FString Var = FParse::Param(FCommandLine::Get(), TEXT("F1Colapso")) ? TEXT("colapso") : FParse::Param(FCommandLine::Get(), TEXT("F1Atalho")) ? TEXT("atalho") : FParse::Param(FCommandLine::Get(), TEXT("F1Salvar")) ? TEXT("salvar") : TEXT("");
	if (Var == TEXT("salvar") && Ph >= 2 && Ph < 9)
	{
		// 2 ciclos: Ruina destroi; Neon reconstroi aos 8,0 s (ciclo 1) e aos 11,6 s (ciclo 2, perto do limite de 12 s)
		static int32 Cyc = 0; static float TDown = -1.f; static bool bAnyCrush = false;
		for (TActorIterator<ACoopCharacter> It(W); It; ++It) if (It->bCrushed) bAnyCrush = true;
		const float Lim[2] = { 8.0f, 11.6f };
		if (Ph == 2) { if (Walk(FVector2D(620.f, -700.f + Oy)) < 25.f && PhT > 3.f) SetPh(3, TEXT("sob a estrutura")); return; }
		if (Ph == 3)
		{
			if (!bNeon && !Col->IsDestroyed() && PhT > 1.f && PhT < 1.f + Dt * 1.5f) Fire(Me, Col);
			if (Col->IsDestroyed() && TDown < 0.f) { TDown = Total; Log += FString::Printf(TEXT("EVT ciclo %d: coluna DESTRUIDA t=%.2f\n"), Cyc + 1, Total); }
			if (TDown >= 0.f && bNeon && Col->IsDestroyed() && Total - TDown >= Lim[Cyc]) Fire(Me, Col);
			if (TDown >= 0.f && !Col->IsDestroyed())
			{
				Log += FString::Printf(TEXT("RESULTADO ciclo %d: coluna RECONSTRUIDA %.2f s apos a quebra (limite 12) | esmagado=%d\n"), Cyc + 1, Total - TDown, (int32)bAnyCrush);
				TDown = -1.f; ++Cyc; SetPh(Cyc >= 2 ? 4 : 3, TEXT("estabilizado")); Save();
			}
			return;
		}
		if (Ph == 4 && PhT > 8.f) { Log += FString::Printf(TEXT("RESULTADO SALVAMENTO: queda/esmagamento depois da reconstrucao = %s\n"), bAnyCrush ? TEXT("SIM (FALHA)") : TEXT("NAO (OK)")); Save(); SetPh(11, TEXT("fim salvar")); }
		return;
	}
	if (Var == TEXT("colapso") && Ph >= 2 && Ph < 9)
	{
		static FVector StartPos = FVector::ZeroVector;
		if (Ph == 2 && PhT < Dt + 0.001f) StartPos = Me->GetActorLocation();
		if (Ph == 2 && !bNeon && !Col->IsDestroyed() && PhT > 2.f && PhT < 2.f + Dt * 1.5f) Fire(Me, Col);
		if (Ph == 2 && Col->IsDestroyed()) { SetPh(3, TEXT("coluna DESTRUIDA; ninguem reconstroi")); }
		if (Ph == 3) Walk(FVector2D(620.f, -700.f + Oy));   // fica sob a estrutura sustentada pela coluna
		if (Ph == 3 && PhT > 18.f)
		{
			const bool bReset = !Col->IsDestroyed() && Br->IsDestroyed();
			Log += FString::Printf(TEXT("RESULTADO COLAPSO: coluna=%s ponte=%s pos=(%.0f,%.0f,%.0f) -> %s\n"), Col->IsDestroyed() ? TEXT("destruida") : TEXT("inteira(reiniciada)"),
				Br->IsDestroyed() ? TEXT("destruida(reiniciada)") : TEXT("inteira"), Me->GetActorLocation().X, Me->GetActorLocation().Y, Me->GetActorLocation().Z, bReset ? TEXT("SUCESSO") : TEXT("FALHA"));
			Save(); SetPh(11, TEXT("fim colapso"));
		}
		return;
	}
	if (Var == TEXT("atalho") && Ph >= 2 && Ph < 9)
	{
		// so o host tenta: corre e pula da borda em 3 direcoes (centro do vao, pier norte, diagonal); o cliente espera
		static int32 Try = 0; static float MaxX = -1e9f;
		if (!bNeon) { if (PhT > 40.f) SetPh(11, TEXT("cliente aguardou")); return; }
		const FVector L = Me->GetActorLocation();
		const float TY[3] = { 0.f, -825.f, -500.f };   // pier fica em Y -700..-950 (UE)
		if (Try >= 3) { Log += FString::Printf(TEXT("RESULTADO ATALHO: maior X alcancado acima do piso = %.0f (lado de saida comeca em %.0f) -> %s\n"), MaxX, AcrossX - 250.f, MaxX < AcrossX - 450.f ? TEXT("SEM ATALHO") : TEXT("ATALHO POSSIVEL")); Save(); SetPh(11, TEXT("fim atalho")); return; }
		if (L.Z > -80.f && L.X > MaxX && PhT > 0.5f) MaxX = L.X;
		if (PhT < 3.f) { Me->SetActorLocation(FVector(EdgeX - 700.f, TY[Try], 100.f)); return; }       // recua para pegar embalo
		Walk(FVector2D(EdgeX + 3000.f, TY[Try]));
		if (L.X > EdgeX + 90.f && !Me->GetCharacterMovement()->IsFalling()) Me->Jump();
		if (PhT > 9.f) { Log += FString::Printf(TEXT("tentativa %d (Y=%.0f): maxX=%.0f\n"), Try + 1, TY[Try], MaxX); ++Try; PhT = 0.f; }
		return;
	}
	switch (Ph)
	{
	case 0: if (GS->PlayerArray.Num() >= 2 && PhT > 6.f) SetPh(1, TEXT("2 jogadores")); break;
	case 1: if (Walk(FVector2D(EdgeX, DoorY + Oy)) < 25.f && PhT > 1.f && NP >= 2) SetPh(2, TEXT("na borda da plataforma inicial")); else if (PhT > 30.f) SetPh(9, TEXT("TIMEOUT borda")); break;
	case 2: // Ruina destroi a coluna (aguarda o parceiro chegar)
		if (!bNeon && !Col->IsDestroyed() && PhT > 2.f && PhT < 2.f + Dt * 1.5f) Fire(Me, Col);
		if (bNeon && Col->IsDestroyed() && Br->IsDestroyed()) { Fire(Me, Br); SetPh(3, TEXT("Neon: coluna caiu -> constroi a ponte")); }
		else if (!bNeon && Col->IsDestroyed()) SetPh(3, TEXT("Ruina: coluna DESTRUIDA"));
		else if (PhT > 15.f) SetPh(9, TEXT("TIMEOUT coluna"));
		break;
	case 3: if (!Br->IsDestroyed()) SetPh(4, TEXT("ponte CONSTRUIDA -> atravessar")); else if (PhT > 10.f) SetPh(9, TEXT("TIMEOUT ponte")); break;
	case 4: // atravessa a ponte
		if (Walk(FVector2D(AcrossX, DoorY + Oy)) < 25.f) SetPh(5, TEXT("do outro lado"));
		else if (PhT > 15.f) SetPh(9, FString::Printf(TEXT("TIMEOUT travessia (X=%.0f)"), Me->GetActorLocation().X));
		break;
	case 5: // Ruina destroi a ponte; Neon reconstroi a coluna
		if (!bNeon && NBeyond >= 2 && !Br->IsDestroyed() && PhT > 0.3f) { Fire(Me, Br); SetPh(6, TEXT("Ruina: destroi a ponte")); }
		if (bNeon && Br->IsDestroyed() && Col->IsDestroyed()) { Fire(Me, Col); SetPh(6, TEXT("Neon: ponte caiu -> reconstroi a coluna")); }
		else if (PhT > 12.f) SetPh(9, TEXT("TIMEOUT etapa ponte->coluna"));
		break;
	case 6: if (!Col->IsDestroyed() && Br->IsDestroyed()) SetPh(7, TEXT("coluna RECONSTRUIDA (teto estabilizado)")); else if (PhT > 8.f) SetPh(9, TEXT("TIMEOUT coluna nao reconstruida")); break;
	case 7: if (Walk(FVector2D(DoorApX, DoorY + Oy)) < 25.f) SetPh(8, TEXT("diante da porta")); else if (PhT > 20.f) SetPh(9, TEXT("TIMEOUT porta")); break;
	case 8: // a abertura da porta e estreita (Y -69..57): passa um de cada vez pelo centro, depois se afasta para o lado no tunel
		if (PhT > (bNeon ? 2.f : 3.5f))
		{
			const FVector2D Tg = Me->GetActorLocation().X < PassX ? FVector2D(DoorPassX, DoorY - 6.f) : FVector2D(TunnelX, DoorY + Oy * 1.5f);
			if (Me->GetActorLocation().X > PassX && Walk(Tg) < 25.f) SetPh(10, TEXT("fim do tunel"));
			else if (Me->GetActorLocation().X <= PassX) Walk(Tg);
			if (PhT > 25.f) SetPh(9, FString::Printf(TEXT("TIMEOUT tunel (X=%.0f Y=%.0f)"), Me->GetActorLocation().X, Me->GetActorLocation().Y));
		}
		break;
	case 10: if (NExit >= 2 || PhT > 6.f) { Log += FString::Printf(TEXT("RESULTADO: SUCESSO (jogadores no fim do tunel: %d de %d)\n"), NExit, NP); Save(); SetPh(11, TEXT("fim")); } break;
	case 11: if (PhT > (Tag == TEXT("host") ? 10.f : 4.f)) { Save(); GEngine->Exec(W, TEXT("quit")); Ph = 99; } break;
	case 9: if (PhT < Dt + 0.001f) { Log += TEXT("RESULTADO: FALHA\n"); Save(); } if (PhT > 5.f) { GEngine->Exec(W, TEXT("quit")); Ph = 99; } break;
	}
}
