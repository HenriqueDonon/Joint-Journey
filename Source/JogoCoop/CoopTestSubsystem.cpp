#include "CoopTestSubsystem.h"
#include "CoopCharacter.h"
#include "CoopDestructible.h"
#include "Camera/CameraActor.h"
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

void ACoopF1Tour::Tick(float Dt)
{
	Super::Tick(Dt);
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn()) return;
	FParse::Value(FCommandLine::Get(), TEXT("TourTag="), Tag);
	if (Tag.IsEmpty()) Tag = TEXT("tour");
	T += Dt;
	if (T < 4.f) return;   // carrega texturas/luz
	if (!bStarted) { Cam = GetWorld()->SpawnActor<ACameraActor>(GTour[0].P, FRotator(GTour[0].Pitch, GTour[0].Yaw, 0)); Cam->GetCameraComponent()->SetFieldOfView(85.f); PC->SetViewTargetWithBlend(Cam, 0.f); bStarted = true; return; }
	const int32 NK = UE_ARRAY_COUNT(GTour); const float Dur = 18.f;
	const float U = FMath::Clamp((T - 4.f) / Dur, 0.f, 1.f) * (NK - 1);
	const int32 I = FMath::Min((int32)U, NK - 2); const float A = U - I;
	auto K = [&](int32 i) { return GTour[FMath::Clamp(i, 0, NK - 1)]; };
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
	const bool bNeon = Me->CoopRole == ECoopRole::Neon;
	const float Oy = bNeon ? 100.f : -100.f;
	int32 NP = 0, NBeyond = 0, NExit = 0;
	for (TActorIterator<ACoopCharacter> It(W); It; ++It) { ++NP; if (It->GetActorLocation().X > 1600.f) ++NBeyond; if (It->GetActorLocation().X > 5000.f) ++NExit; }
	auto Walk = [&](FVector2D Tg)->float
	{
		const FVector L = Me->GetActorLocation(); const FVector2D D = Tg - FVector2D(L.X, L.Y);
		if (D.Size() > 25.f) { Me->AddMovementInput(FVector(D.X, D.Y, 0).GetSafeNormal(), 1.f); PC->SetControlRotation(FRotator(-10.f, FMath::RadiansToDegrees(FMath::Atan2(D.Y, D.X)), 0)); }
		return D.Size();
	};
	if (Ph >= 1 && ShotT >= 0.25f) { ShotT = 0.f; FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/f1fluxo/%s/f_%05d.jpg"), *Tag, Shots++), false, false); }
	if (Ph >= 1 && SaveT > 0.5f)
	{
		SaveT = 0.f; const FVector L = Me->GetActorLocation();
		Log += FString::Printf(TEXT("t=%.2f ph=%d %s pos=(%.0f,%.0f,%.0f) coluna=%s ponte=%s\n"), Total, Ph, bNeon ? TEXT("Neon") : TEXT("Ruina"), L.X, L.Y, L.Z,
			Col->IsDestroyed() ? TEXT("DESTRUIDA") : TEXT("inteira"), Br->IsDestroyed() ? TEXT("DESTRUIDA") : TEXT("inteira"));
		Save();
	}
	switch (Ph)
	{
	case 0: if (GS->PlayerArray.Num() >= 2 && PhT > 6.f) SetPh(1, TEXT("2 jogadores")); break;
	case 1: if (Walk(FVector2D(-130.f, Oy)) < 25.f && PhT > 1.f && NP >= 2) SetPh(2, TEXT("na borda da plataforma inicial")); else if (PhT > 30.f) SetPh(9, TEXT("TIMEOUT borda")); break;
	case 2: // Ruina destroi a coluna (aguarda o parceiro chegar)
		if (!bNeon && !Col->IsDestroyed() && PhT > 2.f && PhT < 2.f + Dt * 1.5f) Fire(Me, Col);
		if (bNeon && Col->IsDestroyed() && Br->IsDestroyed()) { Fire(Me, Br); SetPh(3, TEXT("Neon: coluna caiu -> constroi a ponte")); }
		else if (!bNeon && Col->IsDestroyed()) SetPh(3, TEXT("Ruina: coluna DESTRUIDA"));
		else if (PhT > 15.f) SetPh(9, TEXT("TIMEOUT coluna"));
		break;
	case 3: if (!Br->IsDestroyed()) SetPh(4, TEXT("ponte CONSTRUIDA -> atravessar")); else if (PhT > 10.f) SetPh(9, TEXT("TIMEOUT ponte")); break;
	case 4: // atravessa a ponte
		if (Walk(FVector2D(1750.f, Oy)) < 25.f) SetPh(5, TEXT("do outro lado"));
		else if (PhT > 15.f) SetPh(9, FString::Printf(TEXT("TIMEOUT travessia (X=%.0f)"), Me->GetActorLocation().X));
		break;
	case 5: // Ruina destroi a ponte; Neon reconstroi a coluna
		if (!bNeon && NBeyond >= 2 && !Br->IsDestroyed() && PhT > 0.3f) { Fire(Me, Br); SetPh(6, TEXT("Ruina: destroi a ponte")); }
		if (bNeon && Br->IsDestroyed() && Col->IsDestroyed()) { Fire(Me, Col); SetPh(6, TEXT("Neon: ponte caiu -> reconstroi a coluna")); }
		else if (PhT > 12.f) SetPh(9, TEXT("TIMEOUT etapa ponte->coluna"));
		break;
	case 6: if (!Col->IsDestroyed() && Br->IsDestroyed()) SetPh(7, TEXT("coluna RECONSTRUIDA (teto estabilizado)")); else if (PhT > 8.f) SetPh(9, TEXT("TIMEOUT coluna nao reconstruida")); break;
	case 7: if (Walk(FVector2D(3300.f, Oy)) < 25.f) SetPh(8, TEXT("diante da porta")); else if (PhT > 20.f) SetPh(9, TEXT("TIMEOUT porta")); break;
	case 8: // a abertura da porta e estreita (Y -69..57): passa um de cada vez pelo centro, depois se afasta para o lado no tunel
		if (PhT > (bNeon ? 2.f : 3.5f))
		{
			const FVector2D Tg = Me->GetActorLocation().X < 3720.f ? FVector2D(3760.f, -6.f) : FVector2D(5150.f, Oy * 1.5f);
			if (Me->GetActorLocation().X > 3720.f && Walk(Tg) < 25.f) SetPh(10, TEXT("fim do tunel"));
			else if (Me->GetActorLocation().X <= 3720.f) Walk(Tg);
			if (PhT > 25.f) SetPh(9, FString::Printf(TEXT("TIMEOUT tunel (X=%.0f Y=%.0f)"), Me->GetActorLocation().X, Me->GetActorLocation().Y));
		}
		break;
	case 10: if (NExit >= 2 || PhT > 6.f) { Log += FString::Printf(TEXT("RESULTADO: SUCESSO (jogadores no fim do tunel: %d de %d)\n"), NExit, NP); Save(); SetPh(11, TEXT("fim")); } break;
	case 11: if (PhT > (Tag == TEXT("host") ? 10.f : 4.f)) { Save(); GEngine->Exec(W, TEXT("quit")); Ph = 99; } break;
	case 9: if (PhT < Dt + 0.001f) { Log += TEXT("RESULTADO: FALHA\n"); Save(); } if (PhT > 5.f) { GEngine->Exec(W, TEXT("quit")); Ph = 99; } break;
	}
}
