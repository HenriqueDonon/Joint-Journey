#include "CoopBalanceMachine.h"
#include "CoopDestructible.h"
#include "CoopGate.h"
#include "JJVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

// geometria (cm, mundo): salas de B no Fase2_Funcional (piso 600, teto 1160). Plataformas 400x400 centradas em X 1450, Y +-760.
static const FVector2D GPlatN(1450.f, 760.f), GPlatS(1450.f, -760.f);
static const FVector GCradle(1650.f, 0.f, 880.f);

ACoopBalanceMachine::ACoopBalanceMachine()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; bAlwaysRelevant = true;
	SetReplicateMovement(false);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Raiz"));
}

void ACoopBalanceMachine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopBalanceMachine, St);
	DOREPLIFETIME(ACoopBalanceMachine, Brake); DOREPLIFETIME(ACoopBalanceMachine, Clamp0); DOREPLIFETIME(ACoopBalanceMachine, Clamp1);
	DOREPLIFETIME(ACoopBalanceMachine, Clamp2); DOREPLIFETIME(ACoopBalanceMachine, Chute); DOREPLIFETIME(ACoopBalanceMachine, Pin);
}

float ACoopBalanceMachine::Now() const
{
	const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	return GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}

// deslocamento deterministico: do P0 ate o alvo a velocidade constante, a partir de T0 (igual em servidor e clientes)
float ACoopBalanceMachine::PAt(float T) const
{
	if ((!St.bBrakeFree && !St.bReturning) || St.bLocked) return St.P0;
	const float D = St.Target - St.P0, S = Speed * FMath::Max(T - St.T0, 0.f);
	return St.P0 + FMath::Sign(D) * FMath::Min(FMath::Abs(D), S);
}
float ACoopBalanceMachine::PlatformOffset() const { return PAt(Now()); }
int32 ACoopBalanceMachine::MassNorth() const { int32 N = 0; for (int32 i = 0; i < 3; ++i) N += Loc(i) == 0; return N; }
int32 ACoopBalanceMachine::MassSouth() const { int32 N = 1; for (int32 i = 0; i < 3; ++i) N += Loc(i) == 2; return N; }

void ACoopBalanceMachine::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority()) { SpawnParts(); St.P0 = St.Target = JamP; St.T0 = Now(); }
	Build();
}

void ACoopBalanceMachine::SpawnParts()
{
	auto Make = [&](FVector C, FVector Size, FRotator R, FLinearColor Col, bool bBroken, float Drop) -> ACoopDestructible*
	{
		const FTransform T(R, C);
		ACoopDestructible* D = GetWorld()->SpawnActorDeferred<ACoopDestructible>(ACoopDestructible::StaticClass(), T);
		D->Setup(Size, Col, Drop, 0.f, bBroken);
		return D;
	};
	const float TopN = LevelZ + JamP;
	Brake = Make(FVector(1450, 470, 1080), FVector(100, 60, 70), FRotator::ZeroRotator, FLinearColor(0.35f, 0.12f, 0.06f), false, 1055.f - 600.f);
	Brake->bCanRebuild = false;
	ACoopDestructible** Cl[3] = { &Clamp0, &Clamp1, &Clamp2 };
	for (int32 i = 0; i < 3; ++i)
	{
		*Cl[i] = Make(FVector(1350.f + 100.f * i, 812.f, TopN + 40.f), FVector(80, 20, 80), FRotator::ZeroRotator, FLinearColor(0.4f, 0.2f, 0.08f), false, 0.f);
		(*Cl[i])->bCanRebuild = false;
	}
	// calha de transferencia (sul): do hub ate a plataforma sul; peca AUSENTE (sem escombros proprios)
	Chute = Make(FVector(1600, -440, 1000), FVector(240, 70, 16), FRotator(0, 90, 0), FLinearColor(0.25f, 0.35f, 0.5f), true, 0.f);
	Chute->bCanBreak = false; Chute->bShowOwnDebris = false; Chute->bAssemblyFx = true;
	// pino de travamento (sul): encaixa na trava da plataforma na altura de nivel
	Pin = Make(FVector(1180, -760, LevelZ - 15.f), FVector(100, 30, 30), FRotator::ZeroRotator, FLinearColor(0.25f, 0.35f, 0.5f), true, 0.f);
	Pin->bCanBreak = false; Pin->bShowOwnDebris = false; Pin->bAssemblyFx = true;
	// material: cada reconstrucao consome a pilha de destrocos mais antiga (Partner = origem real, usada na animacao)
	Chute->RebuildCheck = [this](ACoopDestructible* D, FString& Why) -> bool
	{
		if (St.bDone) { Why = TEXT("Mecanismo ja travado"); return false; }
		if (Piles.Num() == 0) { Why = TEXT("Sem material"); return false; }
		D->Partner = Piles[0]; Piles.RemoveAt(0); return true;
	};
	Pin->RebuildCheck = [this](ACoopDestructible* D, FString& Why) -> bool
	{
		const float P = PAt(Now());
		if (!St.bBrakeFree || FMath::Abs(P) > PinTol || FMath::Abs(St.Target) > 0.5f) { Why = TEXT("O pino nao encaixa"); return false; }
		if (Piles.Num() == 0) { Why = TEXT("Sem material"); return false; }
		D->Partner = Piles[0]; Piles.RemoveAt(0); return true;
	};
	for (ACoopDestructible* D : { Brake, Clamp0, Clamp1, Clamp2, Chute, Pin }) D->FinishSpawning(D->GetActorTransform());
	UE_LOG(LogTemp, Warning, TEXT("F2X P2 maquina montada: freio, 3 bracadeiras, calha e pino"));
}

