#include "CoopExitDoor.h"
#include "Components/StaticMeshComponent.h"

ACoopExitDoor::ACoopExitDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(60.f);
	Mesh->SetMobility(EComponentMobility::Movable);
}

void ACoopExitDoor::BeginPlay() { Super::BeginPlay(); Home = GetActorLocation(); }

void ACoopExitDoor::Tick(float DT)
{
	Super::Tick(DT);
	if (!HasAuthority()) return;
	const float Target = bOpen ? 1.f : 0.f;
	if (FMath::IsNearlyEqual(Open01, Target)) return;
	Open01 = FMath::FInterpConstantTo(Open01, Target, DT, 0.7f);
	SetActorLocation(Home + FVector(0, 0, (Size.Z + 12.f) * Open01));
}
