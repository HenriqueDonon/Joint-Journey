#include "CoopFase1GameMode.h"
#include "CoopGameState.h"
#include "CoopCharacter.h"
#include "CoopHUD.h"
#include "CoopBlock.h"
#include "CoopSun.h"
#include "CoopDestructible.h"
#include "CoopCeiling.h"
#include "CoopLamp.h"
#include "Components/StaticMeshComponent.h"
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
		GetWorldTimerManager().SetTimer(FallTimer, this, &ACoopFase1GameMode::CheckFall, 0.1f, true);
	}
	if (Starts.Num() == 0) return Super::ChoosePlayerStart_Implementation(Player);
	return Starts[NextStart++ % Starts.Num()];
}

void ACoopFase1GameMode::SpawnBarrier()
{
	// Barreira INVISIVEL provisoria no vao (ver relatorio): o vao seria pulavel (pulo corrido ~700-800cm).
	// Ignora Visibility/Camera (ghost), entao nao atrapalha mira nem linha de visao dos poderes.
	UWorld* W = GetWorld();
	ACoopBlock* B = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), FVector(500, 0, 125), FRotator::ZeroRotator);
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
	// E. plataforma de saida: X 1000..2400, topo Z=+40 (elevada 40cm; degrau < MaxStepHeight 45)
	B(TEXT("F1_PlataformaSaida"), FVector(1700, 0, -280), FVector(1400, 1000, 640), Floor2);
	// C. vao central X 0..1000 (10m), fundo de lava com topo em Z=-350 (queda 3,5m)
	B(TEXT("F1_Lava"), FVector(500, 0, -400), FVector(1000, 1000, 100), FLinearColor(0.25f, 0.04f, 0.01f), false, FLinearColor(1.0f, 0.28f, 0.03f) * 2.5f, 0.6f);
	// G. paredes/teto
	B(TEXT("F1_ParedeLateralEsq"), FVector(500, -520, 50), FVector(3850, 40, 1300), Wall);
	B(TEXT("F1_ParedeLateralDir"), FVector(500, 520, 50), FVector(3850, 40, 1300), Wall);
	B(TEXT("F1_ParedeFundoInicio"), FVector(-1420, 0, 50), FVector(40, 1080, 1300), Wall);
	B(TEXT("F1_ParedeFundoSaida"), FVector(2420, 0, 50), FVector(40, 1080, 1300), Wall);
	// teto cobrindo a sala toda (visual, cai no colapso) + paineis de luz + luzes sem sombra
	auto Roofed = [&](const TCHAR* Name, FVector C, FVector S, FLinearColor Col, FLinearColor Em = FLinearColor::Black) -> ACoopCeiling*
	{
		ACoopCeiling* c = W->SpawnActor<ACoopCeiling>(ACoopCeiling::StaticClass(), C, FRotator::ZeroRotator);
		if (c) { c->Setup(S, Col, false, Em, 0.9f, 0.f); Lbl(c, Name); Roof.Add(c); }
		return c;
	};
	Roofed(TEXT("F1_Teto"), FVector(500, 0, 620), FVector(3890, 1080, 40), FLinearColor(0.22f, 0.23f, 0.26f));
	for (float X : {-1200.f, -800.f, 300.f, 700.f, 1100.f, 1500.f, 1900.f, 2250.f})
		for (float Y : {-250.f, 250.f})
		{
			Roofed(TEXT("F1_PainelLuz"), FVector(X, Y, 596), FVector(160, 50, 8), FLinearColor(0.8f, 0.8f, 0.75f), FLinearColor(1.f, 0.95f, 0.8f) * 3.f);
			if (ACoopLamp* L = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), FVector(X, Y, 540), FRotator::ZeroRotator)) L->Setup(FLinearColor(1.f, 0.95f, 0.85f), 7000.f, 900.f);
		}
	Roofed(TEXT("F1_TetoEstrutural"), FVector(-200, 0, 590), FVector(600, 1000, 60), FLinearColor(0.26f, 0.27f, 0.30f));
	// luz sob o teto estrutural (ilumina a coluna) e brilho da lava (sem sombras)
	if (ACoopLamp* L = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), FVector(-200, 0, 500), FRotator::ZeroRotator)) L->Setup(FLinearColor(1.f, 0.95f, 0.85f), 9000.f, 900.f);
	if (ACoopLamp* L = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), FVector(500, 0, -250), FRotator::ZeroRotator)) L->Setup(FLinearColor(1.f, 0.35f, 0.08f), 40000.f, 1600.f);
	// B. coluna estrutural DESTRUTIVEL: ao lado do caminho da ponte (Y=-330), 160x160x560, sustenta o teto estrutural
	if (ACoopDestructible* Col = W->SpawnActorDeferred<ACoopDestructible>(ACoopDestructible::StaticClass(), FTransform(FVector(-200, -330, 280))))
	{
		Col->Setup(FVector(160, 160, 560), FLinearColor(0.62f, 0.62f, 0.64f));
		Col->FinishSpawning(FTransform(FVector(-200, -330, 280)));
		Lbl(Col, TEXT("F1_Coluna")); Column = Col;
	}
	// teto estrutural visivel apoiado pela coluna (X -500..100, base em Z=560 = topo da coluna)
	// D. ponte danificada: toco ligado ao lado direito (X 850..1000, largura 250) + laje caida no vao
	B(TEXT("F1_Ponte_Toco"), FVector(925, 0, 26), FVector(150, 240, 24), FLinearColor(0.55f, 0.52f, 0.46f));
	// ponte DESTRUTIVEL: nasce destruida (escombros na lava); Neon so a constroi depois que a coluna cair
	if (ACoopDestructible* Br = W->SpawnActorDeferred<ACoopDestructible>(ACoopDestructible::StaticClass(), FTransform(FVector(500, 0, 25))))
	{
		Br->Setup(FVector(1000, 250, 30), FLinearColor(0.55f, 0.52f, 0.46f), 360.f, 350.f, true);
		Br->FinishSpawning(FTransform(FVector(500, 0, 25)));
		Lbl(Br, TEXT("F1_Ponte")); Bridge = Br;
	}
	if (Column && Bridge) { Column->Partner = Bridge; Bridge->Partner = Column; }
	if (ACoopBlock* Slab = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), FVector(780, 0, -140), FRotator(-35.f, 0.f, 0.f)))
	{ Slab->Setup(FVector(150, 250, 30), FLinearColor(0.45f, 0.42f, 0.38f)); Lbl(Slab, TEXT("F1_Ponte_LajeCaida")); }
	// F. saida: porta emissiva na parede do fundo da plataforma de saida
	B(TEXT("F1_PortaSaida"), FVector(2395, 0, 170), FVector(10, 200, 260), FLinearColor(0.02f, 0.1f, 0.05f), false, FLinearColor(0.1f, 1.f, 0.4f) * 1.5f);

	SpawnBarrier();

	Starts.Add(W->SpawnActor<APlayerStart>(FVector(-1250, -150, 100), FRotator::ZeroRotator));
	Starts.Add(W->SpawnActor<APlayerStart>(FVector(-1250, 150, 100), FRotator::ZeroRotator));
	Starts.Add(W->SpawnActor<APlayerStart>(FVector(-1350, 0, 100), FRotator::ZeroRotator));
}

