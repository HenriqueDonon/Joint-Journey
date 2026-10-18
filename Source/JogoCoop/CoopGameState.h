#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CoopGameState.generated.h"

// Guarda quantos jogadores a campanha atual precisa (2 ou 3), escolhido no menu principal.
UCLASS()
class JOGOCOOP_API ACoopGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Replicated) int32 RequiredPlayers = 2;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
