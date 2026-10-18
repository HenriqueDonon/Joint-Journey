#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CoopGameMode.generated.h"

UCLASS()
class JOGOCOOP_API ACoopGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ACoopGameMode();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	void RoomCompleted(int32 RoomIndex);
private:
	void BuildRoom(int32 Index, FVector Origin);
	void TeleportToRoom2();
	UPROPERTY() TArray<AActor*> Starts;
	TArray<FVector> Room2Starts;
	FTimerHandle TeleportTimer;
	int32 NextStart = 0;
	bool bBuilt = false;
};