void ACoopBalanceMachine::Recalc()
{
	const float T = Now();
	St.P0 = PAt(T); St.T0 = T;
	if (St.bBrakeFree) St.bReturning = false;
	St.Target = (St.bBrakeFree && !St.bLocked) ? FMath::Clamp(PerUnit * (MassSouth() - MassNorth()), -Range, Range) : St.bReturning ? JamP : St.P0;
	ForceNetUpdate();
}

void ACoopBalanceMachine::FlowLingotes()
{
	for (int32 Guard = 0; Guard < 6; ++Guard)
	{
		bool bMoved = false;
		int32 InCradle = -1; for (int32 i = 0; i < 3; ++i) if (Loc(i) == 1) InCradle = i;
		if (InCradle >= 0 && Chute && !Chute->IsDestroyed())
		{
			int32 NS = 0; for (int32 i = 0; i < 3; ++i) NS += Loc(i) == 2;
			UE_LOG(LogTemp, Warning, TEXT("F2X P2 lingote %d: berco -> plataforma SUL"), InCradle);
			SetLoc(InCradle, 2); SetSSlot(InCradle, (uint8)NS); bMoved = true; InCradle = -1;
		}
		if (InCradle < 0)
			for (int32 i = 0; i < 3; ++i)
				if (bReleased[i] && Loc(i) == 0) { SetLoc(i, 1); bMoved = true; UE_LOG(LogTemp, Warning, TEXT("F2X P2 lingote %d: gaiola norte -> berco do hub"), i); break; }
		if (!bMoved) break;
	}
	Recalc();
}

void ACoopBalanceMachine::Complete()
{
	St.bDone = true;
	for (ACoopDestructible* D : { Brake, Clamp0, Clamp1, Clamp2, Chute, Pin }) if (D) { D->bCanBreak = false; D->bCanRebuild = false; }
	for (TActorIterator<ACoopGate> It(GetWorld()); It; ++It)
		if (It->GateId == FName(TEXT("P2_Norte")) || It->GateId == FName(TEXT("P2_Sul"))) It->ServerSetOpen(true);
	ForceNetUpdate();
	UE_LOG(LogTemp, Warning, TEXT("F2X P2 CONCLUIDO: equilibrio %d x %d travado, portoes abertos"), MassNorth(), MassSouth());
}

