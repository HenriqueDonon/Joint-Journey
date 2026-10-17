#include "CoopProp.h"
#include "CoopBurst.h"
#include "JJVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ACoopProp::ACoopProp()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);
	SetNetUpdateFrequency(100.f);
	SetMinNetUpdateFrequency(60.f);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (Cube.Succeeded()) Mesh->SetStaticMesh(Cube.Object);
	Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->BodyInstance.DOFMode = EDOFMode::SixDOF;
	Mesh->BodyInstance.bLockXRotation = true;   // o cubo não tomba, só gira em Z
	Mesh->BodyInstance.bLockYRotation = true;

	auto Part = [&](const FString& Name, UStaticMesh* M, FVector Loc, FVector Scale) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(*Name);
		C->SetupAttachment(Mesh);
		C->SetStaticMesh(M);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetCastShadow(false);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Scale);
		C->SetVisibility(false);
		return C;
	};
	UStaticMesh* ConeM = Cone.Succeeded() ? Cone.Object : nullptr;
	UStaticMesh* CubeM = Cube.Succeeded() ? Cube.Object : nullptr;
	for (int32 i = 0; i < 4; ++i)
		Arrows.Add(Part(FString::Printf(TEXT("Arrow%d"), i), ConeM, FVector((i & 1) ? 62.f : -62.f, (i & 2) ? 62.f : -62.f, 20.f), FVector(0.18f, 0.18f, 0.5f)));
	for (int32 i = 0; i < 8; ++i)
	{
		const float A = i * 45.f;
		UStaticMeshComponent* S = Part(FString::Printf(TEXT("Shard%d"), i), ConeM, FVector(FMath::Cos(FMath::DegreesToRadians(A)) * 72.f, FMath::Sin(FMath::DegreesToRadians(A)) * 72.f, 0.f), FVector(0.14f, 0.14f, 0.6f));
		S->SetRelativeRotation(FQuat(FRotator(0, A, 0)) * FQuat(FRotator(0, 0, -60)));
		Shards.Add(S);
	}
	OrbitPivot = CreateDefaultSubobject<USceneComponent>(TEXT("OrbitPivot"));
	OrbitPivot->SetupAttachment(Mesh);
	for (int32 i = 0; i < 8; ++i)
	{
		const float A = i * 45.f;
		Orbit.Add(Part(FString::Printf(TEXT("Orbit%d"), i), CubeM, FVector(FMath::Cos(FMath::DegreesToRadians(A)) * 85.f, FMath::Sin(FMath::DegreesToRadians(A)) * 85.f, (i & 1) ? 25.f : -25.f), FVector(0.1f)));
		Orbit.Last()->SetupAttachment(OrbitPivot);
	}
}

void ACoopProp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopProp, State);
	DOREPLIFETIME(ACoopProp, RepPos);
	DOREPLIFETIME(ACoopProp, RepYaw);
	DOREPLIFETIME(ACoopProp, bHasRep);
}

void ACoopProp::BeginPlay()
{
	Super::BeginPlay();
	SpawnLoc = GetActorLocation();
	Mesh->SetSimulatePhysics(HasAuthority()); // física só no servidor; clientes recebem o movimento
	if (HasAuthority()) Mesh->SetMassOverrideInKg(NAME_None, 25.f, true);

	BodyMat = JJ::Mat(this, FLinearColor(0.50f, 0.42f, 0.34f), FLinearColor::Black, 0.55f, 0.35f);
	Mesh->SetMaterial(0, BodyMat);
	const FLinearColor G(0.1f, 1.f, 0.3f), P(0.6f, 0.3f, 1.f), C(0.f, 0.8f, 1.f);
	UMaterialInstanceDynamic* mG = JJ::Mat(this, G * 0.1f, G * 5.f, 0.3f);
	UMaterialInstanceDynamic* mP = JJ::Mat(this, P * 0.1f, P * 5.f, 0.3f);
	UMaterialInstanceDynamic* mC = JJ::Mat(this, C * 0.1f, C * 5.f, 0.3f);
	for (UStaticMeshComponent* X : Arrows) X->SetMaterial(0, mG);
	for (UStaticMeshComponent* X : Shards) X->SetMaterial(0, mP);
	for (UStaticMeshComponent* X : Orbit) X->SetMaterial(0, mC);
	ApplyVis();
}

void ACoopProp::SetPhysics(bool bOn) { Mesh->SetSimulatePhysics(bOn); }

void ACoopProp::RefreshState()
{
	State = bRewinding ? 3 : (bFrozen ? 2 : (bReversed ? 1 : 0));
	ApplyVis();
}
void ACoopProp::OnRep_State() { ApplyVis(); }

