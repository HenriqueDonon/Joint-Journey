#include "CoopSun.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"

ACoopSun::ACoopSun()
{
	bReplicates = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetMobility(EComponentMobility::Movable);

	Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
	Sun->SetupAttachment(Root);
	Sun->SetMobility(EComponentMobility::Movable);
	Sun->SetIntensity(2.0f);
	Sun->SetAtmosphereSunLight(true);
	Sun->SetRelativeRotation(FRotator(-50.f, -35.f, 0.f));

	Sky = CreateDefaultSubobject<USkyLightComponent>(TEXT("Sky"));
	Sky->SetupAttachment(Root);
	Sky->SetMobility(EComponentMobility::Movable);
	Sky->SetIntensity(0.5f);
	Sky->bRealTimeCapture = true;

	Atmos = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("Atmos"));
	Atmos->SetupAttachment(Root);
}
