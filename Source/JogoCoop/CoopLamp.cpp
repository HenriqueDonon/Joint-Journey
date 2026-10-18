#include "CoopLamp.h"
#include "Components/PointLightComponent.h"
#include "Net/UnrealNetwork.h"

ACoopLamp::ACoopLamp()
{
	bReplicates = true;
	SetReplicateMovement(false);
	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	RootComponent = Light;
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetCastShadows(false);
	Light->SetIntensityUnits(ELightUnits::Lumens);
	OnRep_Lamp();
}

void ACoopLamp::Setup(FLinearColor InColor, float InLumens, float InRadius)
{
	Col = InColor; Lumens = InLumens; Radius = InRadius;
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
}
