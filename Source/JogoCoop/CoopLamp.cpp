#include "CoopLamp.h"
#include "Components/PointLightComponent.h"
#include "Net/UnrealNetwork.h"

ACoopLamp::ACoopLamp()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.04f;
	SetReplicateMovement(false);
	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	RootComponent = Light;
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetCastShadows(false);
	Light->SetIntensityUnits(ELightUnits::Lumens);
	OnRep_Lamp();
}

void ACoopLamp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Flicker <= 0.f || !Light) return;
	const float T = GetWorld()->GetTimeSeconds();
	Light->SetIntensity(Lumens * FMath::Clamp(1.f + Flicker * FMath::PerlinNoise1D(T * 7.f + GetActorLocation().X * 0.013f) * 1.6f, 0.2f, 2.f));
}

void ACoopLamp::Setup(FLinearColor InColor, float InLumens, float InRadius, float InFlicker)
{
	Col = InColor; Lumens = InLumens; Radius = InRadius; Flicker = InFlicker;
	OnRep_Lamp();
}

void ACoopLamp::OnRep_Lamp()
{
	if (!Light) return;
	Light->SetLightColor(Col);
	Light->SetIntensity(Lumens);
	Light->SetAttenuationRadius(Radius);
}

void ACoopLamp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopLamp, Col);
	DOREPLIFETIME(ACoopLamp, Lumens);
	DOREPLIFETIME(ACoopLamp, Radius);
	DOREPLIFETIME(ACoopLamp, Flicker);
}
