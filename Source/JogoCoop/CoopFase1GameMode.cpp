#include "CoopFase1GameMode.h"
#include "CoopGameState.h"
#include "CoopCharacter.h"
#include "CoopHUD.h"
#include "CoopBlock.h"
#include "CoopSun.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

static void Lbl(AActor* A, const TCHAR* N)
{
#if WITH_EDITOR
	if (A) A->SetActorLabel(N);
#endif
}

ACoopFase1GameMode::ACoopFase1GameMode()
{
	DefaultPawnClass = ACoopCharacter::StaticClass();
	HUDClass = ACoopHUD::StaticClass();
	GameStateClass = ACoopGameState::StaticClass();
}

AActor* ACoopFase1GameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!bBuilt)
	{
		bBuilt = true;
		BuildLevel();
		GetWorldTimerManager().SetTimer(FallTimer, this, &ACoopFase1GameMode::CheckFall, 0.25f, true);
	}
	if (Starts.Num() == 0) return Super::ChoosePlayerStart_Implementation(Player);
	return Starts[NextStart++ % Starts.Num()];
}

void ACoopFase1GameMode::SpawnBarrier()
{
	// Barreira INVISIVEL provisoria no vao (ver relatorio): o vao de 450cm seria pulavel (pulo corrido ~700-800cm).
	// Ignora Visibility/Camera (ghost), entao nao atrapalha mira nem linha de visao dos poderes.
	UWorld* W = GetWorld();
	ACoopBlock* B = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), FVector(225, 0, 125), FRotator::ZeroRotator);
	if (B) { B->Setup(FVector(20, 1000, 950), FLinearColor::Black, true); Lbl(B, TEXT("F1_BarreiraVao_Provisoria")); }
	Barrier = B;
}

void ACoopFase1GameMode::BuildLevel()
{
	UWorld* W = GetWorld();
	W->SpawnActor<ACoopSun>(ACoopSun::StaticClass(), FVector(0, 0, 1000), FRotator::ZeroRotator);

	auto B = [&](const TCHAR* Name, FVector C, FVector S, FLinearColor Col, bool bGhost = false, FLinearColor Em = FLinearColor::Black, float Rough = 0.9f, float Metal = 0.f) -> ACoopBlock*
	{
		ACoopBlock* b = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), C, FRotator::ZeroRotator);
		if (b) { b->Setup(S, Col, bGhost, Em, Rough, Metal); Lbl(b, Name); }
		return b;
	};
	const FLinearColor Floor(0.30f, 0.31f, 0.34f), Floor2(0.42f, 0.43f, 0.46f), Wall(0.20f, 0.21f, 0.24f);

	// A. plataforma inicial: X -1400..0, topo Z=0
	B(TEXT("F1_PlataformaInicial"), FVector(-700, 0, -300), FVector(1400, 1000, 600), Floor);
	// E. plataforma de saida: X 450..1850, topo Z=+40 (elevada 40cm; degrau < MaxStepHeight 45)
	B(TEXT("F1_PlataformaSaida"), FVector(1150, 0, -280), FVector(1400, 1000, 640), Floor2);
	// C. vao central X 0..450 (4,5m), fundo de lava com topo em Z=-350 (queda 3,5m)
	B(TEXT("F1_Lava"), FVector(225, 0, -400), FVector(450, 1000, 100), FLinearColor(0.25f, 0.04f, 0.01f), false, FLinearColor(1.0f, 0.28f, 0.03f) * 2.5f, 0.6f);
	// G. paredes/teto
	B(TEXT("F1_ParedeLateralEsq"), FVector(225, -520, 50), FVector(3300, 40, 1300), Wall);
	B(TEXT("F1_ParedeLateralDir"), FVector(225, 520, 50), FVector(3300, 40, 1300), Wall);
	B(TEXT("F1_ParedeFundoInicio"), FVector(-1420, 0, 50), FVector(40, 1080, 1300), Wall);
	B(TEXT("F1_ParedeFundoSaida"), FVector(1870, 0, 50), FVector(40, 1080, 1300), Wall);
	B(TEXT("F1_Teto"), FVector(225, 0, 620), FVector(3340, 1080, 40), Wall, true);
	// B. coluna estrutural: junto a borda da plataforma inicial, encosta no teto (160x160x600)
	B(TEXT("F1_Coluna"), FVector(-200, 0, 300), FVector(160, 160, 600), FLinearColor(0.62f, 0.62f, 0.64f), false, FLinearColor(0.35f, 0.12f, 0.02f), 0.8f);
	// D. ponte danificada: toco ligado ao lado direito (X 300..450, largura 250) + laje caida no vao
	B(TEXT("F1_Ponte_Toco"), FVector(375, 0, 25), FVector(150, 250, 30), FLinearColor(0.55f, 0.52f, 0.46f));
	if (ACoopBlock* Slab = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), FVector(255, 0, -140), FRotator(-35.f, 0.f, 0.f)))
	{ Slab->Setup(FVector(150, 250, 30), FLinearColor(0.45f, 0.42f, 0.38f)); Lbl(Slab, TEXT("F1_Ponte_LajeCaida")); }
	// F. saida: porta emissiva na parede do fundo da plataforma de saida
	B(TEXT("F1_PortaSaida"), FVector(1845, 0, 170), FVector(10, 200, 260), FLinearColor(0.02f, 0.1f, 0.05f), false, FLinearColor(0.1f, 1.f, 0.4f) * 1.5f);

	SpawnBarrier();

	Starts.Add(W->SpawnActor<APlayerStart>(FVector(-1250, -150, 100), FRotator::ZeroRotator));
	Starts.Add(W->SpawnActor<APlayerStart>(FVector(-1250, 150, 100), FRotator::ZeroRotator));
	Starts.Add(W->SpawnActor<APlayerStart>(FVector(-1350, 0, 100), FRotator::ZeroRotator));
}

bool ACoopFase1GameMode::ToggleTestBridge()
{
	UWorld* W = GetWorld();
	if (FullBridge && IsValid(FullBridge))
	{
		FullBridge->Destroy(); FullBridge = nullptr;
		SpawnBarrier();
		return false;
	}
	if (Barrier && IsValid(Barrier)) { Barrier->Destroy(); Barrier = nullptr; }
	// ponte completa: X 0..450, largura 250, topo Z=+40 (liga a borda esquerda ao lado direito)
	ACoopBlock* Br = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), FVector(225, 0, 25), FRotator::ZeroRotator);
	if (Br) { Br->Setup(FVector(450, 250, 30), FLinearColor(0.55f, 0.52f, 0.46f)); Lbl(Br, TEXT("F1_Ponte_Completa")); }
	FullBridge = Br;
	return true;
}

void ACoopFase1GameMode::CheckFall()
{
	if (Starts.Num() == 0) return;
	int32 i = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It, ++i)
	{
		APlayerController* PC = It->Get();
		APawn* P = PC ? PC->GetPawn() : nullptr;
		if (!P || P->GetActorLocation().Z > -120.f) continue;
		P->SetActorLocation(Starts[i % Starts.Num()]->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		if (ACharacter* C = Cast<ACharacter>(P)) C->GetCharacterMovement()->StopMovementImmediately();
	}
}
