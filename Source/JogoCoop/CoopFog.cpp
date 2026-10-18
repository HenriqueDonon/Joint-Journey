#include "CoopFog.h"
#include "Components/ExponentialHeightFogComponent.h"

ACoopFog::ACoopFog()
{
	bReplicates = true;
	Fog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("Fog"));
	RootComponent = Fog;
	Fog->SetFogDensity(0.012f);
	Fog->SetFogHeightFalloff(0.08f);
	Fog->SetFogInscatteringColor(FLinearColor(0.32f, 0.17f, 0.10f)); // fumaca quente
	Fog->SetFogMaxOpacity(0.85f);
	Fog->SetStartDistance(250.f);
}
