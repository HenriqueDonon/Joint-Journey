#include "CoopDressing.h"
#include "JJVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "Math/RandomStream.h"

ACoopDressing::ACoopDressing()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ACoopDressing::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopDressing, Seed);
	DOREPLIFETIME(ACoopDressing, Variant);
}

void ACoopDressing::BeginPlay() { Super::BeginPlay(); Build(); }

void ACoopDressing::Build()
{
	FRandomStream R(Seed);
	UStaticMesh* Cube = JJ::Mesh(TEXT("Cube"));
	UStaticMesh* Cyl  = JJ::Mesh(TEXT("Cylinder"));
	UStaticMesh* Cone = JJ::Mesh(TEXT("Cone"));
	const FLinearColor Black = FLinearColor::Black;
	const FLinearColor Green(0.05f, 1.f, 0.25f), Purple(0.55f, 0.05f, 1.f);

	UMaterialInstanceDynamic* mStain  = JJ::Mat(this, FLinearColor(0.34f, 0.32f, 0.28f), Black, 1.f);
	UMaterialInstanceDynamic* mDark   = JJ::Mat(this, FLinearColor(0.15f, 0.14f, 0.13f), Black, 1.f);
	UMaterialInstanceDynamic* mLight  = JJ::Mat(this, FLinearColor(0.95f, 0.94f, 0.92f), Black, 1.f);
	UMaterialInstanceDynamic* mCrack  = JJ::Mat(this, FLinearColor(0.02f, 0.02f, 0.02f), Black, 1.f);
	UMaterialInstanceDynamic* mGrime  = JJ::Mat(this, FLinearColor(0.40f, 0.38f, 0.35f), Black, 1.f);
	UMaterialInstanceDynamic* mGreen  = JJ::Mat(this, FLinearColor(0.f, 0.03f, 0.f), Green * 4.f, 0.3f);
	UMaterialInstanceDynamic* mPurple = JJ::Mat(this, FLinearColor(0.03f, 0.f, 0.05f), Purple * 4.f, 0.3f);
	UMaterialInstanceDynamic* mGlass  = JJ::Mat(this, FLinearColor(0.01f, 0.01f, 0.03f), Black, 0.05f, 0.9f);
	UMaterialInstanceDynamic* mConc   = JJ::Mat(this, FLinearColor(0.36f, 0.36f, 0.37f), Black, 1.f);
	UMaterialInstanceDynamic* mConcD  = JJ::Mat(this, FLinearColor(0.20f, 0.20f, 0.21f), Black, 1.f);
	UMaterialInstanceDynamic* mCryst  = JJ::Mat(this, FLinearColor(0.15f, 0.02f, 0.30f), Purple * 3.5f, 0.1f, 0.2f);

	auto Add = [&](UStaticMesh* M, FVector L, FVector S, FRotator Rot, UMaterialInstanceDynamic* Mat, USceneComponent* Par = nullptr) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetStaticMesh(M);
		C->SetupAttachment(Par ? Par : RootComponent.Get());
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetCastShadow(false);
		C->SetRelativeLocation(L);
		C->SetRelativeRotation(Rot);
		C->SetRelativeScale3D(S / 100.f);
		C->SetMaterial(0, Mat);
		C->RegisterComponent();
		return C;
	};
	auto Light = [&](FVector L, FLinearColor Col, float Intensity, float Radius)
	{
		UPointLightComponent* P = NewObject<UPointLightComponent>(this);
		P->SetupAttachment(RootComponent);
		P->SetRelativeLocation(L);
		P->SetLightColor(Col, false);
		P->SetIntensity(Intensity);
		P->SetAttenuationRadius(Radius);
		P->SetCastShadows(false);
		P->RegisterComponent();
	};
	const FRotator R0 = FRotator::ZeroRotator;

	// ---------- Paredes brancas desgastadas: manchas, rachaduras, sujeira na base e no topo ----------
	struct FFace { FVector Pt; bool bUX; float NS; float U0, U1; int32 Num; bool bBands; };
	const FFace Faces[] = {
		{ FVector(0, -580, 0),  true,  +1.f, -1170.f, 1170.f, 16, true },
		{ FVector(0,  580, 0),  true,  -1.f, -1170.f, 1170.f, 16, true },
		{ FVector(-1180, 0, 0), false, +1.f, -570.f,  570.f,  10, true },
		{ FVector( 1180, 0, 0), false, -1.f, -570.f,  570.f,  10, true },
		{ FVector(-20, 0, 0),   false, -1.f, -570.f, -160.f,  5, false },
		{ FVector(-20, 0, 0),   false, -1.f,  160.f,  570.f,  5, false },
		{ FVector( 20, 0, 0),   false, +1.f, -570.f, -160.f,  5, false },
		{ FVector( 20, 0, 0),   false, +1.f,  160.f,  570.f,  5, false },
	};
	for (const FFace& F : Faces)
	{
		auto At = [&](float u, float z, float n) { return F.Pt + (F.bUX ? FVector(u, F.NS * n, z) : FVector(F.NS * n, u, z)); };
		for (int32 i = 0; i < F.Num; ++i)
		{
			const float u = R.FRandRange(F.U0 + 30.f, F.U1 - 30.f), z = R.FRandRange(30.f, 370.f);
			const float w = R.FRandRange(25.f, 150.f), h = R.FRandRange(15.f, 110.f), p = R.GetFraction();
			UMaterialInstanceDynamic* M = p < 0.6f ? mStain : (p < 0.85f ? mDark : mLight);
			Add(Cube, At(u, z, 0.9f), F.bUX ? FVector(w, 1.6f, h) : FVector(1.6f, w, h), R0, M);
		}
		for (int32 i = 0; i < F.Num / 2 + 1; ++i)
		{
			const float u = R.FRandRange(F.U0 + 30.f, F.U1 - 30.f), z = R.FRandRange(120.f, 300.f);
			const float L = R.FRandRange(80.f, 240.f), a = R.FRandRange(-35.f, 35.f);
			Add(Cube, At(u, z, 1.0f), F.bUX ? FVector(2.5f, 1.8f, L) : FVector(1.8f, 2.5f, L),
				F.bUX ? FRotator(a, 0, 0) : FRotator(0, 0, a), mCrack);
		}
		if (F.bBands)
		{
			const float len = F.U1 - F.U0, mid = (F.U0 + F.U1) * 0.5f;
			Add(Cube, At(mid, 18.f, 0.6f),  F.bUX ? FVector(len, 1.2f, 36.f) : FVector(1.2f, len, 36.f), R0, mGrime);
			Add(Cube, At(mid, 385.f, 0.6f), F.bUX ? FVector(len, 1.2f, 30.f) : FVector(1.2f, len, 30.f), R0, mGrime);
		}
	}

	// ---------- LADO NEON (y < 0): verde + roxo, moderno ----------
	Add(Cube, FVector(0, -577, 60),  FVector(2340, 3, 6), R0, mGreen);
	Add(Cube, FVector(0, -577, 340), FVector(2340, 3, 6), R0, mPurple);
	for (int32 i = -2; i <= 2; ++i)
		Add(Cube, FVector(i * 400.f, -577, 200), FVector(6, 3, 220), R0, (i & 1) ? mPurple : mGreen);
	for (float px : { -650.f, 650.f })
	{
		Add(Cube, FVector(px, -577, 225), FVector(170, 3, 100), R0, mGlass);
		Add(Cube, FVector(px, -575, 277), FVector(176, 3, 4), R0, mGreen);
		Add(Cube, FVector(px, -575, 173), FVector(176, 3, 4), R0, mGreen);
		Add(Cube, FVector(px - 87, -575, 225), FVector(4, 3, 104), R0, mPurple);
		Add(Cube, FVector(px + 87, -575, 225), FVector(4, 3, 104), R0, mPurple);
	}
	Add(Cube, FVector(-1177, -300, 60),  FVector(3, 560, 6), R0, mGreen);
	Add(Cube, FVector(-1177, -300, 340), FVector(3, 560, 6), R0, mPurple);
	Add(Cube, FVector(1177, -300, 60),   FVector(3, 560, 6), R0, mPurple);
	Add(Cube, FVector(1177, -300, 340),  FVector(3, 560, 6), R0, mGreen);
	Add(Cube, FVector(-21, -375, 60),  FVector(3, 430, 6), R0, mPurple);
	Add(Cube, FVector(21, -375, 60),   FVector(3, 430, 6), R0, mGreen);
	// grade luminosa no chão
	for (float gy : { -100.f, -250.f, -400.f, -550.f })
		Add(Cube, FVector(0, gy, 0.9f), FVector(2380, 3, 1.8f), R0, (FMath::RoundToInt(gy) / 150) & 1 ? mGreen : mPurple);
	for (int32 i = -4; i <= 4; ++i)
		Add(Cube, FVector(i * 270.f, -300, 0.9f), FVector(3, 560, 1.8f), R0, (i & 1) ? mGreen : mPurple);
	// colunas de luz
	Add(Cyl, FVector(-1120, -530, 200), FVector(14, 14, 400), R0, mGreen);
	Add(Cyl, FVector(1120, -530, 200),  FVector(14, 14, 400), R0, mPurple);
	Light(FVector(-700, -300, 260), Green, 900.f, 900.f);
	Light(FVector(-50, -400, 260),  Purple, 900.f, 900.f);
	Light(FVector(650, -300, 260),  Green, 900.f, 900.f);
	Light(FVector(1000, -450, 260), Purple, 700.f, 800.f);

	// ---------- LADO RUÍNA (y > 0): concreto, rachaduras, entulho, cristais roxos ----------
	Add(Cube, FVector(0, 572, 290), FVector(2340, 16, 20), R0, mConcD);
	for (float cx : { -800.f, 300.f, 900.f })
		Add(Cyl, FVector(cx, 545, 200), FVector(70, 70, 400), R0, mConc);
	// coluna quebrada + pedaços no chão
	Add(Cyl, FVector(-300, 520, 110), FVector(70, 70, 220), R0, mConc);
	Add(Cube, FVector(-300, 520, 225), FVector(85, 70, 22), FRotator(8, 20, 12), mConcD);
	Add(Cyl, FVector(-120, 430, 35), FVector(66, 66, 170), FRotator(90, 25, 0), mConc);
	for (int32 i = 0; i < 16; ++i)
		Add(Cube, FVector(R.FRandRange(-1150.f, 1150.f), R.FRandRange(40.f, 570.f), 0.7f),
			FVector(R.FRandRange(100.f, 300.f), 3.f, 1.4f), FRotator(0, R.FRandRange(0.f, 180.f), 0), mCrack);
	for (int32 i = 0; i < 22; ++i)
	{
		const float s = R.FRandRange(12.f, 50.f);
		Add(Cube, FVector(R.FRandRange(-1150.f, 1150.f), R.FRandRange(330.f, 570.f), s * 0.35f), FVector(s, s * R.FRandRange(0.6f, 1.3f), s * 0.7f),
			FRotator(R.FRandRange(0.f, 30.f), R.FRandRange(0.f, 180.f), R.FRandRange(0.f, 30.f)), (i & 1) ? mConc : mConcD);
	}
	auto Crystal = [&](FVector Loc, float H, float Rad, FRotator Tilt)
	{
		USceneComponent* P = NewObject<USceneComponent>(this);
		P->SetupAttachment(RootComponent);
		P->SetRelativeLocation(Loc);
		P->SetRelativeRotation(Tilt);
		P->RegisterComponent();
		Add(Cone, FVector(0, 0, H * 0.5f), FVector(Rad * 2.f, Rad * 2.f, H), R0, mCryst, P);
		Add(Cone, FVector(0, 0, -H * 0.18f), FVector(Rad * 2.f, Rad * 2.f, H * 0.36f), FRotator(180, 0, 0), mCryst, P);
	};
	auto Cluster = [&](FVector C, int32 N)
	{
		for (int32 i = 0; i < N; ++i)
			Crystal(C + FVector(R.FRandRange(-45.f, 45.f), R.FRandRange(-45.f, 45.f), 0),
				R.FRandRange(80.f, 180.f), R.FRandRange(13.f, 26.f), FRotator(R.FRandRange(-22.f, 22.f), R.FRandRange(0.f, 360.f), R.FRandRange(-22.f, 22.f)));
		Light(C + FVector(0, 0, 110), Purple, 600.f, 650.f);
	};
	Cluster(FVector(-1020, 490, 0), 5);
	Cluster(FVector(-480, 560, 0), 3);
	Cluster(FVector(250, 470, 0), 4);
	Cluster(FVector(720, 420, 0), 4);
	Cluster(FVector(1060, 540, 0), 5);
	if (Variant == 2) Cluster(FVector(-1090, 500, 300), 4); // topo da plataforma da Sala 2
}
