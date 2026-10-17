#include "CoopBurst.h"
#include "JJVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ACoopBurst::ACoopBurst()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ACoopBurst::Init(FLinearColor Col, FVector From)
{
	UStaticMesh* Cube = JJ::Mesh(TEXT("Cube"));
	UMaterialInstanceDynamic* M = JJ::Mat(this, Col * 0.1f, Col * 6.f, 0.3f);
	for (int32 i = 0; i < 28; ++i)
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetStaticMesh(Cube);
		C->SetupAttachment(RootComponent);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetCastShadow(false);
		C->SetMaterial(0, M);
		C->SetRelativeScale3D(FVector(0.12f));
		C->RegisterComponent();
		Parts.Add(C);
		Dirs.Add(FMath::VRand());
	}
	Beam = NewObject<UStaticMeshComponent>(this);
	Beam->SetStaticMesh(Cube);
	Beam->SetupAttachment(RootComponent);
	Beam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Beam->SetCastShadow(false);
	Beam->SetMaterial(0, M);
	Beam->RegisterComponent();
	const FVector To = GetActorLocation();
	const FVector D = To - From;
	Beam->SetWorldLocationAndRotation((From + To) * 0.5f, D.Rotation());
	Beam->SetWorldScale3D(FVector(FMath::Max(D.Size(), 1.f) / 100.f, 0.06f, 0.06f));
}

void ACoopBurst::Tick(float DT)
{
	Super::Tick(DT);
	Age += DT;
	const float T = Age / 0.7f;
	for (int32 i = 0; i < Parts.Num(); ++i)
	{
		Parts[i]->SetRelativeLocation(Dirs[i] * (30.f + 260.f * T * (0.5f + (i % 3) * 0.25f)));
		Parts[i]->SetRelativeScale3D(FVector(FMath::Max(0.12f * (1.f - T), 0.001f)));
	}
	if (Beam)
	{
		const float B = FMath::Clamp(1.f - Age / 0.3f, 0.f, 1.f);
		Beam->SetVisibility(B > 0.f);
		const FVector S = Beam->GetComponentScale();
		Beam->SetWorldScale3D(FVector(S.X, 0.07f * B, 0.07f * B));
	}
	if (Age > 0.8f) Destroy();
}
