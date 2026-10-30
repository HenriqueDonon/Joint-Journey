#include "CoopConsole.h"
#include "CoopElevator.h"
#include "CoopCharacter.h"
#include "JJVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

ACoopConsole::ACoopConsole()
{
	bReplicates = false;   // ator de nivel: existe em todas as maquinas; so o servidor executa
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
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
	ScreenMat = JJ::Mat(this, FLinearColor(0.2f, 0.05f, 0.03f), FLinearColor(1.f, 0.1f, 0.05f) * 4.f, 0.4f);
	Screen->SetMaterial(0, ScreenMat);
	Lamp = NewObject<UPointLightComponent>(this); Lamp->SetupAttachment(Mesh); Lamp->RegisterComponent();
	Lamp->SetRelativeLocation(FVector(-120.f, 0.f, 30.f)); Lamp->SetAttenuationRadius(260.f); Lamp->SetCastShadows(false); Lamp->SetIntensity(1500.f);
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

// indicador: vermelho = travado | ambar = em movimento | verde = disponivel (le o estado replicado do elevador)
void ACoopConsole::Tick(float Dt)
{
	Super::Tick(Dt);
	if (!Elevator || !ScreenMat) return;
	const int32 S = Elevator->bLockedP1 ? 0 : Elevator->IsMoving() ? 1 : 2;
	if (S == LastState) return;
	LastState = S;
	const FLinearColor C = S == 0 ? FLinearColor(1.f, 0.08f, 0.04f) : S == 1 ? FLinearColor(1.f, 0.55f, 0.05f) : FLinearColor(0.15f, 1.f, 0.35f);
	ScreenMat->SetVectorParameterValue(TEXT("Color"), C * 0.15f);
	ScreenMat->SetVectorParameterValue(TEXT("Emissive"), C * 4.f);
	if (Lamp) Lamp->SetLightColor(C);
}

FString ACoopConsole::Prompt(bool& bAvailable) const
{
	bAvailable = false;
	if (!Elevator) return FString();
	if (Elevator->bLockedP1) return TEXT("[E] Elevador TRAVADO - o mecanismo precisa de conserto");
	if (Elevator->IsMoving()) return TEXT("Elevador em movimento...");
	bAvailable = true;
	switch (Action)
	{
	case ECoopConsoleAction::ElevatorToggle: return Elevator->IsTargetTop() ? TEXT("[E] Descer") : TEXT("[E] Subir");
	case ECoopConsoleAction::ElevatorCallBottom: return Elevator->IsTargetTop() ? TEXT("[E] Chamar o elevador") : TEXT("[E] Elevador aqui - entre na cabine");
	case ECoopConsoleAction::ElevatorCallTop: return Elevator->IsTargetTop() ? TEXT("[E] Elevador aqui - entre na cabine") : TEXT("[E] Chamar o elevador");
	}
	return FString();
}
