#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopFog.generated.h"

// Neblina/fumaca de ambiente (exponential height fog), replicada como o CoopSun.
UCLASS()
class JOGOCOOP_API ACoopFog : public AActor
{
	GENERATED_BODY()
public:
	ACoopFog();
	UPROPERTY(VisibleAnywhere) class UExponentialHeightFogComponent* Fog;
};
