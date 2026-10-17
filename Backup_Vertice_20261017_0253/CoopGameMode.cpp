#include "CoopGameMode.h"
#include "CoopCharacter.h"
#include "CoopHUD.h"
#include "CoopBlock.h"
#include "CoopProp.h"
#include "CoopGoal.h"
#include "CoopEasterEgg.h"
#include "CoopSun.h"
#include "CoopDressing.h"
#include "PuzzleButton.h"
#include "PuzzleDoor.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

ACoopGameMode::ACoopGameMode() { DefaultPawnClass = ACoopCharacter::StaticClass(); HUDClass = ACoopHUD::StaticClass(); }

// Sala = corredor ao longo de X (2400 x 1200 x 400). Divisória em x=0 com porta no meio.
// Lado y<0 = Neon, lado y>0 = Ruína. Sala 1 no origin 0, Sala 2 em x=+5000.
void ACoopGameMode::BuildRoom(int32 Idx, FVector O)
{
	UWorld* W = GetWorld();
	const FLinearColor Wall(0.78f, 0.77f, 0.74f), Black = FLinearColor::Black;
	auto B = [&](FVector C, FVector S, FLinearColor Col, bool bGhost = false, FLinearColor Em = FLinearColor::Black, float Rough = 0.95f, float Metal = 0.f) -> ACoopBlock*
	{
		ACoopBlock* b = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), O + C, FRotator::ZeroRotator);
		if (b) b->Setup(S, Col, bGhost, Em, Rough, Metal);
		return b;
	};

	// chão: lado Neon (escuro, brilhante) e lado Ruína (concreto)
	B(FVector(0, -300, -50), FVector(2400, 600, 100), FLinearColor(0.015f, 0.015f, 0.04f), false, Black, 0.12f, 0.7f);
	B(FVector(0, 300, -50),  FVector(2400, 600, 100), FLinearColor(0.30f, 0.30f, 0.31f), false, Black, 0.97f, 0.f);
	B(FVector(0, 0, 420),    FVector(2400, 1200, 40), Wall, true);                     // teto invisível
	// paredes brancas
	B(FVector(0, -600, 200), FVector(2400, 40, 400), Wall);
	B(FVector(0, 600, 200),  FVector(2400, 40, 400), Wall);
	B(FVector(-1200, 0, 200), FVector(40, 1200, 400), Wall);
	B(FVector(1200, 0, 200),  FVector(40, 1200, 400), Wall);
	B(FVector(0, -375, 200), FVector(40, 450, 400), Wall);                              // divisória c/ vão de 300
	B(FVector(0, 375, 200),  FVector(40, 450, 400), Wall);
	B(FVector(0, 0, 350),    FVector(40, 300, 100), Wall);                              // verga

	APuzzleDoor* Door = W->SpawnActor<APuzzleDoor>(APuzzleDoor::StaticClass(), O + FVector(0, 0, 150), FRotator(0, 90, 0));
	ACoopGoal* Goal = W->SpawnActor<ACoopGoal>(ACoopGoal::StaticClass(), O + FVector(950, 0, 150), FRotator::ZeroRotator);
	if (Goal) Goal->RoomIndex = Idx;

	if (Idx == 1)
	{
		APuzzleButton* Btn = W->SpawnActor<APuzzleButton>(APuzzleButton::StaticClass(), O + FVector(-500, 0, 0), FRotator::ZeroRotator);
		if (Btn && Door) Btn->Doors.Add(Door);
		W->SpawnActor<ACoopProp>(ACoopProp::StaticClass(), O + FVector(-700, 300, 60), FRotator::ZeroRotator);

		if (ACoopEasterEgg* E1 = W->SpawnActor<ACoopEasterEgg>(ACoopEasterEgg::StaticClass(), O + FVector(-1080, 520, 0), FRotator(0, 25, 0)))
			E1->Message = TEXT("Easter egg 1/2: um cigarro enrolado na seda, pontinha torcida do jeito de sempre. Noite de jogo aprovada.");
		if (ACoopEasterEgg* E2 = W->SpawnActor<ACoopEasterEgg>(ACoopEasterEgg::StaticClass(), O + FVector(1060, -520, 0), FRotator(0, -140, 0)))
			E2->Message = TEXT("Easter egg 2/2: outro apertado, esse ficou pro final. Joint Journey: a jornada e conjunta.");

		Starts.Add(W->SpawnActor<APlayerStart>(O + FVector(-700, -150, 100), FRotator::ZeroRotator));
		Starts.Add(W->SpawnActor<APlayerStart>(O + FVector(-700, 150, 100), FRotator::ZeroRotator));
	}
	else
	{
		// Fase 2: o botão (que trava ligado) está no alto de uma plataforma. Só dá pra subir
		// se o Neon soltar o cubo pra cima e a Ruína congelar ele no ar como degrau.
		B(FVector(-1000, 400, 150), FVector(300, 300, 300), FLinearColor(0.36f, 0.36f, 0.37f));
		APuzzleButton* Btn = W->SpawnActor<APuzzleButton>(APuzzleButton::StaticClass(), O + FVector(-1000, 400, 300), FRotator::ZeroRotator);
		if (Btn) { Btn->bLatch = true; if (Door) Btn->Doors.Add(Door); }
		W->SpawnActor<ACoopProp>(ACoopProp::StaticClass(), O + FVector(-800, 400, 52), FRotator::ZeroRotator);
		Room2Starts.Add(O + FVector(-500, -150, 100));
		Room2Starts.Add(O + FVector(-500, 150, 100));
	}

	if (ACoopDressing* D = W->SpawnActorDeferred<ACoopDressing>(ACoopDressing::StaticClass(), FTransform(O)))
	{
		D->Seed = 1337 + Idx * 77;
		D->Variant = Idx;
		D->FinishSpawning(FTransform(O));
	}
}

AActor* ACoopGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!bBuilt)
	{
		bBuilt = true;
		GetWorld()->SpawnActor<ACoopSun>(ACoopSun::StaticClass(), FVector(0, 0, 1000), FRotator::ZeroRotator);
		BuildRoom(1, FVector::ZeroVector);
		BuildRoom(2, FVector(5000, 0, 0));
	}
	if (Starts.Num() == 0) return Super::ChoosePlayerStart_Implementation(Player);
	return Starts[NextStart++ % Starts.Num()];
}

void ACoopGameMode::RoomCompleted(int32 RoomIndex)
{
	if (RoomIndex == 1)
		GetWorldTimerManager().SetTimer(TeleportTimer, this, &ACoopGameMode::TeleportToRoom2, 4.f, false);
}

void ACoopGameMode::TeleportToRoom2()
{
	if (Room2Starts.Num() < 2) return;
	int32 i = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It, ++i)
	{
		APlayerController* PC = It->Get();
		if (APawn* P = PC ? PC->GetPawn() : nullptr)
		{
			P->SetActorLocationAndRotation(Room2Starts[i % 2], FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
			PC->ClientSetRotation(FRotator(-18.f, 0.f, 0.f));
		}
	}
}
