#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CoopHUD.generated.h"

// Mira em cruz no centro da tela + indicador de modo (Poderes / Portais)
UCLASS()
class JOGOCOOP_API ACoopHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
};
