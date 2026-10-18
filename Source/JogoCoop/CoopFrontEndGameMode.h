#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CoopFrontEndGameMode.generated.h"

// GameMode do mapa de menu principal: sem pawn, so mostra a UI.
UCLASS()
class JOGOCOOP_API ACoopFrontEndGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ACoopFrontEndGameMode();
};
