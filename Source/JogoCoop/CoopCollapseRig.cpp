#include "CoopCollapseRig.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Camera/CameraComponent.h"
#include "JJVisual.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

static UMaterialInterface* LoadM(const TCHAR* P) { return LoadObject<UMaterialInterface>(nullptr, P); }
static float H01(int32 i, int32 s) { return FMath::Frac(FMath::Sin(i * 12.9898f + s * 78.233f) * 43758.5453f); }   // pseudo-aleatorio deterministico

ACoopCollapseRig::ACoopCollapseRig()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Raiz"));
	LoseCam = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraDerrota"));
	LoseCam->SetupAttachment(RootComponent);
	LoseCam->SetFieldOfView(82.f);
}

void ACoopCollapseRig::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopCollapseRig, St);
}

float ACoopCollapseRig::Now() const
{
	const AGameStateBase* GS = GetWorld()->GetGameState();
	return GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}

void ACoopCollapseRig::ServerBeginInstability(float Duration) { if (!HasAuthority()) return; St.Stage = 1; St.T0 = Now(); St.Dur = Duration; ForceNetUpdate(); }
void ACoopCollapseRig::ServerStabilize() { if (!HasAuthority()) return; St.StressAtT1 = Stress(Now()); St.Stage = 3; St.T1 = Now(); ForceNetUpdate(); }
void ACoopCollapseRig::ServerBeginFall() { if (!HasAuthority()) return; St.StressAtT1 = 1.f; St.Stage = 2; St.T1 = Now() + Lead; ForceNetUpdate(); }
void ACoopCollapseRig::ServerReset() { if (!HasAuthority()) return; St.Stage = 0; ForceNetUpdate(); }
float ACoopCollapseRig::BayFallTime() const { return FMath::Sqrt(2.f * FMath::Max(BayZ - (FloorZ + RestH), 50.f) / 980.f); }

UStaticMeshComponent* ACoopCollapseRig::Add(int32 Group, UStaticMesh* M, UMaterialInterface* Mat, FVector Loc, FVector Size, FRotator Rot, bool bModelBase)
{
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
	C->SetupAttachment(RootComponent); C->SetMobility(EComponentMobility::Movable);
	C->SetStaticMesh(M); if (Mat) C->SetMaterial(0, Mat);
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision); C->SetCanEverAffectNavigation(false);
	C->RegisterComponent();
	FPiece Pc; Pc.C = C; Pc.Group = Group; Pc.Rest = Loc; Pc.RestRot = Rot;
	Pc.RestScale = bModelBase ? Size : Size / 100.f;   // cubo da engine = 100 cm
	Pc.Phase = P.Num() * 1.37f;
	C->SetWorldLocationAndRotation(Loc, Rot); C->SetWorldScale3D(Pc.RestScale);
	P.Add(Pc);
	return C;
}

