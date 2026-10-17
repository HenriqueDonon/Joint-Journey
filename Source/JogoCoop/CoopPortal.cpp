#include "CoopPortal.h"
#include "CoopCharacter.h"
#include "CoopProp.h"
#include "JJVisual.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

static TMap<TWeakObjectPtr<AActor>, double> GLastTP;

ACoopPortal::ACoopPortal()
{
	bReplicates = true;
	SetReplicateMovement(false);
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FCyl(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	auto Mk = [&](const TCHAR* N, FVector Loc, FVector Sc) {
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(N);
		C->SetupAttachment(Root);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Sc);
		if (FCyl.Object) C->SetStaticMesh(FCyl.Object);
		C->SetCastShadow(false);
		return C;
	};
	Rim = Mk(TEXT("Rim"), FVector::ZeroVector, FVector(2.3f, 1.3f, 0.04f));
	Inner = Mk(TEXT("Inner"), FVector(0, 0, 1.f), FVector(2.0f, 1.05f, 0.05f));
	Trig = CreateDefaultSubobject<UBoxComponent>(TEXT("Trig"));
	Trig->SetupAttachment(Root);
	Trig->SetBoxExtent(FVector(115.f, 65.f, 20.f));
	Trig->SetRelativeLocation(FVector(0, 0, 20.f));
	Trig->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Trig->SetGenerateOverlapEvents(true);
	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Root);
	Light->SetRelativeLocation(FVector(0, 0, 60.f));
	Light->SetIntensity(4000.f);
	Light->SetAttenuationRadius(500.f);
	Light->SetCastShadows(false);
}

void ACoopPortal::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority()) Trig->OnComponentBeginOverlap.AddDynamic(this, &ACoopPortal::OnOverlap);
	OnRep_Setup();
}

void ACoopPortal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopPortal, bOrange);
	DOREPLIFETIME(ACoopPortal, RepLoc);
	DOREPLIFETIME(ACoopPortal, RepNormal);
}

void ACoopPortal::Setup(bool bInOrange, FVector Loc, FVector Normal)
{
	bOrange = bInOrange;
	RepLoc = Loc;
	RepNormal = Normal.GetSafeNormal();
	OnRep_Setup();
}

void ACoopPortal::OnRep_Setup()
{
	if (RepNormal.IsNearlyZero()) return;
	const FVector N = FVector(RepNormal).GetSafeNormal();
	const FVector Ref = FMath::Abs(N.Z) > 0.95f ? FVector::ForwardVector : FVector::UpVector;
	SetActorLocationAndRotation(FVector(RepLoc) + N * 2.f, FRotationMatrix::MakeFromZX(N, Ref).Rotator());
	const FLinearColor C = bOrange ? FLinearColor(1.f, 0.45f, 0.05f) : FLinearColor(0.1f, 0.5f, 1.f);
	Rim->SetMaterial(0, JJ::Mat(this, FLinearColor::Black, C * 4.f, 0.2f));
	Inner->SetMaterial(0, JJ::Mat(this, FLinearColor(0.01f, 0.01f, 0.02f), C * 0.6f, 0.1f));
	Light->SetLightColor(C);
}

void ACoopPortal::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACoopCharacter* Ch = Cast<ACoopCharacter>(Other);
	ACoopProp* Pr = Cast<ACoopProp>(Other);
	if (!Ch && !Pr) return;
	ACoopPortal* Dest = nullptr;
	for (TActorIterator<ACoopPortal> It(GetWorld()); It; ++It)
		if (It->bOrange != bOrange && !It->RepNormal.IsNearlyZero()) Dest = *It;
	if (!Dest) return;
	const double Now = GetWorld()->GetTimeSeconds();
	if (double* L = GLastTP.Find(Other)) if (Now - *L < 0.7) return;
	GLastTP.Add(Other, Now);

	const FVector N = FVector(Dest->RepNormal).GetSafeNormal();
	const bool bFlat = FMath::Abs(N.Z) > 0.7f;
	if (Ch)
	{
		const float Speed = FMath::Max(Ch->GetVelocity().Size(), 400.f);
		FVector Exit = FVector(Dest->RepLoc) + N * (bFlat ? 95.f : 85.f);
		if (!bFlat) Exit.Z = FMath::Max(Exit.Z, FVector(Dest->RepLoc).Z);
		Ch->SetActorLocation(Exit, false, nullptr, ETeleportType::TeleportPhysics);
		Ch->GetCharacterMovement()->Velocity = N * Speed;
		if (APlayerController* PC = Cast<APlayerController>(Ch->GetController()))
			PC->ClientSetRotation(FRotator(-10.f, bFlat ? Ch->GetActorRotation().Yaw : N.Rotation().Yaw, 0.f));
	}
	else
	{
		const float Speed = FMath::Max(Pr->Mesh->GetPhysicsLinearVelocity().Size(), 250.f);
		Pr->SetActorLocation(FVector(Dest->RepLoc) + N * 105.f, false, nullptr, ETeleportType::TeleportPhysics);
		Pr->Mesh->SetPhysicsLinearVelocity(N * Speed);
		Pr->Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
	GLastTP.Add(Other, Now);
}
