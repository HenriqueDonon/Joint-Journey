#include "CoopConsole.h"
#include "CoopElevator.h"
#include "CoopCharacter.h"
#include "JJVisual.h"
#include "Components/StaticMeshComponent.h"

ACoopConsole::ACoopConsole()
{
	bReplicates = false;   // ator de nivel: existe em todas as maquinas; so o servidor executa
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Corpo"));
	SetRootComponent(Mesh);
	Mesh->SetMobility(EComponentMobility::Movable);
	Screen = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tela"));
	Screen->SetupAttachment(Mesh);
	Screen->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACoopConsole::BeginPlay()
{
	Super::BeginPlay();
	UStaticMesh* Cube = JJ::Mesh(TEXT("Cube"));
	Mesh->SetStaticMesh(Cube); Mesh->SetWorldScale3D(FVector(0.35f, 0.5f, 1.2f));   // 35x50x120 cm, origem no centro
	Mesh->SetMaterial(0, JJ::Mat(this, FLinearColor(0.22f, 0.22f, 0.24f), FLinearColor::Black, 0.6f, 0.4f));
	Screen->SetStaticMesh(Cube); Screen->SetRelativeLocation(FVector(-52.f, 0.f, 25.f)); Screen->SetRelativeScale3D(FVector(0.1f, 0.8f, 0.3f));
	const FLinearColor G = Action == ECoopConsoleAction::ElevatorToggle ? FLinearColor(1.f, 0.6f, 0.1f) : FLinearColor(0.2f, 1.f, 0.4f);
	Screen->SetMaterial(0, JJ::Mat(this, G * 0.2f, G * 4.f, 0.4f));
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
	if (bAttachToElevator && Elevator) AttachToActor(Elevator, FAttachmentTransformRules::KeepWorldTransform);
}

bool ACoopConsole::ServerUse(ACoopCharacter* By, FString& OutWhy)
{
	if (!HasAuthority()) { OutWhy = TEXT("sem autoridade"); return false; }
	if (!Elevator) { OutWhy = TEXT("Console sem elevador"); return false; }
	switch (Action)
	{
	case ECoopConsoleAction::ElevatorToggle: return Elevator->ServerRequestGo(!Elevator->IsTargetTop(), OutWhy);
	case ECoopConsoleAction::ElevatorCallBottom: return Elevator->ServerRequestGo(false, OutWhy);
	case ECoopConsoleAction::ElevatorCallTop: return Elevator->ServerRequestGo(true, OutWhy);
	}
	return false;
}