void ACoopCollapseRig::Build()
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Sph = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMesh* Panel = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Fase1/Teto_Painel/SM_Teto_Painel.SM_Teto_Painel"));
	UMaterialInterface *Steel = LoadM(TEXT("/Game/Fase1Arq/HF/M_steel_dark.M_steel_dark")), *Paint = LoadM(TEXT("/Game/Fase1Arq/HF/M_steel_paint.M_steel_paint")),
		*Conc = LoadM(TEXT("/Game/Fase1Arq/HF/M_concrete_dark.M_concrete_dark")), *Haz = LoadM(TEXT("/Game/Fase1Arq/HF/M_hazard_yellow.M_hazard_yellow")),
		*Ember = LoadM(TEXT("/Game/Fase1Arq/HF/M_ember.M_ember")), *Cable = LoadM(TEXT("/Game/Fase1Arq/HF/M_cable.M_cable")), *DustM = LoadM(TEXT("/Game/Fase2/Materiais/M_F2_Barreira.M_F2_Barreira"));
	const FVector C = GetActorLocation();
	const float TopZ = C.Z + ColumnTopZ;
	// 1) deck do mezanino (paineis do Blender) + moldura de vigas + capitel sobre a coluna
	const int32 NX = 2, NY = 2;
	for (int32 ix = 0; ix < NX; ++ix) for (int32 iy = 0; iy < NY; ++iy)
	{
		const float W = (DeckMax.X - DeckMin.X) / NX, Hh = (DeckMax.Y - DeckMin.Y) / NY;
		const FVector L(DeckMin.X + (ix + 0.5f) * W, DeckMin.Y + (iy + 0.5f) * Hh, TopZ);
		if (Panel) Add(1, Panel, nullptr, L, FVector(W / 583.5f, Hh / 540.f, 1.f), FRotator::ZeroRotator, true);
		else Add(1, Cube, Conc, L + FVector(0, 0, 40), FVector(W, Hh, 80));
	}
	const FVector DC((DeckMin.X + DeckMax.X) * 0.5f, (DeckMin.Y + DeckMax.Y) * 0.5f, TopZ);
	const float DW = DeckMax.X - DeckMin.X, DH = DeckMax.Y - DeckMin.Y;
	Add(1, Cube, Steel, FVector(DC.X, DeckMin.Y, TopZ - 20), FVector(DW + 40, 40, 50));
	Add(1, Cube, Steel, FVector(DC.X, DeckMax.Y, TopZ - 20), FVector(DW + 40, 40, 50));
	Add(1, Cube, Steel, FVector(DeckMin.X, DC.Y, TopZ - 20), FVector(40, DH, 50));
	Add(1, Cube, Steel, FVector(DeckMax.X, DC.Y, TopZ - 20), FVector(40, DH, 50));
	Add(1, Cube, Steel, FVector(C.X, DC.Y, TopZ - 25), FVector(50, DH, 60));          // viga principal que apoia na coluna
	Add(1, Cube, Paint, FVector(C.X, C.Y, TopZ - 10), FVector(320, 320, 30));         // capitel (chapa de apoio)
	Add(1, Cube, Haz, FVector(C.X, C.Y, TopZ - 28), FVector(330, 330, 6));
	// 2) torre de transferencia: 4 montantes da coluna ate a secao do teto, com contraventamento em X
	const float T0z = TopZ + 80.f, T1z = BayZ;
	const float Hs = 115.f;
	for (int32 k = 0; k < 4; ++k)
	{
		const float sx = (k & 1) ? 1.f : -1.f, sy = (k & 2) ? 1.f : -1.f;
		Add(2, Cube, Steel, FVector(C.X + sx * Hs, C.Y + sy * Hs, (T0z + T1z) * 0.5f), FVector(34, 34, T1z - T0z));
		Joints.Add(FVector(C.X + sx * Hs, C.Y + sy * Hs, T1z - 10.f));
	}
	const float Hh = T1z - T0z, Dg = FMath::Sqrt(Hh * Hh + 4 * Hs * Hs), Ang = FMath::RadiansToDegrees(FMath::Atan2(Hh, 2 * Hs));
	for (int32 f = 0; f < 4; ++f)
		for (int32 d = 0; d < 2; ++d)
		{
			const bool bX = f < 2; const float sgn = (f % 2) ? 1.f : -1.f; const float flip = d ? 1.f : -1.f;
			const FVector L = bX ? FVector(C.X, C.Y + sgn * Hs, (T0z + T1z) * 0.5f) : FVector(C.X + sgn * Hs, C.Y, (T0z + T1z) * 0.5f);
			const FRotator R = bX ? FRotator(flip * Ang, 0, 0) : FRotator(flip * Ang, 90, 0);
			Add(2, Cube, Paint, L, FVector(Dg, 14, 14), R);
		}
	// 3) secao pesada do teto sustentada (laje + vigas por baixo) e ligacao com as trelicas principais
	const FVector BC((BayMin.X + BayMax.X) * 0.5f, (BayMin.Y + BayMax.Y) * 0.5f, BayZ);
	const float BW = BayMax.X - BayMin.X, BH = BayMax.Y - BayMin.Y;
	Add(3, Cube, Conc, BC + FVector(0, 0, BayThick * 0.5f), FVector(BW, BH, BayThick));
	// viga de transferencia: atravessa o galpao de parede a parede e apoia na torre/coluna (faixa de perigo = peca critica)
	Add(3, Cube, Paint, FVector(C.X, BC.Y, BayZ - 65), FVector(95, BH, 130));
	for (float sx : { -1.f, 1.f }) Add(3, Cube, Haz, FVector(C.X + sx * 49.f, BC.Y, BayZ - 65), FVector(4, BH, 22));
	for (float X : { BayMin.X + 90.f, (BayMin.X + C.X) * 0.5f, BayMax.X - 90.f }) Add(3, Cube, Steel, FVector(X, BC.Y, BayZ - 35), FVector(55, BH, 70));      // vigas principais
	const int32 NT = FMath::Max(5, FMath::RoundToInt(BH / 350.f));
	for (int32 i = 0; i < NT; ++i) Add(3, Cube, Paint, FVector(BC.X, BayMin.Y + (i + 0.5f) * BH / NT, BayZ - 22), FVector(BW, 30, 44));   // travessas
	for (int32 k = 0; k < 8; ++k)                                                                                                     // montantes ate a trelica
	{
		const float X = (k < 4) ? BayMin.X + 60 : BayMax.X - 60, Y = BayMin.Y + 120 + (k % 4) * (BH - 240) / 3.f;
		Add(3, Cube, Steel, FVector(X, Y, (BayZ + BayThick + TrussZ) * 0.5f), FVector(30, 30, TrussZ - BayZ - BayThick));
		Joints.Add(FVector(X, Y, TrussZ - 5.f));
	}
	for (int32 k = 0; k < 4; ++k)                                                                                                     // tirantes secao -> deck
	{
		const float X = (k & 1) ? DeckMax.X - 20 : DeckMin.X + 20, Y = (k & 2) ? DeckMax.Y - 20 : DeckMin.Y + 20;
		Add(3, Cube, Cable, FVector(X, Y, (TopZ + 80 + BayZ) * 0.5f), FVector(8, 8, BayZ - TopZ - 80));
		Joints.Add(FVector(X, Y, BayZ - 15.f));
	}
	// mao-francesa: do topo da torre ate as vigas principais
	for (float sx : { -1.f, 1.f }) Add(3, Cube, Paint, FVector(C.X + sx * 260.f, C.Y, BayZ - 150.f), FVector(400, 22, 22), FRotator(sx * 35.f, 0, 0));
	// 4) consoles de ancoragem na parede (ficam; soltam faiscas)
	for (float Wy : { WallY, -WallY })
		for (float X : { BayMin.X + 150.f, BC.X, BayMax.X - 150.f })
		{
			const float s = Wy > 0.f ? -1.f : 1.f;
			Add(0, Cube, Steel, FVector(X, Wy + s * 30.f, BayZ - 60.f), FVector(80, 60, 120));
			Joints.Add(FVector(X, Wy + s * 40.f, BayZ - 10.f));
		}
	// luminarias penduradas na secao (iluminam a area de inicio por baixo dela; caem junto)
	{
		UMaterialInterface* LampM = JJ::Mat(this, FLinearColor(0.8f, 0.85f, 0.9f), FLinearColor(0.72f, 0.86f, 1.f) * 6.f, 0.5f);
		for (int32 k = 0; k < 6; ++k)
		{
			const float X = (k < 3) ? (BayMin.X + C.X) * 0.5f : (C.X + BayMax.X) * 0.5f + 40.f, Y = BayMin.Y + (0.5f + (k % 3)) * BH / 3.f;
			Add(3, Cube, Cable, FVector(X, Y, BayZ - 75.f), FVector(6, 6, 110));
			Add(3, Cube, Steel, FVector(X, Y, BayZ - 140.f), FVector(110, 50, 22));
			const int32 Face = P.Num(); Add(3, Cube, LampM, FVector(X, Y, BayZ - 152.f), FVector(96, 38, 4));
			USpotLightComponent* L = NewObject<USpotLightComponent>(this); L->SetupAttachment(RootComponent); L->RegisterComponent();
			L->SetWorldRotation(FRotator(-90, 0, 0)); L->SetLightColor(FLinearColor(0.72f, 0.86f, 1.f)); L->SetIntensityUnits(ELightUnits::Lumens);
			L->SetIntensity(9000.f); L->SetAttenuationRadius(2200.f); L->SetOuterConeAngle(62.f); L->SetInnerConeAngle(30.f); L->SetCastShadows(k % 2 == 0);
			BayLamps.Add(L); BayLampPiece.Add(Face);
		}
	}
	// 2a onda: trelicas/vigas do vao central que despencam logo depois (o colapso se espalha alem da secao)
	for (int32 i = 0; i < 7; ++i)
	{
		const float X = 1350.f + i * 210.f, Y = (i % 2 ? 1.f : -1.f) * (500.f + H01(i, 41) * 700.f);
		Add(6, Cube, i % 3 == 0 ? Paint : Steel, FVector(X, Y, TrussZ - 40.f), FVector(i % 3 == 0 ? 520.f : 760.f, 40, 55), FRotator(0, 90.f * (i % 2) + H01(i, 43) * 30.f, 0));
		P.Last().Detach = Lead + 0.25f + i * 0.13f;
	}
	// 5) pecas secundarias que se soltam antes do colapso (chapas do deck, travessa, bandeja de cabos, luminarias)
	const float DetT[6] = { 0.42f, 0.55f, 0.66f, 0.76f, 0.85f, 0.93f };
	for (int32 i = 0; i < 6; ++i)
	{
		const float X = BayMin.X + 200 + H01(i, 3) * (BW - 400), Y = BayMin.Y + 200 + H01(i, 7) * (BH - 400);
		const bool bBig = i % 2 == 0;
		UStaticMeshComponent* S = Add(4, Cube, bBig ? Paint : Steel, FVector(X, Y, BayZ - (bBig ? 60.f : 90.f)), bBig ? FVector(380, 40, 45) : FVector(180, 90, 26), FRotator(0, H01(i, 9) * 180.f, 0));
		P.Last().Detach = DetT[i];
		P.Last().Land = FVector(X + (H01(i, 4) - 0.5f) * 120.f, Y + (H01(i, 5) - 0.5f) * 120.f, FloorZ + 15.f);
		(void)S;
	}
	// 6) rachaduras (emissivas) nas zonas de tensao: capitel, base dos montantes, vigas, laje
	for (int32 i = 0; i < 18; ++i)
	{
		FVector L; FRotator R(0, H01(i, 1) * 180.f, 0);
		if (i < 6) L = FVector(C.X + (H01(i, 2) - 0.5f) * 280.f, C.Y + (H01(i, 6) - 0.5f) * 280.f, TopZ - 26.f);
		else if (i < 12) L = Joints[i % Joints.Num()] - FVector(0, 0, 40.f + H01(i, 8) * 60.f), R.Pitch = 70.f;
		else L = FVector(BayMin.X + H01(i, 2) * BW, BayMin.Y + H01(i, 6) * BH, BayZ - 1.f);
		Add(5, Cube, Ember, L, FVector(140.f + H01(i, 3) * 160.f, 7, 5), R);
		P.Last().Detach = 0.03f + (i / 18.f) * 0.9f;   // ordem de aparicao
	}
	Pivot = FVector(DC.X, DC.Y, FloorZ);
	// 7) particulas leves (instancias): poeira, faiscas e detritos
	UMaterialInterface* DustMat = DustM;
	if (DustM) { UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(DustM, this); M->SetVectorParameterValue(TEXT("Cor"), FLinearColor(0.09f, 0.075f, 0.06f)); M->SetScalarParameterValue(TEXT("Opac"), 0.16f); DustMat = M; }
	auto Pool = [&](TArray<UStaticMeshComponent*>& Arr, int32 N, UStaticMesh* M, UMaterialInterface* Mat, bool bShadow)
	{
		for (int32 i = 0; i < N; ++i)
		{
			UStaticMeshComponent* Q = NewObject<UStaticMeshComponent>(this); Q->SetupAttachment(RootComponent); Q->SetMobility(EComponentMobility::Movable);
			Q->SetStaticMesh(M); if (Mat) Q->SetMaterial(0, Mat); Q->SetCollisionEnabled(ECollisionEnabled::NoCollision); Q->SetCastShadow(bShadow);
			Q->SetCanEverAffectNavigation(false); Q->SetVisibility(false); Q->RegisterComponent(); Arr.Add(Q);
		}
	};
	Pool(DustC, 110, Sph, DustMat, false); Pool(SparkC, 40, Cube, Ember, false); Pool(DebrisC, 36, Cube, Conc, true);
	for (int32 i = 0; i < 36; ++i)
	{
		DebrisFrom.Add(FVector(BayMin.X + 100 + H01(i, 11) * (BW - 200), BayMin.Y + 100 + H01(i, 13) * (BH - 200), BayZ - 5.f));
		DebrisAt.Add(FMath::Pow((i + 1) / 37.f, 0.75f));    // mais detritos perto do fim
		DebrisLandZ.Add(FloorZ + 8.f);
	}
	SparkLight = NewObject<UPointLightComponent>(this); SparkLight->SetupAttachment(RootComponent); SparkLight->RegisterComponent();
	SparkLight->SetLightColor(FLinearColor(1.f, 0.55f, 0.15f)); SparkLight->SetAttenuationRadius(700.f); SparkLight->SetCastShadows(false); SparkLight->SetIntensity(0.f);
	ImpactLight = NewObject<UPointLightComponent>(this); ImpactLight->SetupAttachment(RootComponent); ImpactLight->RegisterComponent();
	ImpactLight->SetLightColor(FLinearColor(1.f, 0.75f, 0.5f)); ImpactLight->SetAttenuationRadius(2500.f); ImpactLight->SetCastShadows(false); ImpactLight->SetIntensity(0.f);
	ImpactLight->SetWorldLocation(FVector(DC.X, DC.Y, FloorZ + 250.f));
	// luz de servico fria na estrutura (deixa ler torre, vigas e a secao do teto); vira aviso vermelho pulsante na instabilidade
	for (int32 k = 0; k < 3; ++k)
	{
		UPointLightComponent* L = NewObject<UPointLightComponent>(this); L->SetupAttachment(RootComponent); L->RegisterComponent();
		const FVector Lp = k == 0 ? FVector(C.X + 250.f, C.Y + 350.f, (TopZ + BayZ) * 0.5f) : k == 1 ? FVector(BC.X, BC.Y + 300.f, BayZ - 180.f) : FVector(BayMin.X + 200.f, BC.Y, BayZ - 200.f);
		L->SetWorldLocation(Lp); L->SetLightColor(FLinearColor(0.75f, 0.86f, 1.f)); L->SetIntensityUnits(ELightUnits::Lumens); L->SetIntensity(12000.f);
		L->SetAttenuationRadius(1400.f); L->SetCastShadows(false); ServiceLights.Add(L);
	}
}

