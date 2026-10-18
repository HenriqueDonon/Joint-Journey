#include "CoopCeiling.h"
#include "Components/StaticMeshComponent.h"

ACoopCeiling::ACoopCeiling()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(60.f);
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // visual: nao atrapalha jogadores nem mira
}

void ACoopCeiling::BeginPlay() { Super::BeginPlay(); Home = GetActorLocation(); }

void ACoopCeiling::StartFall(float Duration, float Dist)
{
	if (!HasAuthority()) return;
	FallDur = FMath::Max(Duration, 0.05f); FallDist = Dist; FallT = 0.f; bFalling = true;
}

void ACoopCeiling::ResetPos()
{
	if (!HasAuthority()) return;
	bFalling = false;
	SetActorLocation(Home);
}

void ACoopCeiling::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!HasAuthority() || !bFalling) return;
	FallT += DeltaTime;
	const float A = FMath::Clamp(FallT / FallDur, 0.f, 1.f);
	SetActorLocation(Home - FVector(0, 0, FallDist * A * A)); // acelera (queda livre)
	if (A >= 1.f) bFalling = false;
}
