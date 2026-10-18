#include "CoopFase1GameMode.h"
#include "CoopGameState.h"
#include "CoopCharacter.h"
#include "CoopHUD.h"
#include "CoopBlock.h"
#include "CoopSun.h"
#include "CoopDestructible.h"
#include "CoopCeiling.h"
#include "CoopLamp.h"
#include "CoopFog.h"
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
	auto M = [&](ACoopBlock* b, const TCHAR* Slot) { if (b) b->SetOpts(FName(Slot), false); };
	const FLinearColor Floor(0.30f, 0.31f, 0.34f), Floor2(0.42f, 0.43f, 0.46f), Wall(0.20f, 0.21f, 0.24f);

	// A. plataforma inicial: X -1400..0, topo Z=0
	M(B(TEXT("F1_PlataformaInicial"), FVector(-700, 0, -300), FVector(1400, 1000, 600), Floor), TEXT("Plataforma_Inicial"));
	// E. plataforma de saida: X 1000..2400, topo Z=+40 (elevada 40cm; degrau < MaxStepHeight 45)
	M(B(TEXT("F1_PlataformaSaida"), FVector(1700, 0, -280), FVector(1400, 1000, 640), Floor2), TEXT("Plataforma_Saida"));
	// C. vao central X 0..1000 (10m), fundo de lava com topo em Z=-350 (queda 3,5m)
	M(B(TEXT("F1_Lava"), FVector(500, 0, -400), FVector(1000, 1000, 100), FLinearColor(0.25f, 0.04f, 0.01f), false, FLinearColor(1.0f, 0.28f, 0.03f) * 2.5f, 0.6f), TEXT("Lava"));
	// G. paredes/teto
	M(B(TEXT("F1_ParedeLateralEsq"), FVector(500, -520, 50), FVector(3850, 40, 1300), Wall), TEXT("Parede_Lateral_Esq"));
	M(B(TEXT("F1_ParedeLateralDir"), FVector(500, 520, 50), FVector(3850, 40, 1300), Wall), TEXT("Parede_Lateral_Dir"));
	M(B(TEXT("F1_ParedeFundoInicio"), FVector(-1420, 0, 50), FVector(40, 1080, 1300), Wall), TEXT("Parede_Fundo_Inicio"));
	M(B(TEXT("F1_ParedeFundoSaida"), FVector(2420, 0, 50), FVector(40, 1080, 1300), Wall), TEXT("Parede_Fundo_Saida"));
	// teto cobrindo a sala toda (visual, cai no colapso) + paineis de luz + luzes sem sombra
	auto Roofed = [&](const TCHAR* Name, FVector C, FVector S, FLinearColor Col, FLinearColor Em = FLinearColor::Black) -> ACoopCeiling*
	{
		ACoopCeiling* c = W->SpawnActor<ACoopCeiling>(ACoopCeiling::StaticClass(), C, FRotator::ZeroRotator);
		if (c) { c->Setup(S, Col, false, Em, 0.9f, 0.f); Lbl(c, Name); Roof.Add(c); }
		return c;
	};
	if (ACoopCeiling* T = Roofed(TEXT("F1_Teto"), FVector(500, 0, 620), FVector(3890, 1080, 40), FLinearColor(0.22f, 0.23f, 0.26f))) T->SetOpts(TEXT("Teto_Principal"), false);
	for (float X : {-1200.f, -800.f, 300.f, 700.f, 1100.f, 1500.f, 1900.f, 2250.f})
		for (float Y : {-250.f, 250.f})
		{
			Roofed(TEXT("F1_PainelLuz"), FVector(X, Y, 596), FVector(160, 50, 8), FLinearColor(0.8f, 0.8f, 0.75f), FLinearColor(1.f, 0.95f, 0.8f) * 3.f);
			if (ACoopLamp* L = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), FVector(X, Y, 540), FRotator::ZeroRotator)) L->Setup(X > 900.f ? FLinearColor(0.82f, 0.9f, 1.f) : FLinearColor(1.f, 0.9f, 0.78f), 7000.f, 900.f);
		}
	if (ACoopCeiling* T = Roofed(TEXT("F1_TetoEstrutural"), FVector(-200, 0, 590), FVector(600, 1000, 60), FLinearColor(0.26f, 0.27f, 0.30f))) T->SetOpts(TEXT("Teto_Estrutural"), false);
	// luz sob o teto estrutural (ilumina a coluna) e brilho da lava (sem sombras)
	if (ACoopLamp* L = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), FVector(-200, 0, 500), FRotator::ZeroRotator)) L->Setup(FLinearColor(1.f, 0.95f, 0.85f), 9000.f, 900.f);
	if (ACoopLamp* L = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), FVector(500, 0, -250), FRotator::ZeroRotator)) L->Setup(FLinearColor(1.f, 0.35f, 0.08f), 40000.f, 1600.f);
	// B. coluna estrutural DESTRUTIVEL: ao lado do caminho da ponte (Y=-330), 160x160x560, sustenta o teto estrutural
	if (ACoopDestructible* Col = W->SpawnActorDeferred<ACoopDestructible>(ACoopDestructible::StaticClass(), FTransform(FVector(-200, -330, 280))))
	{
		Col->Setup(FVector(160, 160, 560), FLinearColor(0.62f, 0.62f, 0.64f));
		Col->Model = TEXT("Coluna_Inteira");
		Col->FinishSpawning(FTransform(FVector(-200, -330, 280)));
		Lbl(Col, TEXT("F1_Coluna")); Column = Col;
	}
	// teto estrutural visivel apoiado pela coluna (X -500..100, base em Z=560 = topo da coluna)
	// D. ponte danificada: toco ligado ao lado direito (X 850..1000, largura 250) + laje caida no vao
	M(B(TEXT("F1_Ponte_Toco"), FVector(925, 0, 26), FVector(150, 240, 24), FLinearColor(0.55f, 0.52f, 0.46f)), TEXT("Ponte_Quebrada"));
	// ponte DESTRUTIVEL: nasce destruida (escombros na lava); Neon so a constroi depois que a coluna cair
	if (ACoopDestructible* Br = W->SpawnActorDeferred<ACoopDestructible>(ACoopDestructible::StaticClass(), FTransform(FVector(500, 0, 25))))
	{
		Br->Setup(FVector(1000, 250, 30), FLinearColor(0.55f, 0.52f, 0.46f), 360.f, 350.f, true);
		Br->Model = TEXT("Ponte_Inteira");
		Br->FinishSpawning(FTransform(FVector(500, 0, 25)));
		Lbl(Br, TEXT("F1_Ponte")); Bridge = Br;
	}
	if (Column && Bridge) { Column->Partner = Bridge; Bridge->Partner = Column; }
	if (ACoopBlock* Slab = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), FVector(780, 0, -140), FRotator(-35.f, 0.f, 0.f)))
	{ Slab->Setup(FVector(150, 250, 30), FLinearColor(0.45f, 0.42f, 0.38f)); Lbl(Slab, TEXT("F1_Ponte_LajeCaida")); }
	// F. saida: porta emissiva na parede do fundo da plataforma de saida
	M(B(TEXT("F1_PortaSaida"), FVector(2395, 0, 170), FVector(10, 200, 260), FLinearColor(0.02f, 0.1f, 0.05f), false, FLinearColor(0.1f, 1.f, 0.4f) * 1.5f), TEXT("Porta_Saida"));

	SpawnBarrier();
	W->SpawnActor<ACoopFog>(ACoopFog::StaticClass(), FVector(500, 0, 200), FRotator::ZeroRotator);
	BuildDecor();

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
			B->SetOpts(NAME_None, true);
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

