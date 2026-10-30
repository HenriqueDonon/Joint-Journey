#include "CoopGate.h"
#include "JJVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

ACoopGate::ACoopGate()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; bAlwaysRelevant = true;
	SetReplicateMovement(false);
	USceneComponent* R = CreateDefaultSubobject<USceneComponent>(TEXT("Raiz"));
	SetRootComponent(R);
	Panel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Painel"));
	Panel->SetupAttachment(R);
	Panel->SetMobility(EComponentMobility::Movable);
	Panel->SetCollisionProfileName(TEXT("BlockAll"));
}

void ACoopGate::BeginPlay()
{
	Super::BeginPlay();
	Panel->SetStaticMesh(JJ::Mesh(TEXT("Cube")));
	Panel->SetRelativeScale3D(Size / 100.f);
	Panel->SetMaterial(0, JJ::Mat(this, FLinearColor(0.55f, 0.42f, 0.08f), FLinearColor(0.6f, 0.35f, 0.02f) * 0.4f, 0.5f, 0.6f));
	A = bOpen ? 1.f : 0.f;
	Panel->SetRelativeLocation(FVector(0, 0, Size.Z * 0.5f + A * (Size.Z - 20.f)));
}

void ACoopGate::ServerSetOpen(bool b) { if (HasAuthority()) { bOpen = b; ForceNetUpdate(); } }

void ACoopGate::Tick(float Dt)
{
	Super::Tick(Dt);
	A = FMath::FInterpConstantTo(A, bOpen ? 1.f : 0.f, Dt, 0.8f);
	Panel->SetRelativeLocation(FVector(0, 0, Size.Z * 0.5f + A * (Size.Z - 20.f)));   // sobe quase toda a altura
}

void ACoopGate::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopGate, bOpen);
}