void ACoopBalanceMachine::ServerTick()
{
	if (!Brake || !Chute || !Pin) return;
	if (St.bReturning && FMath::Abs(PAt(Now()) - JamP) < 0.5f) { St.P0 = St.Target = JamP; St.T0 = Now(); St.bReturning = false; ForceNetUpdate(); }
	bool bChange = false;
	if (Brake->IsDestroyed() && !bPrevBrake) { Piles.Add(Brake); St.bBrakeFree = true; bChange = true; UE_LOG(LogTemp, Warning, TEXT("F2X P2 freio destruido: maquina livre")); }
	bPrevBrake = Brake->IsDestroyed();
	for (int32 i = 0; i < 3; ++i)
	{
		ACoopDestructible* C = ClampOf(i);
		if (C && C->IsDestroyed() && !bPrevClamp[i]) { Piles.Add(C); bReleased[i] = true; bChange = true; UE_LOG(LogTemp, Warning, TEXT("F2X P2 bracadeira %d destruida: lingote solto"), i); }
		bPrevClamp[i] = C && C->IsDestroyed();
	}
	if (!Chute->IsDestroyed() && bPrevChute) { bChange = true; UE_LOG(LogTemp, Warning, TEXT("F2X P2 calha reconstruida")); }
	bPrevChute = Chute->IsDestroyed();
	if (!Pin->IsDestroyed() && bPrevPin && !St.bDone)
	{
		St.P0 = PAt(Now()); St.T0 = Now(); St.Target = St.P0; St.bLocked = true;
		UE_LOG(LogTemp, Warning, TEXT("F2X P2 pino encaixado em P=%.1f, massas %d x %d"), St.P0, MassNorth(), MassSouth());
		if (MassNorth() == MassSouth() && FMath::Abs(St.P0) <= PinTol) Complete();
		ForceNetUpdate();
	}
	bPrevPin = Pin->IsDestroyed();
	if (bChange && !St.bLocked) FlowLingotes();
}

bool ACoopBalanceMachine::ServerReset(FString& OutWhy)
{
	if (!HasAuthority()) { OutWhy = TEXT("sem autoridade"); return false; }
	if (St.bDone) { OutWhy = TEXT("Mecanismo ja travado"); return false; }
	for (ACoopDestructible* D : { Brake, Clamp0, Clamp1, Clamp2 }) if (D) D->ServerForce(false, false);
	for (ACoopDestructible* D : { Chute, Pin }) if (D) { D->ServerForce(true, false); D->Partner = nullptr; }
	Piles.Reset();
	bPrevBrake = false; bPrevChute = true; bPrevPin = true;
	for (int32 i = 0; i < 3; ++i) { bPrevClamp[i] = false; bReleased[i] = false; SetLoc(i, 0); SetSSlot(i, 0); }
	St.P0 = PAt(Now()); St.T0 = Now(); St.Target = JamP; St.bReturning = true;   // volta devagar ate a posicao emperrada
	St.bBrakeFree = false; St.bLocked = false; St.Resets++;
	ForceNetUpdate();
	UE_LOG(LogTemp, Warning, TEXT("F2X P2 REARME: maquina voltou ao estado inicial (%d)"), St.Resets);
	return true;
}

void ACoopBalanceMachine::Tick(float Dt)
{
	Super::Tick(Dt);
	if (HasAuthority()) ServerTick();
	ApplyVisual(Dt);
}

// ------------------------------- visual (local em cada maquina) -------------------------------
static UStaticMeshComponent* MakeC(AActor* O, UStaticMesh* M, UMaterialInterface* Mat, bool bShadow = true)
{
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(O);
	C->SetupAttachment(O->GetRootComponent()); C->SetMobility(EComponentMobility::Movable);
	C->SetStaticMesh(M); C->SetMaterial(0, Mat); C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C->SetCanEverAffectNavigation(false); C->SetCastShadow(bShadow); C->RegisterComponent();
	return C;
}
static void Place(UStaticMeshComponent* C, FVector Center, FVector Size, FRotator R = FRotator::ZeroRotator) { C->SetWorldLocationAndRotation(Center, R); C->SetWorldScale3D(Size / 100.f); }