void ACoopCollapseRig::BeginPlay()
{
	Super::BeginPlay();
	Build();
	CamRest = LoseCamPos; CamRot = (LoseCamTarget - LoseCamPos).Rotation();
	LoseCam->SetWorldLocationAndRotation(CamRest, CamRot);
}

void ACoopCollapseRig::SetP(UStaticMeshComponent* C, const FTransform& X)
{
	const bool bV = !X.GetScale3D().IsNearlyZero(1e-3f);
	if (bV) C->SetWorldTransform(X);
	if (C->IsVisible() != bV) C->SetVisibility(bV);
}

float ACoopCollapseRig::Stress(float T) const
{
	switch (St.Stage)
	{
	case 1: return FMath::Clamp((T - St.T0) / FMath::Max(St.Dur, 1.f), 0.f, 1.f);
	case 2: return 1.f;
	case 3: return St.StressAtT1 * FMath::Clamp(1.f - (T - St.T1) / 1.2f, 0.f, 1.f);
	default: return 0.f;
	}
}

void ACoopCollapseRig::Tick(float Dt)
{
	Super::Tick(Dt);
	if (P.Num()) ApplyVisual(Now(), Dt);
}

void ACoopCollapseRig::ApplyVisual(float T, float Dt)
{
	const float S = Stress(T);
	const bool bFall = St.Stage == 2;
	const float Tf = bFall ? FMath::Max(T - St.T1, 0.f) : 0.f;
	const float BayT = BayFallTime();
	const float DeckDrop0 = ColumnTopZ - 40.f + (GetActorLocation().Z - FloorZ);   // deck ate o piso
	const float DeckDrop = bFall ? FMath::Min(0.5f * 980.f * Tf * Tf, DeckDrop0) : 0.f;
	const float BayDrop = bFall ? FMath::Min(0.5f * 980.f * Tf * Tf, BayZ - (FloorZ + RestH)) : 0.f;
	const bool bImpact = bFall && Tf >= BayT;
	// cedencia (sem a coluna o deck afunda e inclina) + vibracao crescente
	const float Sag = (St.Stage == 1 || St.Stage == 3) ? 40.f * S * S : bFall ? 40.f : 0.f;
	const float TiltSag = 3.f * S;
	const float Amp = (St.Stage == 1 || St.Stage == 3) ? (0.4f + 5.f * S * S) * (S > 0.f ? 1.f : 0.f) : 0.f;
	const float TiltFall = bFall ? FMath::Min(Tf / FMath::Max(BayT, 0.1f), 1.f) * 6.f : 0.f;
	for (FPiece& Pc : P)
	{
		FVector L = Pc.Rest; FRotator R = Pc.RestRot; FVector Sc = Pc.RestScale; bool bVis = true;
		const FVector Jit = FVector(FMath::Sin(T * 37.f + Pc.Phase), FMath::Cos(T * 41.f + Pc.Phase * 1.7f), FMath::Sin(T * 29.f + Pc.Phase * 0.6f)) * Amp;
		auto Tilt = [&](float Deg, float Drop)
		{
			const FVector Rel = L - Pivot; const FQuat Q(FVector(0, 1, 0), FMath::DegreesToRadians(Deg));
			L = Pivot + Q.RotateVector(Rel) - FVector(0, 0, Drop); R = (Q * R.Quaternion()).Rotator();
		};
		switch (Pc.Group)
		{
		case 1: L += Jit - FVector(0, 0, Sag); Tilt(TiltSag + TiltFall, DeckDrop); break;
		case 2:   // torre: acompanha o deck embaixo e a secao em cima (comprime na queda)
		{
			L += Jit * 0.8f;
			if (bFall) { const float Bot = Pc.Rest.Z - Pc.RestScale.Z * 50.f - DeckDrop, Top = Pc.Rest.Z + Pc.RestScale.Z * 50.f - BayDrop; const float Hn = FMath::Max(Top - Bot, 20.f); L.Z = (Bot + Top) * 0.5f; Sc.Z = Pc.RestScale.Z * Hn / FMath::Max(Pc.RestScale.Z * 100.f, 1.f); if (Pc.RestRot.Pitch != 0.f) R.Pitch = Pc.RestRot.Pitch * FMath::Clamp(Hn / (Pc.RestScale.X * 100.f), 0.1f, 1.f); Tilt(TiltFall * 0.5f, 0.f); }
			break;
		}
		case 3: L += Jit * 0.6f - FVector(0, 0, Sag * 0.35f); Tilt(TiltFall * 0.6f, BayDrop); break;
		case 4:   // secundaria: solta no seu instante e cai ate o piso, girando
		{
			const float tD = (St.Stage == 1) ? (T - St.T0) - Pc.Detach * St.Dur : (St.Stage == 3 || bFall) ? 9999.f : -1.f;
			const bool bGone = (St.Stage == 1 && tD > 0.f) || ((St.Stage == 3 || bFall) && PrevStress >= Pc.Detach);
			if (St.Stage == 1 && tD > 0.f)
			{
				const float Dz = Pc.Rest.Z - Pc.Land.Z, tl = FMath::Sqrt(2.f * Dz / 980.f), a = FMath::Clamp(tD / tl, 0.f, 1.f);
				L = FMath::Lerp(Pc.Rest, Pc.Land, a); L.Z = Pc.Rest.Z - FMath::Min(0.5f * 980.f * tD * tD, Dz);
				R = Pc.RestRot + FRotator(a * 70.f, a * 40.f, a * 25.f);
			}
			else if (bGone) { L = Pc.Land; R = Pc.RestRot + FRotator(70.f, 40.f, 25.f); }
			else if (bFall) Tilt(TiltFall * 0.6f, BayDrop);
			else { L += Jit * 1.4f; Tilt(TiltSag, 0.f); }
			break;
		}
		case 5:   // rachadura: aparece e cresce conforme a tensao
		{
			const float G = FMath::Clamp((S - Pc.Detach) / 0.25f, 0.f, 1.f);
			bVis = G > 0.01f && !bFall; Sc.X = Pc.RestScale.X * (0.2f + 0.8f * G);
			L += Jit - FVector(0, 0, Sag);
			break;
		}
		case 6:   // trelica do vao: cai pouco depois da secao, girando, ate sumir no poco
		{
			L += Jit * 0.5f;
			if (bFall && Tf > Pc.Detach)
			{
				const float tt = Tf - Pc.Detach; L.Z -= 0.5f * 980.f * tt * tt; R += FRotator(tt * 55.f, 0, tt * 35.f * (Pc.Phase > 5.f ? 1.f : -1.f));
				if (L.Z < -1400.f || (L.X > 2800.f && L.Z < FloorZ + 30.f)) { L.Z = FMath::Max(L.Z, FloorZ + 30.f); bVis = L.X > 2800.f; }
			}
			break;
		}
		default: L += Jit * 0.3f; break;
		}
		Pc.C->SetWorldLocationAndRotation(L, R); Pc.C->SetWorldScale3D(Sc); Pc.C->SetVisibility(bVis);
	}
	// detritos pequenos caindo da secao (agendados ao longo da contagem; ficam no chao ate o reinicio)
	for (int32 i = 0; i < DebrisFrom.Num(); ++i)
	{
		const float tStart = St.T0 + DebrisAt[i] * St.Dur;
		const bool bActive = St.Stage == 1 || St.Stage == 2 || St.Stage == 3;
		const float tD = T - tStart;
		FTransform X(FQuat::Identity, FVector::ZeroVector, FVector(0.f));
		const bool bLaunched = (St.Stage == 1) ? tD > 0.f : (PrevStress >= DebrisAt[i]);   // apos estabilizar/desabar, so os que ja tinham caido
		if (bActive && bLaunched)
		{
			const float Dz = DebrisFrom[i].Z - DebrisLandZ[i]; const float tt = FMath::Max(tD, 0.f);
			const float z = DebrisFrom[i].Z - FMath::Min(0.5f * 980.f * tt * tt, Dz);
			const float s = 0.12f + H01(i, 17) * 0.22f;
			X = FTransform(FRotator(tt * 300.f, tt * 200.f, 0).Quaternion(), FVector(DebrisFrom[i].X, DebrisFrom[i].Y, z), FVector(s, s * 0.8f, s * 0.6f));
		}
		SetP(DebrisC[i], X);
	}
	// poeira: escorre das juncoes (intensidade = tensao); no impacto vira nuvem no chao
	const float DustRate = bImpact ? 1.f : S;
	for (int32 i = 0; i < DustC.Num(); ++i)
	{
		FTransform X(FQuat::Identity, FVector::ZeroVector, FVector(0.f));
		const float Life = 2.6f, ph = FMath::Frac((T + H01(i, 21) * Life) / Life), Age = ph * Life;
		if (bImpact)
		{
			// nuvem de impacto: nasce sob toda a secao e se espalha (cobre os soterrados)
			const float tI = Tf - BayT;
			const FVector Base(FMath::Lerp(BayMin.X, BayMax.X, H01(i, 23)), FMath::Lerp(BayMin.Y, BayMax.Y, H01(i, 24)), FloorZ);
			const FVector Out = (Base - FVector((BayMin.X + BayMax.X) * 0.5f, (BayMin.Y + BayMax.Y) * 0.5f, FloorZ)).GetSafeNormal2D();
			const float s = (1.2f + tI * 1.6f) * (0.6f + H01(i, 27)) * FMath::Clamp(1.3f - tI / 3.2f, 0.f, 1.f);
			X = FTransform(FQuat::Identity, Base + Out * tI * 260.f * (0.4f + H01(i, 26)) + FVector(0, 0, 60.f + tI * 140.f * H01(i, 29)), FVector(s));
		}
		else if (H01(i, 31) < DustRate * 0.95f && Joints.Num() && (St.Stage == 1 || St.Stage == 3))
		{
			const FVector J = Joints[i % Joints.Num()];
			const float s = (0.06f + Age * 0.22f) * (0.4f + S);
			X = FTransform(FQuat::Identity, J + FVector(FMath::Sin(Age * 2.f + i) * 30.f, FMath::Cos(Age * 1.7f + i) * 30.f, -Age * 70.f), FVector(s));
		}
		SetP(DustC[i], X);
	}
	// faiscas: rajadas curtas nas conexoes, mais frequentes com a tensao
	float LightI = 0.f; FVector LightP = FVector::ZeroVector;
	for (int32 i = 0; i < 40; ++i)
	{
		FTransform X(FQuat::Identity, FVector::ZeroVector, FVector(0.f));
		const float Period = FMath::Lerp(2.2f, 0.45f, S), Burst = FMath::FloorToFloat((T + H01(i / 8, 33) * Period) / Period);
		const float tb = (T + H01(i / 8, 33) * Period) - Burst * Period;
		if ((St.Stage == 1 && S > 0.08f) || (bFall && !bImpact))
		{
			const int32 j = (int32)(H01((int32)Burst, i / 8) * Joints.Num()) % FMath::Max(Joints.Num(), 1);
			if (tb < 0.7f && Joints.Num())
			{
				const FVector V(FMath::Cos(H01(i, 35) * 6.283f) * 260.f, FMath::Sin(H01(i, 35) * 6.283f) * 260.f, 120.f + H01(i, 37) * 200.f);
				const FVector Pp = Joints[j] + V * tb - FVector(0, 0, 490.f * tb * tb);
				X = FTransform(FQuat::Identity, Pp, FVector(0.05f * (1.f - tb / 0.7f) + 0.01f));
				if (tb < 0.15f) { LightI = FMath::Max(LightI, 4500.f * (1.f - tb / 0.15f)); LightP = Joints[j]; }
			}
		}
		SetP(SparkC[i], X);
	}
	SparkLight->SetIntensity(LightI); if (LightI > 0.f) SparkLight->SetWorldLocation(LightP);
	ImpactLight->SetIntensity(bImpact ? 60000.f * FMath::Clamp(1.f - (Tf - BayT) / 0.8f, 0.f, 1.f) : 0.f);
	// tremor da camera local (somente visual) perto da estrutura
	const float KImp = bImpact ? 1.6f * FMath::Clamp(1.f - (Tf - BayT) / 0.9f, 0.f, 1.f) : 0.f;
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		if (PC->IsLocalController() && PC->GetPawn() && PC->GetViewTarget() == PC->GetPawn())
		{
			const float Dist = FVector::Dist(PC->GetPawn()->GetActorLocation(), Pivot);
			float K = (St.Stage == 1 ? 0.05f + 0.35f * S * S : 0.f) + KImp + (bFall && !bImpact ? 0.4f : 0.f);
			K *= FMath::Clamp(1.f - Dist / 4000.f, 0.25f, 1.f);
			if (K > 0.f) { static int32 Flip = 1; Flip = -Flip; PC->AddPitchInput(Flip * K * 0.35f); PC->AddYawInput(Flip * K * 0.25f); }
		}
	// camera de derrota: tremor na queda e forte no impacto
	{
		const float K = bFall ? (bImpact ? KImp * 18.f + 2.f : 5.f) : 0.f;
		LoseCam->SetWorldLocationAndRotation(CamRest + FVector(FMath::Sin(T * 61.f), FMath::Cos(T * 53.f), FMath::Sin(T * 47.f + 1.f)) * K,
			CamRot + FRotator(FMath::Sin(T * 43.f) * K * 0.05f, FMath::Cos(T * 39.f) * K * 0.05f, 0));
	}
	for (int32 k = 0; k < BayLamps.Num(); ++k)   // luminarias da secao: seguem a peca, piscam com a tensao, apagam no impacto
	{
		const UStaticMeshComponent* F = P[BayLampPiece[k]].C;
		BayLamps[k]->SetWorldLocationAndRotation(F->GetComponentLocation() - FVector(0, 0, 8.f), (F->GetComponentQuat() * FRotator(-90, 0, 0).Quaternion()).Rotator());
		const float Fl = (St.Stage == 1) ? (0.6f + 0.4f * FMath::Sin(T * (6.f + 18.f * S) + k * 1.7f)) : bFall ? (0.3f + 0.7f * FMath::Abs(FMath::Sin(T * 31.f + k))) : 1.f;
		BayLamps[k]->SetIntensity(bImpact ? 0.f : 9000.f * FMath::Lerp(1.f, Fl, FMath::Max(S, bFall ? 1.f : 0.f)));
		BayLamps[k]->SetLightColor(FMath::Lerp(FLinearColor(0.72f, 0.86f, 1.f), FLinearColor(1.f, 0.42f, 0.18f), (St.Stage == 1 || bFall) ? FMath::Min(S * 0.8f, 0.6f) : 0.f));
	}
	for (UPointLightComponent* L : ServiceLights)   // pisca e avermelha com a tensao; apaga quando desaba
	{
		const float Fl = (St.Stage == 1) ? (0.55f + 0.45f * FMath::Sin(T * (8.f + 20.f * S))) : 1.f;
		L->SetIntensity(bImpact ? 0.f : bFall ? 3000.f : 12000.f * FMath::Lerp(1.f, Fl, S));
		L->SetLightColor(FMath::Lerp(FLinearColor(0.75f, 0.86f, 1.f), FLinearColor(1.f, 0.38f, 0.15f), (St.Stage == 1 || bFall) ? FMath::Min(S, 0.7f) : 0.f));
	}
	PrevStress = (St.Stage == 1) ? S : PrevStress;
	if (St.Stage == 0) PrevStress = 0.f;
}