// Decoracao procedural (so visual, SEM colisao) para dar profundidade/atmosfera ate os modelos do Blender entrarem.
// Cada peca tem um "slot": se existir /Game/Fase1/<Slot>/SM_<Slot>, o modelo substitui o placeholder (ver MapaFase1/MODELOS.md).
void ACoopFase1GameMode::BuildDecor()
{
	UWorld* W = GetWorld();
	auto D = [&](const TCHAR* Slot, FVector C, FVector S, FLinearColor Col, FLinearColor Em = FLinearColor::Black, FRotator Rot = FRotator::ZeroRotator, bool bCollide = false) -> ACoopBlock*
	{
		ACoopBlock* b = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), C, Rot);
		if (b) { b->Setup(S, Col, false, Em, 0.85f, 0.25f); b->SetOpts(FName(Slot), !bCollide); Lbl(b, Slot); }
		return b;
	};
	auto Lamp = [&](FVector P, FLinearColor C, float Lm, float Rad, float Fl) { if (ACoopLamp* L = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), P, FRotator::ZeroRotator)) L->Setup(C, Lm, Rad, Fl); };
	const FLinearColor Rust(0.30f, 0.13f, 0.06f), Dark(0.05f, 0.05f, 0.06f), Steel(0.36f, 0.38f, 0.41f), Grime(0.16f, 0.15f, 0.15f);
	const FLinearColor Cyan(0.05f, 0.9f, 1.f), Orange(1.f, 0.4f, 0.05f), Fire(1.f, 0.3f, 0.04f);
	FRandomStream R(4242);

	for (int32 s = -1; s <= 1; s += 2)
	{
		const float Y = s * 497.f;
		D(TEXT("Parede_Rodape"), FVector(500, s * 495.f, 75), FVector(3800, 8, 150), Rust);                 // rodape enferrujado
		for (float X = -1300.f; X <= 2300.f; X += 250.f)
			D(TEXT("Parede_Junta"), FVector(X, Y, 300), FVector(5, 3, 500), Dark);                          // juntas dos paineis
		int32 k = 0;
		for (float X = -1300.f; X <= 2300.f; X += 300.f)
		{
			if (X > -100.f && X < 1100.f) continue;
			D(TEXT("Neon_Parede"), FVector(X, Y - s * 1.f, 300), FVector(200, 4, 8), Dark, ((k++ % 2) ? Orange : Cyan) * 2.5f); // faixas neon ciano/laranja
		}
		// janelas superiores: fogo do lado inicial, luz fria do lado da saida
		for (float X : {-1100.f, -700.f, -300.f, 1300.f, 1700.f, 2100.f})
		{
			const bool bStart = X < 0.f;
			D(TEXT("Janela_Moldura"), FVector(X, s * 494.f, 440), FVector(300, 6, 170), Dark);
			D(TEXT("Janela_Vidro"), FVector(X, s * 491.f, 440), FVector(270, 3, 140), Dark, (bStart ? FLinearColor(1.f, 0.33f, 0.06f) * 1.6f : FLinearColor(0.5f, 0.75f, 1.f) * 1.1f));
		}
	}
	// canos no teto + vigas quebradas penduradas sobre o vao + gotejo de lava
	D(TEXT("Cano_Teto"), FVector(500, -380, 565), FVector(3700, 36, 36), Rust);
	D(TEXT("Cano_Teto"), FVector(500, 380, 565), FVector(3700, 36, 36), Rust);
	D(TEXT("Cano_Teto"), FVector(500, 0, 575), FVector(3700, 56, 56), Steel);
	D(TEXT("Viga_Quebrada"), FVector(250, -120, 470), FVector(340, 26, 26), Rust, FLinearColor::Black, FRotator(-28.f, 15.f, 0.f));
	D(TEXT("Viga_Quebrada"), FVector(700, 150, 500), FVector(300, 26, 26), Steel, FLinearColor::Black, FRotator(24.f, -25.f, 0.f));
	D(TEXT("Viga_Quebrada"), FVector(480, 20, 430), FVector(280, 22, 22), Rust, FLinearColor::Black, FRotator(-12.f, 40.f, 8.f));
	D(TEXT("Viga_Quebrada"), FVector(850, -150, 520), FVector(240, 22, 22), Steel, FLinearColor::Black, FRotator(35.f, 10.f, 0.f));
	D(TEXT("Lava_Gota"), FVector(560, 60, 400), FVector(8, 8, 360), FLinearColor::Black, Orange * 4.f);
	// fluxos de lava + brilho vindo do fundo do vao
	const FVector4 Flows[] = { FVector4(150, -150, 260, 380), FVector4(430, 200, 300, 330), FVector4(720, -90, 280, 420), FVector4(900, 230, 200, 300) };
	for (const FVector4& F : Flows) D(TEXT("Lava_Fluxo"), FVector(F.X, F.Y, -349.f), FVector(F.Z, F.W, 2), FLinearColor::Black, FLinearColor(1.f, 0.5f, 0.05f) * 5.f);
	for (float X : {200.f, 500.f, 800.f}) Lamp(FVector(X, 0, -200), FLinearColor(1.f, 0.35f, 0.08f), 30000.f, 1400.f, 0.35f);
	// focos de incendio
	const FVector2D Fires[] = { FVector2D(-1250, -420), FVector2D(-950, 440), FVector2D(-600, -440), FVector2D(1400, 440), FVector2D(2000, -440) };
	for (const FVector2D& P : Fires)
	{
		const float Z0 = P.X > 900.f ? 40.f : 0.f;
		for (int32 i = 0; i < 4; ++i)
		{
			const float Hh = R.FRandRange(60.f, 150.f), Ww = R.FRandRange(28.f, 55.f);
			D(TEXT("Fogo"), FVector(P.X + R.FRandRange(-50.f, 50.f), P.Y + R.FRandRange(-50.f, 50.f), Z0 + Hh * 0.5f), FVector(Ww, Ww, Hh), FLinearColor::Black, Fire * 3.f, FRotator(0.f, R.FRandRange(0.f, 90.f), 0.f));
		}
		Lamp(FVector(P.X, P.Y, Z0 + 110), FLinearColor(1.f, 0.4f, 0.08f), 14000.f, 750.f, 0.5f);
	}
	// escombros: base da coluna + espalhados
	auto Rubble = [&](float CX, float CY, float Z0, float Rad, int32 N)
	{
		for (int32 i = 0; i < N; ++i)
		{
			const float S = R.FRandRange(12.f, 46.f), A = R.FRandRange(0.f, 6.283f), Dd = R.FRandRange(30.f, Rad);
			D(TEXT("Escombro"), FVector(CX + FMath::Cos(A) * Dd, CY + FMath::Sin(A) * Dd, Z0 + S * 0.4f), FVector(S, S * R.FRandRange(0.6f, 1.4f), S * R.FRandRange(0.5f, 1.f)), (i % 3 == 0) ? Rust : Grime, FLinearColor::Black, FRotator(R.FRandRange(-20.f, 20.f), R.FRandRange(0.f, 360.f), R.FRandRange(-20.f, 20.f)));
		}
	};
	Rubble(-200, -330, 0, 260, 18);
	Rubble(-800, 0, 0, 450, 14);
	Rubble(1500, 0, 40, 450, 12);
	// juntas do piso (ladrilhos) e faixas de perigo nas bordas
	for (int32 i = 0; i < 7; ++i) D(TEXT("Piso_Junta"), FVector(-1300.f + i * 200.f, 0, 0.6f), FVector(3, 1000, 1), Dark);
	for (int32 i = 0; i < 4; ++i) D(TEXT("Piso_Junta"), FVector(-700, -300.f + i * 200.f, 0.6f), FVector(1400, 3, 1), Dark);
	for (int32 i = 0; i < 7; ++i) D(TEXT("Piso_Junta"), FVector(1100.f + i * 200.f, 0, 40.6f), FVector(3, 1000, 1), Dark);
	for (int32 i = 0; i < 4; ++i) D(TEXT("Piso_Junta"), FVector(1700, -300.f + i * 200.f, 40.6f), FVector(1400, 3, 1), Dark);
	D(TEXT("Faixa_Perigo"), FVector(-15, 0, 0.7f), FVector(24, 1000, 1), FLinearColor(0.9f, 0.7f, 0.05f));
	D(TEXT("Faixa_Perigo"), FVector(1015, 0, 40.7f), FVector(24, 1000, 1), FLinearColor(0.9f, 0.7f, 0.05f));
	// discos vermelhos no piso da saida
	for (float Y : {-300.f, 0.f, 300.f}) D(TEXT("Disco_Piso"), FVector(1900, Y, 42.f), FVector(90, 90, 4), FLinearColor(0.3f, 0.03f, 0.02f), FLinearColor(1.f, 0.1f, 0.05f) * 1.5f);
	// corrimao laranja nas bordas do vao (abertura de 280 para a ponte) - so visual
	for (float EdgeX : {-6.f, 1006.f})
	{
		const float Z0 = EdgeX > 500.f ? 40.f : 0.f;
		D(TEXT("Corrimao"), FVector(EdgeX, -320, Z0 + 100), FVector(5, 360, 5), Orange * 0.6f, FLinearColor::Black);
		D(TEXT("Corrimao"), FVector(EdgeX, 320, Z0 + 100), FVector(5, 360, 5), Orange * 0.6f, FLinearColor::Black);
		for (float Y : {-480.f, -360.f, -240.f, 240.f, 360.f, 480.f}) D(TEXT("Corrimao_Poste"), FVector(EdgeX, Y, Z0 + 50), FVector(6, 6, 100), Rust);
	}
	// sinalizacao de saida (EXIT) + moldura da porta
	D(TEXT("Placa_Saida"), FVector(2393, 0, 345), FVector(6, 170, 44), FLinearColor(0.8f, 0.8f, 0.8f), FLinearColor(1.f, 1.f, 1.f) * 2.5f);
	D(TEXT("Porta_Moldura"), FVector(2396, 0, 175), FVector(8, 240, 300), Dark);
	// mezanino lateral (visual) com corrimao, ao fundo da area de saida
	D(TEXT("Mezanino"), FVector(1900, -400, 395), FVector(900, 190, 20), Steel);
	D(TEXT("Mezanino_Corrimao"), FVector(1900, -310, 440), FVector(900, 4, 60), Orange * 0.6f);
	for (float X : {1500.f, 1900.f, 2300.f}) D(TEXT("Mezanino_Pilar"), FVector(X, -330, 210), FVector(24, 24, 400), Rust);
}