bool ACoopFase1GameMode::ToggleTestBridge()
{
	if (!Bridge) return false;
	Bridge->ServerForce(!Bridge->IsDestroyed());
	return !Bridge->IsDestroyed();
}

void ACoopFase1GameMode::CheckFall()
{
	TickCollapse();
	// barreira invisivel do vao existe so enquanto a ponte nao existe
	if (Bridge)
	{
		const bool bHasBarrier = Barrier && IsValid(Barrier);
		if (Bridge->IsDestroyed() && !bHasBarrier) SpawnBarrier();
		else if (!Bridge->IsDestroyed() && bHasBarrier) { Barrier->Destroy(); Barrier = nullptr; }
	}
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

void ACoopFase1GameMode::Announce(const FString& Text)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		if (ACoopCharacter* C = It->Get() ? Cast<ACoopCharacter>(It->Get()->GetPawn()) : nullptr) C->ClientToast(Text);
}

void ACoopFase1GameMode::SendAllToStart()
{
	if (Starts.Num() == 0) return;
	int32 i = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It, ++i)
	{
		APawn* P = It->Get() ? It->Get()->GetPawn() : nullptr;
		if (!P) continue;
		P->SetActorLocation(Starts[i % Starts.Num()]->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		if (ACharacter* C = Cast<ACharacter>(P)) C->GetCharacterMovement()->StopMovementImmediately();
	}
}

