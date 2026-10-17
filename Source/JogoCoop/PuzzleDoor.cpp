#include "PuzzleDoor.h"
#include "JJVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

APuzzleDoor::APuzzleDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetMobility(EComponentMobility::Movable);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Mesh->SetStaticMesh(Cube.Object);
	Mesh->SetRelativeScale3D(FVector(3.f, 0.3f, 3.f));
}

void APuzzleDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APuzzleDoor, bOpenVis);
}

void APuzzleDoor::BeginPlay()
{
	Super::BeginPlay();
	Closed = GetActorLocation();
	Mat = JJ::Mat(this, FLinearColor(0.12f, 0.12f, 0.15f), FLinearColor::Black, 0.35f, 0.7f);
	Mesh->SetMaterial(0, Mat);
	OnRep_Open();
}

void APuzzleDoor::OnRep_Open()
{
	if (Mat) Mat->SetVectorParameterValue(TEXT("Emissive"), bOpenVis ? FLinearColor(0.1f, 1.f, 0.3f) * 1.2f : FLinearColor(1.f, 0.08f, 0.05f) * 2.f);
}

void APuzzleDoor::SetOpen(bool bOpen)
{
	if (bWantOpen == bOpen) return;
	bWantOpen = bOpen;
	bOpenVis = bOpen;
	OnRep_Open();
}

void APuzzleDoor::Tick(float DT)
{
	Super::Tick(DT);
	if (!HasAuthority()) return;
	const FVector Target = Closed + FVector(0, 0, bWantOpen ? OpenHeight : 0.f);
	SetActorLocation(FMath::VInterpConstantTo(GetActorLocation(), Target, DT, 250.f));
}
