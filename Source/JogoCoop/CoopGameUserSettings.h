#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "CoopGameUserSettings.generated.h"

// Configuracoes comuns do jogo (persistem em Saved/Config/.../GameUserSettings.ini).
UCLASS()
class JOGOCOOP_API UCoopGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(Config) float MouseSensitivity = 1.f;
	UPROPERTY(Config) float MasterVolume = 1.f;

	static UCoopGameUserSettings* Get();
	void ApplyAudio();
};
