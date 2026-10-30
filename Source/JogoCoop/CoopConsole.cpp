#include "CoopConsole.h"
#include "CoopElevator.h"
#include "CoopBalanceMachine.h"
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
	if (Action == ECoopConsoleAction::MachineReset) return Machine ? Machine->ServerReset(OutWhy) : false;
	if (!Elevator) { OutWhy = TEXT("Console sem elevador"); return false; }
	switch (Action)
	{
	case ECoopConsoleAction::ElevatorToggle: return Elevator->ServerRequestGo(!Elevator->IsTargetTop(), OutWhy);
	case ECoopConsoleAction::ElevatorCallBottom: return Elevator->ServerRequestGo(false, OutWhy);
	case ECoopConsoleAction::ElevatorCallTop: return Elevator->ServerRequestGo(true, OutWhy);
	}
	return false;
}

// indicador: vermelho = travado/bloqueado | ambar = sem efeito agora | verde = disponivel (estado replicado do elevador/maquina)
void ACoopConsole::Tick(float Dt)
{
	Super::Tick(Dt);
	if (!ScreenMat || (!Elevator && !Machine)) return;
	int32 S = 0; Prompt(S);
	if (S == LastState) return;
	LastState = S;
	const FLinearColor C = S == 0 ? FLinearColor(1.f, 0.08f, 0.04f) : S == 1 ? FLinearColor(1.f, 0.55f, 0.05f) : FLinearColor(0.15f, 1.f, 0.35f);
	ScreenMat->SetVectorParameterValue(TEXT("Color"), C * 0.15f);
	ScreenMat->SetVectorParameterValue(TEXT("Emissive"), C * 4.f);
	if (Lamp) Lamp->SetLightColor(C);
}

FString ACoopConsole::Prompt(int32& S) const
{
	S = 1;
	if (Action == ECoopConsoleAction::MachineReset)
	{
		if (!Machine) return FString();
		if (Machine->IsDone()) { S = 1; return TEXT("Mecanismo travado"); }
		S = 2; return TEXT("[E] Rearmar mecanismo");
	}
	if (!Elevator) return FString();
	if (Elevator->bLockedP1) { S = 0; return TEXT("Elevador travado"); }
	if (Elevator->IsMoving()) { S = 1; return TEXT("Elevador em movimento"); }
	const bool bTop = Elevator->IsTargetTop();
	switch (Action)
	{
	case ECoopConsoleAction::ElevatorToggle: S = 2; return bTop ? TEXT("[E] Descer") : TEXT("[E] Subir");
	case ECoopConsoleAction::ElevatorCallBottom: if (bTop) { S = 2; return TEXT("[E] Chamar o elevador"); } S = 1; return TEXT("Elevador neste andar");
	case ECoopConsoleAction::ElevatorCallTop: if (!bTop) { S = 2; return TEXT("[E] Chamar o elevador"); } S = 1; return TEXT("Elevador neste andar");
	default: break;
	}
	return FString();
}
