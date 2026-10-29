#include "CoopElevTest.h"
#include "CoopElevator.h"
#include "CoopCharacter.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"

void ACoopElevTest::BeginPlay()
{
	Super::BeginPlay();
	bAuto = FParse::Param(FCommandLine::Get(), TEXT("ElevAuto"));
	FParse::Value(FCommandLine::Get(), TEXT("ElevTag="), Tag);
	int32 Lag = 0;
	if (bAuto && FParse::Value(FCommandLine::Get(), TEXT("ElevLag="), Lag) && Lag > 0 && GEngine)
		GEngine->Exec(GetWorld(), *FString::Printf(TEXT("Net PktLag=%d"), Lag));   // latencia artificial (Development)
}

void ACoopElevTest::SetPh(int32 P, const TCHAR* Why)
{
	const AGameStateBase* GS = GetWorld()->GetGameState();
	Log += FString::Printf(TEXT("EVT srvT=%.3f fase %d->%d (%s) ciclos=%d\n"), GS ? GS->GetServerWorldTimeSeconds() : 0.f, Ph, P, Why, Cycles);
	Ph = P; PhT = 0.f;
}

void ACoopElevTest::Save()
{
	FFileHelper::SaveStringToFile(Log, *(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/elev/log_%s.txt"), *Tag)));
}

void ACoopElevTest::Tick(float Dt)
{
	Super::Tick(Dt);
	if (!bAuto) return;
	UWorld* W = GetWorld();
	APlayerController* PC = W->GetFirstPlayerController();
	ACoopCharacter* Me = PC ? Cast<ACoopCharacter>(PC->GetPawn()) : nullptr;
	if (!Elev.IsValid()) for (TActorIterator<ACoopElevator> It(W); It; ++It) { Elev = *It; break; }
	AGameStateBase* GS = W->GetGameState();
	if (!Me || !Elev.IsValid() || !GS) return;
	PhT += Dt; LogT += Dt; ShotT += Dt; SaveT += Dt;
	ACoopElevator* E = Elev.Get();
	const float Srv = GS->GetServerWorldTimeSeconds();
	const bool bHost = Tag == TEXT("host");
	const FVector Home = E->GetHome();
	const FVector2D Off(0.f, bHost ? -60.f : 60.f);

	auto Walk = [&](FVector2D Target)->float
	{
		const FVector2D L(Me->GetActorLocation().X, Me->GetActorLocation().Y);
		const FVector2D D = Target - L;
		if (D.Size() > 20.f) Me->AddMovementInput(FVector(D.X, D.Y, 0.f).GetSafeNormal(), 1.f);
		return D.Size();
	};

	// ---- amostragem (4x por segundo): elevador + todos os personagens visiveis nesta maquina ----
	if (Ph >= 1 && LogT >= 0.25f)
	{
		LogT = 0.f;
		FString L = FString::Printf(TEXT("srvT=%.3f ph=%d elevZ=%.1f mov=%d |"), Srv, Ph, E->GetActorLocation().Z, E->IsMoving() ? 1 : 0);
		for (TActorIterator<ACoopCharacter> It(W); It; ++It)
		{
			const FVector P = It->GetActorLocation(), R = P - E->GetActorLocation();
			UPrimitiveComponent* B = It->GetMovementBase();
			L += FString::Printf(TEXT(" [%s %s pos=(%.0f,%.0f,%.0f) rel=(%.0f,%.0f,%.0f) base=%s]"), (int32)It->CoopRole == 0 ? TEXT("Neon") : TEXT("Ruina"),
				It->IsLocallyControlled() ? TEXT("local") : TEXT("remoto"), P.X, P.Y, P.Z, R.X, R.Y, R.Z, B && B->GetOwner() ? *B->GetOwner()->GetName() : TEXT("-"));
		}
		Log += L + TEXT("\n");
		if (Ph == 3) // viagem: estabilidade do personagem local em relacao a plataforma
		{
			const FVector R = Me->GetActorLocation() - E->GetActorLocation();
			MinRel = FMath::Min(MinRel, R.Z); MaxRel = FMath::Max(MaxRel, R.Z);
			if (!bHaveRef) { RefRel = R; bHaveRef = true; }
			MaxDrift = FMath::Max(MaxDrift, FVector2D(R.X - RefRel.X, R.Y - RefRel.Y).Size());
		}
	}
	if (Ph >= 1 && ShotT >= 0.25f) { ShotT = 0.f; FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/elev/%s/f_%05d.jpg"), *Tag, Shots++), false, false); }
	if (SaveT > 3.f) { SaveT = 0.f; Save(); }

	switch (Ph)
	{
	case 0: // espera os dois jogadores na sessao
		if (GS->PlayerArray.Num() >= FMath::Max(1, FParse::Param(FCommandLine::Get(), TEXT("ElevSolo")) ? 1 : 2) && PhT > 4.f) SetPh(1, TEXT("2 jogadores presentes"));
		break;
	case 1: // caminha ate o centro da plataforma (embaixo no ciclo par, em cima no impar)
		if (Walk(FVector2D(Home.X, Home.Y) + Off) < 20.f) SetPh(2, TEXT("a bordo, aguardando partida"));
		else if (PhT > 40.f) SetPh(9, TEXT("TIMEOUT caminhando ate o elevador"));
		break;
	case 2: // aguarda o servidor acionar (todos a bordo + dwell)
		if (E->IsMoving()) { SetPh(3, TEXT("elevador partiu")); bHaveRef = false; }
		else if (PhT > 30.f) SetPh(9, TEXT("TIMEOUT: elevador nao partiu"));
		break;
	case 3: // viagem
		if (!E->IsMoving()) SetPh(4, TEXT("elevador chegou (parado)"));
		break;
	case 4: // parado com os personagens a bordo
		if (PhT > 1.5f) SetPh(5, TEXT("descer da plataforma"));
		break;
	case 5: // sai da plataforma: no topo vai para +X, embaixo para -X
	{
		const float Dir = E->IsTargetTop() ? 1.f : -1.f;
		if (Walk(FVector2D(Home.X + Dir * 420.f, Home.Y) + Off) < 20.f) SetPh(6, TEXT("fora do elevador, parado"));
		else if (PhT > 40.f) SetPh(9, TEXT("TIMEOUT saindo"));
		break;
	}
	case 6:
		if (PhT > 2.f)
		{
			if (!E->IsTargetTop()) { ++Cycles; Log += FString::Printf(TEXT("EVT ciclo %d completo (base->topo->base)\n"), Cycles); }
			if (Cycles >= 3) SetPh(7, TEXT("3 ciclos concluidos")); else SetPh(1, TEXT("novo embarque"));
		}
		break;
	case 7:
		Log += FString::Printf(TEXT("RESUMO(%s, local): relZ min=%.1f max=%.1f (var=%.1f cm) | deriva XY max=%.1f cm | ciclos=%d | capturas=%d\n"), *Tag, MinRel, MaxRel, MaxRel - MinRel, MaxDrift, Cycles, Shots);
		Save(); SetPh(8, TEXT("fim"));
		break;
	case 8:
		if (PhT > (bHost ? 14.f : 5.f)) { Save(); GEngine->Exec(W, TEXT("quit")); Ph = 10; }
		break;
	case 9: // falha: grava e encerra
		Log += TEXT("RESULTADO: FALHA/TIMEOUT\n"); Save();
		if (PhT > 6.f) { GEngine->Exec(W, TEXT("quit")); Ph = 10; }
		break;
	}
}
