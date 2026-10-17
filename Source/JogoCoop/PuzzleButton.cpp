#include "PuzzleButton.h"
#include "PuzzleDoor.h"
#include "CoopCharacter.h"
#include "CoopProp.h"
#include "JJVisual.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

APuzzleButton::APuzzleButton()
{
	bReplicates = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Mesh->SetStaticMesh(Cube.Object);
	Mesh->SetRelativeScale3D(FVector(1.3f, 1.3f, 0.15f));
	Mesh->SetRelativeLocation(FVector(0, 0, 7.5f));
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(Root);
	Trigger->SetBoxExtent(FVector(65, 65, 40));
	Trigger->SetRelativeLocation(FVector(0, 0, 40));
}

void APuzzleButton::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APuzzleButton, bPressedVis);
}

void APuzzleButton::BeginPlay()
{
	Super::BeginPlay();
	Mat = JJ::Mat(this, FLinearColor(0.08f, 0.08f, 0.1f), FLinearColor::Black, 0.3f, 0.6f);
	Mesh->SetMaterial(0, Mat);
	OnRep_Pressed();
	if (!HasAuthority()) return;
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &APuzzleButton::OnBegin);
	Trigger->OnComponentEndOverlap.AddDynamic(this, &APuzzleButton::OnEnd);
}

void APuzzleButton::OnRep_Pressed()
{
	if (Mat) Mat->SetVectorParameterValue(TEXT("Emissive"), bPressedVis ? FLinearColor(0.1f, 1.f, 0.3f) * 3.f : FLinearColor(1.f, 0.5f, 0.05f) * 1.2f);
}

void APuzzleButton::OnBegin(UPrimitiveComponent*, AActor* A, UPrimitiveComponent*, int32, bool, const FHitResult&)
{ if (Cast<ACoopCharacter>(A) || Cast<ACoopProp>(A)) { ++Count; Update(); } }
void APuzzleButton::OnEnd(UPrimitiveComponent*, AActor* A, UPrimitiveComponent*, int32)
{ if (Cast<ACoopCharacter>(A) || Cast<ACoopProp>(A)) { Count = FMath::Max(0, Count - 1); Update(); } }

void APuzzleButton::Update()
{
	if (bLatch && Count > 0) bLatched = true;
	const bool bOn = Count > 0 || bLatched;
	bPressedVis = bOn;
	OnRep_Pressed();
	for (APuzzleDoor* D : Doors) if (D) D->SetOpen(bOn);
}