void ACoopBalanceMachine::Build()
{
	UStaticMesh* Cube = JJ::Mesh(TEXT("Cube")); UStaticMesh* Cyl = JJ::Mesh(TEXT("Cylinder"));
	UMaterialInterface* Steel = JJ::Mat(this, FLinearColor(0.09f, 0.09f, 0.1f), FLinearColor::Black, 0.45f, 0.8f);
	UMaterialInterface* Deck = JJ::Mat(this, FLinearColor(0.32f, 0.3f, 0.26f), FLinearColor::Black, 0.7f, 0.3f);
	UMaterialInterface* Haz = JJ::Mat(this, FLinearColor(0.85f, 0.62f, 0.05f), FLinearColor(0.85f, 0.62f, 0.05f) * 0.3f, 0.5f);
	UMaterialInterface* Red = JJ::Mat(this, FLinearColor(0.7f, 0.05f, 0.03f), FLinearColor(1.f, 0.1f, 0.05f) * 0.8f, 0.5f);
	UMaterialInterface* Cable = JJ::Mat(this, FLinearColor(0.03f, 0.03f, 0.03f), FLinearColor::Black, 0.6f, 0.5f);
	UMaterialInterface* Iron = JJ::Mat(this, FLinearColor(0.16f, 0.16f, 0.17f), FLinearColor::Black, 0.4f, 0.9f);
	UMaterialInterface* Weld = JJ::Mat(this, FLinearColor(0.3f, 0.12f, 0.03f), FLinearColor(1.f, 0.35f, 0.05f) * 1.2f, 0.5f);
	UMaterialInterface* Spark = JJ::Mat(this, FLinearColor(1.f, 0.6f, 0.2f), FLinearColor(1.f, 0.55f, 0.15f) * 8.f, 0.5f);
	UMaterialInterface* Dark = JJ::Mat(this, FLinearColor(0.02f, 0.02f, 0.02f), FLinearColor::Black, 0.9f);
	// colunas-guia com faixa de nivel (as duas salas): o ponteiro da plataforma encosta na faixa quando esta nivelada
	for (int32 s = 0; s < 2; ++s)
	{
		const float Sg = s == 0 ? 1.f : -1.f;
		for (float X : { 1230.f, 1670.f }) for (float Y : { 560.f, 960.f })
		{
			Place(MakeC(this, Cube, Steel), FVector(X, Sg * Y, 880.f), FVector(20, 20, 560));
			Place(MakeC(this, Cube, Haz, false), FVector(X, Sg * Y, LevelZ), FVector(24, 24, 14));
		}
		// polias no teto e eixo ate o hub
		for (float Y : { 560.f, 960.f }) Place(MakeC(this, Cyl, Iron), FVector(1450.f, Sg * Y, 1125.f), FVector(70, 70, 22), FRotator(0, 0, 90));
		Place(MakeC(this, Cube, Iron), FVector(1450.f, Sg * 640.f, 1140.f), FVector(16, 660, 16));
	}
	// plataformas (sem colisao; a area e cercada no mapa) com ponteiros vermelhos e a trava do pino (sul)
	auto Platform = [&](TArray<UStaticMeshComponent*>& Arr, TArray<FVector>& Rel, float Y, bool bSouth)
	{
		auto Add = [&](UMaterialInterface* M, FVector R, FVector Sz) { UStaticMeshComponent* C = MakeC(this, Cube, M); C->SetWorldScale3D(Sz / 100.f); Arr.Add(C); Rel.Add(R); };
		Add(Deck, FVector(GPlatN.X, Y, -15.f), FVector(400, 400, 30));
		Add(Steel, FVector(GPlatN.X, Y - 195.f, 0.f), FVector(400, 10, 40)); Add(Steel, FVector(GPlatN.X, Y + 195.f, 0.f), FVector(400, 10, 40));
		for (float X : { 1245.f, 1655.f }) for (float Dy : { -190.f, 190.f }) Add(Red, FVector(X, Y + Dy, 0.f), FVector(16, 16, 10));
		if (bSouth) { Add(Steel, FVector(1262.f, Y, -15.f), FVector(24, 60, 50)); Add(Dark, FVector(1250.f, Y, -15.f), FVector(4, 30, 30)); }
	};
	Platform(PlatN, PlatNRel, GPlatN.Y, false);
	Platform(PlatS, PlatSRel, GPlatS.Y, true);
	for (int32 k = 0; k < 4; ++k) { CablesN.Add(MakeC(this, Cube, Cable, false)); CablesS.Add(MakeC(this, Cube, Cable, false)); }
	// lingotes: 3 moveis + 1 soldado na plataforma sul (com cordao de solda)
	for (int32 i = 0; i < 4; ++i) { Ling.Add(MakeC(this, Cube, i == 3 ? Iron : Iron)); Ling.Last()->SetWorldScale3D(FVector(0.6f)); }
	UStaticMeshComponent* W = MakeC(this, Cube, Weld, false); W->SetWorldScale3D(FVector(0.64f, 0.64f, 0.05f)); Ling.Add(W);
	LingPos.SetNum(4);
	// berco de transferencia no hub (visto pelas janelas), calha norte fixa (entra pela parede)
	Place(MakeC(this, Cube, Steel), GCradle - FVector(0, 0, 40), FVector(120, 120, 16));
	for (float Sg : { -1.f, 1.f }) Place(MakeC(this, Cube, Steel), GCradle + FVector(0, Sg * 62.f, -20.f), FVector(120, 6, 40));
	Place(MakeC(this, Cube, Steel), FVector(1600, 440, 980), FVector(70, 240, 14), FRotator(0, 0, -12));
	Place(MakeC(this, Cube, Dark, false), FVector(1600, 321, 960), FVector(80, 4, 80));
	Place(MakeC(this, Cube, Dark, false), FVector(1600, -321, 960), FVector(80, 4, 80));
	// suporte do pino (sul) e da calha
	Place(MakeC(this, Cube, Steel), FVector(1180, -795, 880), FVector(40, 14, 560));
	// mostradores de equilibrio no hub (um virado para cada janela), faixa verde no centro
	DialMat = JJ::Mat(this, FLinearColor(0.85f, 0.85f, 0.8f), FLinearColor::Black, 0.6f);
	for (float Sg : { 1.f, -1.f })
	{
		Place(MakeC(this, Cyl, DialMat), FVector(1700, Sg * 200.f, 980), FVector(200, 200, 6), FRotator(0, 0, 90));
		Place(MakeC(this, Cube, JJ::Mat(this, FLinearColor(0.1f, 0.6f, 0.15f), FLinearColor(0.1f, 0.8f, 0.2f) * 0.6f, 0.5f), false), FVector(1700, Sg * 205.f, 1068), FVector(18, 4, 14));
		UStaticMeshComponent* N = MakeC(this, Cube, Red, false); (Sg > 0 ? NeedleN : NeedleS) = N;
	}
	// faiscas do freio emperrado
	for (int32 i = 0; i < 6; ++i) { Sparks.Add(MakeC(this, Cube, Spark, false)); Sparks.Last()->SetWorldScale3D(FVector(0.04f)); }
	for (float Sg : { 1.f, -1.f })
	{
		UPointLightComponent* L = NewObject<UPointLightComponent>(this); L->SetupAttachment(RootComponent); L->RegisterComponent();
		L->SetWorldLocation(FVector(1700, Sg * 380.f, 1100)); L->SetIntensityUnits(ELightUnits::Lumens); L->SetIntensity(1800.f);
		L->SetAttenuationRadius(700.f); L->SetCastShadows(false); StatusLights.Add(L);
	}
	bVisInit = true;
}

