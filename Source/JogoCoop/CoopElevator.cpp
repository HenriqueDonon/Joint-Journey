#include "CoopElevator.h"
#include "CoopCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ACoopElevator::ACoopElevator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	bReplicates = true;
	SetReplicateMovement(false);   // a posicao nao e replicada: vem da hora do servidor + comando
	NetUpdateFrequency = 20.f;
	USceneComponent* R = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	R->SetMobility(EComponentMobility::Movable);
	SetRootComponent(R);
	Platform = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plataforma"));
	Platform->SetupAttachment(R);
	Platform->SetMobility(EComponentMobility::Movable);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Platform->SetStaticMesh(Cube.Object);
	Platform->SetRelativeLocation(FVector(0, 0, -20.f));          // topo da plataforma = origem do ator
	Platform->SetRelativeScale3D(FVector(2.5f, 2.5f, 0.4f));      // 250 x 250 x 40 cm
	Platform->SetCollisionProfileName(TEXT("BlockAll"));
	Platform->SetCanEverAffectNavigation(false);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/Fase2/Materiais/MI_F2_elev.MI_F2_elev"));
	if (Mat.Succeeded()) Platform->SetMaterial(0, Mat.Object);
}

void ACoopElevator::BeginPlay()
{
	Super::BeginPlay();
	Home = GetActorLocation();   // posicao base (cabine embaixo); o mapa posiciona o ator no topo da plataforma, nivel inferior
}

float ACoopElevator::NowSrv() const
{
	const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	return GS ? GS->GetServerWorldTimeSeconds() : (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f);
}

float ACoopElevator::Alpha() const { return FMath::Clamp((NowSrv() - Cmd.T0) / FMath::Max(TravelSeconds, 0.1f), 0.f, 1.f); }
bool ACoopElevator::IsMoving() const { return NowSrv() < Cmd.T0 + TravelSeconds; }

float ACoopElevator::CurrentOffsetZ() const
{
	const float A = Alpha(), E = A * A * (3.f - 2.f * A);   // suavizado (acelera/desacelera)
	const float From = Cmd.bUp ? 0.f : 1.f, To = Cmd.bUp ? 1.f : 0.f;
	return Rise * (From + (To - From) * E);
}

int32 ACoopElevator::NumAboard() const
{
	int32 N = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APawn* P = It->Get() ? It->Get()->GetPawn() : nullptr;
		if (!P) continue;
		const FVector D = P->GetActorLocation() - GetActorLocation();
		if (FMath::Abs(D.X) < HalfArea.X && FMath::Abs(D.Y) < HalfArea.Y && D.Z > 0.f && D.Z < 250.f) ++N;
	}
	return N;
}

void ACoopElevator::ServerLogic(float Dt)
{
	if (IsMoving()) { DwellAcc = 0.f; bArmed = false; return; }
	const int32 N = NumAboard(), P = GetWorld()->GetNumPlayerControllers();
	if (N == 0) bArmed = true;                       // so parte de novo depois de a plataforma ficar vazia
	if (bArmed && P > 0 && N >= P)
	{
		DwellAcc += Dt;
		if (DwellAcc >= DwellSeconds)
		{
			Cmd.bUp = !Cmd.bUp;
			Cmd.T0 = NowSrv() + LeadSeconds;
			DwellAcc = 0.f; bArmed = false;
			ForceNetUpdate();
		}
	}
	else DwellAcc = 0.f;
}

void ACoopElevator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (HasAuthority()) ServerLogic(DeltaSeconds);
	SetActorLocation(Home + FVector(0, 0, CurrentOffsetZ()), false, nullptr, ETeleportType::None);
}

void ACoopElevator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopElevator, Cmd);
}