void ACoopProp::ApplyVis()
{
	for (UStaticMeshComponent* X : Arrows) X->SetVisibility(State == 1);
	for (UStaticMeshComponent* X : Shards) X->SetVisibility(State == 2);
	for (UStaticMeshComponent* X : Orbit)  X->SetVisibility(State == 3);
}

void ACoopProp::FlipGravity()
{
	bReversed = !bReversed;
	AwakeUntil = GetWorld()->GetTimeSeconds() + 5.f;
	Mesh->WakeAllRigidBodies();
	RefreshState();
}

void ACoopProp::ToggleFreeze()
{
	if (bRewinding) return;
	bFrozen = !bFrozen;
	SetPhysics(!bFrozen);
	if (!bFrozen) { AwakeUntil = GetWorld()->GetTimeSeconds() + 5.f; Mesh->WakeAllRigidBodies(); }
	RefreshState();
}

void ACoopProp::StartRewind()
{
	if (bRewinding || bFrozen || History.Num() == 0) return;
	bRewinding = true;
	SetPhysics(false);
	RefreshState();
}

void ACoopProp::MulticastBurst_Implementation(FLinearColor Col, FVector From)
{
	if (ACoopBurst* B = GetWorld()->SpawnActor<ACoopBurst>(ACoopBurst::StaticClass(), GetActorLocation(), FRotator::ZeroRotator))
		B->Init(Col, From);
}

void ACoopProp::UpdateFx(float DT)
{
	if (!BodyMat) return;
	const float T = GetWorld()->GetTimeSeconds();
	FLinearColor Em = FLinearColor::Black;
	switch (State)
	{
	case 1: Em = FLinearColor(0.1f, 1.f, 0.3f) * (1.5f + FMath::Sin(T * 6.f)); break;
	case 2: Em = FLinearColor(0.6f, 0.3f, 1.f) * 2.5f; break;
	case 3: Em = FLinearColor(0.f, 0.8f, 1.f) * (2.f + FMath::Sin(T * 30.f)); break;
	default: break;
	}
	BodyMat->SetVectorParameterValue(TEXT("Emissive"), Em);
	for (int32 i = 0; i < Arrows.Num(); ++i)
		if (State == 1) Arrows[i]->SetRelativeLocation(FVector((i & 1) ? 62.f : -62.f, (i & 2) ? 62.f : -62.f, 20.f + FMath::Sin(T * 5.f + i) * 14.f));
	if (State == 3 && OrbitPivot) OrbitPivot->AddRelativeRotation(FRotator(0, 540.f * DT, 0));
}

void ACoopProp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateFx(DeltaTime);
	if (!HasAuthority())
	{
		if (bHasRep)
		{
			const FVector Cur = GetActorLocation();
			const FVector Tgt(RepPos);
			if (FVector::DistSquared(Cur, Tgt) > 400.f * 400.f) SetActorLocation(Tgt, false, nullptr, ETeleportType::TeleportPhysics);
			else SetActorLocation(FMath::VInterpTo(Cur, Tgt, DeltaTime, 20.f), false, nullptr, ETeleportType::TeleportPhysics);
			FRotator Rot = GetActorRotation();
			Rot.Yaw = FMath::FInterpTo(Rot.Yaw, RepYaw, DeltaTime, 20.f);
			SetActorRotation(Rot, ETeleportType::TeleportPhysics);
		}
		return;
	}
	RepPos = GetActorLocation();
	RepYaw = GetActorRotation().Yaw;
	bHasRep = true;

	if (GetActorLocation().Z < -300.f) // caiu do mapa: volta pro lugar
	{
		SetActorLocation(SpawnLoc, false, nullptr, ETeleportType::TeleportPhysics);
		Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	}

	if (bRewinding)
	{
		for (int i = 0; i < 2 && History.Num() > 0; ++i)
			SetActorTransform(History.Pop(), false, nullptr, ETeleportType::TeleportPhysics);
		if (History.Num() == 0) { bRewinding = false; SetPhysics(true); RefreshState(); }
		return;
	}
	if (bFrozen) return;
	if (bReversed || GetWorld()->GetTimeSeconds() < AwakeUntil) Mesh->WakeAllRigidBodies();

	if (bReversed) // cancela a gravidade (980) e acelera para cima ReverseAccel
		Mesh->AddForce(FVector(0, 0, 980.f + ReverseAccel), NAME_None, true);

	History.Add(GetActorTransform());
	const int Max = FMath::RoundToInt(RewindSeconds / FMath::Max(DeltaTime, 0.001f));
	if (History.Num() > Max) History.RemoveAt(0, History.Num() - Max);
}