FVector ACoopBalanceMachine::LingTarget(int32 i, float P) const
{
	const float TopN = LevelZ + P, TopS = LevelZ - P;
	if (i == 3) return FVector(1350.f, -860.f, TopS + 30.f);
	switch (Loc(i))
	{
	case 1: return GCradle;
	case 2: { const uint8 S = SSlot(i); return S == 0 ? FVector(1450.f, -860.f, TopS + 30.f) : S == 1 ? FVector(1550.f, -860.f, TopS + 30.f) : FVector(1450.f, -760.f, TopS + 30.f); }
	default: return FVector(1350.f + 100.f * i, 870.f, TopN + 30.f);
	}
}

void ACoopBalanceMachine::ApplyVisual(float Dt)
{
	if (!bVisInit) return;
	const float T = Now(), P = PAt(T);
	const float TopN = LevelZ + P, TopS = LevelZ - P;
	for (int32 k = 0; k < PlatN.Num(); ++k) PlatN[k]->SetWorldLocation(PlatNRel[k] + FVector(0, 0, TopN));
	for (int32 k = 0; k < PlatS.Num(); ++k) PlatS[k]->SetWorldLocation(PlatSRel[k] + FVector(0, 0, TopS));
	int32 c = 0;
	for (float X : { 1260.f, 1640.f }) for (float Dy : { -180.f, 180.f })
	{
		const float LenN = 1160.f - TopN, LenS = 1160.f - TopS;
		Place(CablesN[c], FVector(X, GPlatN.Y + Dy, TopN + LenN * 0.5f), FVector(4, 4, LenN));
		Place(CablesS[c], FVector(X, GPlatS.Y + Dy, TopS + LenS * 0.5f), FVector(4, 4, LenS));
		++c;
	}
	// bracadeiras acompanham a plataforma norte (localmente em cada maquina)
	for (int32 i = 0; i < 3; ++i) if (ACoopDestructible* C = ClampOf(i)) C->SetActorLocation(FVector(1350.f + 100.f * i, 812.f, TopN + 40.f));
	// lingotes: posicao-alvo pelo estado replicado, movimento suave (desliza ate o berco/plataforma)
	if (LastResets != St.Resets) { LastResets = St.Resets; for (int32 i = 0; i < 4; ++i) LingPos[i] = LingTarget(i, P); }
	for (int32 i = 0; i < 4; ++i)
	{
		const FVector Tg = LingTarget(i, P);
		const bool bRide = (i == 3) || Loc(i) != 1 && FVector::Dist2D(LingPos[i], Tg) < 5.f;
		LingPos[i] = bRide ? Tg : FMath::VInterpConstantTo(LingPos[i], Tg, Dt, 420.f);
		Ling[i]->SetWorldLocation(LingPos[i]);
	}
	Ling[4]->SetWorldLocation(LingPos[3] - FVector(0, 0, 28.f));
	// mostradores: ponteiro gira com o deslocamento (centro = equilibrio)
	const float A = FMath::DegreesToRadians(P / Range * 60.f);
	for (int32 s = 0; s < 2; ++s)
	{
		const float Sg = s == 0 ? 1.f : -1.f; const float AA = s == 0 ? A : -A;
		UStaticMeshComponent* N = s == 0 ? NeedleN : NeedleS;
		const FVector Ctr(1700.f, Sg * 207.f, 980.f);
		Place(N, Ctr + FVector(FMath::Sin(AA) * 45.f, 0.f, FMath::Cos(AA) * 45.f), FVector(8, 4, 90), FRotator(-FMath::RadiansToDegrees(AA), 0, 0));
	}
	// faiscas no freio enquanto emperrado
	const bool bJam = Brake && !Brake->IsDestroyed();
	for (int32 i = 0; i < Sparks.Num(); ++i)
	{
		const float Ph = FMath::Frac(T * 1.7f + i * 0.37f);
		Sparks[i]->SetVisibility(bJam && Ph < 0.35f);
		Sparks[i]->SetWorldLocation(FVector(1400.f + i * 20.f, 470.f + FMath::Sin(i * 2.1f) * 30.f, 1040.f - Ph * 160.f));
	}
	for (UPointLightComponent* L : StatusLights) L->SetLightColor(St.bDone ? FLinearColor(0.2f, 1.f, 0.35f) : bJam ? FLinearColor(1.f, 0.25f, 0.1f) : FLinearColor(1.f, 0.7f, 0.2f));
}
