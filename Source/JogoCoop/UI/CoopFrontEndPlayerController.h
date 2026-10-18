#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CoopFrontEndPlayerController.generated.h"

UCLASS()
class JOGOCOOP_API ACoopFrontEndPlayerController : public APlayerController
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY() class UCoopMainMenuWidget* MenuWidget;
};
