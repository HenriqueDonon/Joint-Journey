#include "CoopGoal.h"
#include "CoopCharacter.h"
#include "CoopGameMode.h"
#include "JJVisual.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

ACoopGoal::ACoopGoal()
{
	bReplicates = true;
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	RootComponent = Box;
	Box->SetBoxExtent(FVector(200, 300, 150));
	Pad = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pad"));
	Pad->SetupAttachment(Box);
	Pad->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Pad->SetStaticMesh(Cube.Object);
	Pad->SetRelativeLocation(FVector(0, 0, -148));
	Pad->SetRelativeScale3D(FVector(4.f, 6.f, 0.04f));
}
void ACoopGoal::BeginPlay()
{
	Super::BeginPlay();
	Mat = JJ::Mat(this, FLinearColor(0.02f, 0.05f, 0.1f), FLinearColor(0.1f, 0.5f, 1.f) * 2.5f, 0.3f);
	Pad->SetMaterial(0, Mat);
	if (!HasAuthority()) return;
	Box->OnComponentBeginOverlap.AddDynamic(this, &ACoopGoal::OnBegin);
	Box->OnComponentEndOverlap.AddDynamic(this, &ACoopGoal::OnEnd);
}
void ACoopGoal::OnBegin(UPrimitiveComponent*, AActor* A, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!Cast<ACoopCharacter>(A)) return;
	Inside.Add(A);
	const int32 Need = 3;
	if (Inside.Num() < Need && GEngine)
		GEngine->AddOnScreenDebugMessage(102, 3.f, FColor::Yellow, FString::Printf(TEXT("Chegada: %d/%d jogadores"), Inside.Num(), Need));
	if (Inside.Num() >= Need && !bComplete)
	{
		bComplete = true;
		OnRep_Complete();
		if (ACoopGameMode* GM = GetWorld()->GetAuthGameMode<ACoopGameMode>()) GM->RoomCompleted(RoomIndex);
	}
}
void ACoopGoal::OnEnd(UPrimitiveComponent*, AActor* A, UPrimitiveComponent*, int32) { Inside.Remove(A); }
void ACoopGoal::OnRep_Complete()
{
	if (!bComplete) return;
	if (Mat) Mat->SetVectorParameterValue(TEXT("Emissive"), FLinearColor(0.1f, 1.f, 0.3f) * 3.f);
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, RoomIndex == 1 ? TEXT("SALA 1 COMPLETA! Indo para a Sala 2...") : TEXT("SALA 2 COMPLETA! Fim do prototipo - parabens!"));
}
void ACoopGoal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopGoal, bComplete);
	DOREPLIFETIME(ACoopGoal, RoomIndex);
}