void ACoopFase1GameMode::ClearCracks()
{
	for (AActor* A : Cracks) if (A && IsValid(A)) A->Destroy();
	Cracks.Reset(); CracksSpawned = 0;
}

// rachadura em zigue-zague (3 segmentos) colada na face interna de uma das 4 paredes
void ACoopFase1GameMode::SpawnCrackCluster(int32 Idx)
{
	UWorld* W = GetWorld();
	FRandomStream R(1000 + Idx * 7919);
	const int32 Wall = Idx % 4; // 0: lateral Y- | 1: lateral Y+ | 2: fundo inicio | 3: fundo saida
	const bool bSide = Wall < 2;
	float U = bSide ? R.FRandRange(-1250.f, 2250.f) : R.FRandRange(-380.f, 380.f);
	float Z = R.FRandRange(110.f, 440.f);
	const float Base = 90.f + R.FRandRange(-40.f, 40.f); // quase vertical
	for (int32 s = 0; s < 3; ++s)
	{
		const float L = R.FRandRange(70.f, 150.f);
		const float Th = Base + (s % 2 ? 1.f : -1.f) * R.FRandRange(15.f, 35.f);
		const float dU = L * FMath::Cos(FMath::DegreesToRadians(Th)), dZ = L * FMath::Sin(FMath::DegreesToRadians(Th));
		const float CU = U + dU * 0.5f, CZ = Z + dZ * 0.5f;
		FVector Loc; FRotator Rot; FVector Sz;
		if (bSide) { Loc = FVector(CU, Wall == 0 ? -498.6f : 498.6f, CZ); Rot = FRotator(Th, 0.f, 0.f); Sz = FVector(L, 3.f, 5.f); }
		else { Loc = FVector(Wall == 2 ? -1398.6f : 2398.6f, CU, CZ); Rot = FRotator(0.f, 0.f, -Th); Sz = FVector(3.f, L, 5.f); }
		if (ACoopBlock* B = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), Loc, Rot))
		{
			B->Setup(Sz, FLinearColor(0.02f, 0.02f, 0.025f), false, FLinearColor(1.f, 0.28f, 0.04f) * 1.2f, 0.9f, 0.f);
			B->Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Lbl(B, TEXT("F1_Rachadura"));
			Cracks.Add(B);
		}
		U += dU; Z += dZ;
	}
}

// Colapso: coluna destruida -> 8s de contagem com rachaduras -> teto desaba -> todos voltam ao inicio e tudo reinicia.
// Se a coluna for reconstruida durante a contagem, o teto se estabiliza (contagem cancelada).
void ACoopFase1GameMode::TickCollapse()
{
	if (!Column) return;
	const float Now = GetWorld()->GetTimeSeconds();
	const bool bColDown = Column->IsDestroyed();
	if (Phase == 0)
	{
		if (bColDown) { Phase = 1; PhaseT0 = Now; CracksSpawned = 0; LastSecShown = -1; }
	}
	else if (Phase == 1)
	{
		if (!bColDown) { Phase = 0; ClearCracks(); Announce(TEXT("Coluna restaurada: teto estabilizado")); return; }
		const float El = Now - PhaseT0;
		const int32 Left = FMath::CeilToInt(8.f - El);
		if (Left != LastSecShown && Left > 0) { LastSecShown = Left; Announce(FString::Printf(TEXT("TETO INSTAVEL! Desaba em %d s - reconstrua a coluna!"), Left)); }
		const int32 Want = FMath::Min(16, FMath::FloorToInt(El / 0.5f) + 1);
		while (CracksSpawned < Want) SpawnCrackCluster(CracksSpawned++);
		if (El >= 8.f)
		{
			Phase = 2; PhaseT0 = Now;
			for (ACoopCeiling* C : Roof) if (C) C->StartFall(0.8f, 560.f);
			Announce(TEXT("O TETO DESABOU!"));
		}
	}
	else if (Phase == 2 && Now - PhaseT0 >= 1.4f)
	{
		SendAllToStart();                    // todos voltam ao inicio
		ClearCracks();
		for (ACoopCeiling* C : Roof) if (C) C->ResetPos();
		Column->ServerForce(false);          // estruturas voltam ao estado inicial
		if (Bridge) Bridge->ServerForce(true);
		Phase = 0;
		Announce(TEXT("Fase reiniciada"));
	}
}
