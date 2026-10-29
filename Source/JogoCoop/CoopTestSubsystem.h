#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CoopTestSubsystem.generated.h"

// SO TESTES: nao altera mapas. Com -F1Tour cria um passeio de camera gravado quadro a quadro; com -F1Fluxo cria o
// roteiro automatico do puzzle da Fase 1 (cada instancia controla o proprio personagem). Sem esses parametros, nada.
UCLASS()
class JOGOCOOP_API UCoopTestSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};

UCLASS()
class JOGOCOOP_API ACoopF1Tour : public AActor
{
	GENERATED_BODY()
public:
	ACoopF1Tour() { PrimaryActorTick.bCanEverTick = true; }
	virtual void Tick(float Dt) override;
private:
	float T = 0.f; int32 Frame = 0; bool bStarted = false; FString Tag;
	UPROPERTY() class ACameraActor* Cam = nullptr;
};

UCLASS()
class JOGOCOOP_API ACoopF1Flow : public AActor
{
	GENERATED_BODY()
public:
	ACoopF1Flow() { PrimaryActorTick.bCanEverTick = true; }
	virtual void Tick(float Dt) override;
private:
	int32 Ph = 0, Shots = 0; float PhT = 0.f, ShotT = 0.f, SaveT = 0.f, Total = 0.f; FString Tag, Log;
	void SetPh(int32 P, const FString& Why);
	void Save();
	void Fire(class ACoopCharacter* Me, class ACoopDestructible* T);
};
