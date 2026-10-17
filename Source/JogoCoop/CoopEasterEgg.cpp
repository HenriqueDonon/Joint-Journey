#include "CoopEasterEgg.h"
#include "CoopCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UStaticMeshComponent* ACoopEasterEgg::MakePart(const TCHAR* Name, FVector Loc, FRotator Rot, FVector Scale)
{
	UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	C->SetupAttachment(Root);
	C->SetRelativeLocation(Loc);
	C->SetRelativeRotation(Rot);
	C->SetRelativeScale3D(Scale);
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cyl(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Cyl.Succeeded()) C->SetStaticMesh(Cyl.Object);
	return C;
}

ACoopEasterEgg::ACoopEasterEgg()
{
	bReplicates = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	// Corpo deitado ao longo de X (cilindro gira 90 graus), ~60cm x 8cm
	Paper   = MakePart(TEXT("Paper"),   FVector(0, 0, 4),     FRotator(90, 0, 0),   FVector(0.08f, 0.08f, 0.60f));
	// Pontinha torcida: dois trechos finos, tortos, como a seda torcida na ponta
	TipA    = MakePart(TEXT("TipA"),    FVector(34, 0, 4.5f), FRotator(80, 15, 25), FVector(0.05f, 0.05f, 0.14f));
	TipB    = MakePart(TEXT("TipB"),    FVector(42, 2, 5.5f), FRotator(65, 40, 50), FVector(0.025f, 0.025f, 0.12f));
	Filling = MakePart(TEXT("Filling"), FVector(-30.5f, 0, 4), FRotator(90, 0, 0),  FVector(0.075f, 0.075f, 0.02f));

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(Root);
	Trigger->SetBoxExtent(FVector(160, 160, 100));
}

void ACoopEasterEgg::Tint(UStaticMeshComponent* C, FLinearColor Col)
{
	if (UMaterialInstanceDynamic* M = C->CreateAndSetMaterialInstanceDynamic(0))
		M->SetVectorParameterValue(TEXT("Color"), Col);
}

void ACoopEasterEgg::BeginPlay()
{
	Super::BeginPlay();
	Tint(Paper,   FLinearColor(0.95f, 0.93f, 0.85f));   // seda
	Tint(TipA,    FLinearColor(0.85f, 0.80f, 0.65f));
	Tint(TipB,    FLinearColor(0.80f, 0.74f, 0.58f));
	Tint(Filling, FLinearColor(0.25f, 0.15f, 0.06f));   // fumo
	if (HasAuthority()) Trigger->OnComponentBeginOverlap.AddDynamic(this, &ACoopEasterEgg::OnBegin);
}

void ACoopEasterEgg::OnBegin(UPrimitiveComponent*, AActor* A, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	ACoopCharacter* C = Cast<ACoopCharacter>(A);
	if (!C || Shown.Contains(C)) return;
	Shown.Add(C);
	C->ClientShowMessage(Message);
}
