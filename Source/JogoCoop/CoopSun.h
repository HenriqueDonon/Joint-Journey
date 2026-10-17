#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopSun.generated.h"

// Sol + luz de céu + atmosfera. Replicado, então existe nos dois jogadores.
UCLASS()
class JOGOCOOP_API ACoopSun : public AActor
{
	GENERATED_BODY()
public:
	ACoopSun();
	UPROPERTY(VisibleAnywhere) class UDirectionalLightComponent* Sun;
	UPROPERTY(VisibleAnywhere) class USkyLightComponent* Sky;
	UPROPERTY(VisibleAnywhere) class USkyAtmosphereComponent* Atmos;
};
