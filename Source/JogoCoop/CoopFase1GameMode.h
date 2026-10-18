#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CoopFase1GameMode.generated.h"

// Mapa de TESTE da Fase 1 (blockout): plataforma inicial -> vao com lava -> plataforma de saida.
// Mapa Sala1 (original) NAO e tocado. Sem poderes/puzzle aqui: so geometria, navegacao e medidas.
// Medidas (cm, 1uu = 1cm). Origem X=0 = borda da plataforma inicial; Z=0 = piso da plataforma inicial.
UCLASS()
class JOGOCOOP_API ACoopFase1GameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ACoopFase1GameMode();
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	// Teste manual (tecla F4): forca ponte inteira/quebrada (ignora o material compartilhado). Retorna true se ficou inteira.
	bool ToggleTestBridge();
	virtual void Tick(float DeltaSeconds) override;
private:
	void BuildLevel();
	void CheckFall();
	void SpawnBarrier();
	void BuildDecor();
	void BuildTransition();
	void TickCollapse();
	void SpawnCrackCluster(int32 Idx);
	void ClearCracks();
	void SendAllToStart();
	void Announce(const FString& Text);
	UPROPERTY() TArray<class ACoopCeiling*> Roof;
	UPROPERTY() TArray<AActor*> Cracks;
	int32 Phase = 0;          // 0 estavel | 1 contagem (8s) | 2 desabando
	float PhaseT0 = 0.f;
	int32 CracksSpawned = 0;
	int32 LastSecShown = -1;
	UPROPERTY() TArray<AActor*> Starts;
	UPROPERTY() class ACoopDestructible* Column = nullptr;
	UPROPERTY() class ACoopDestructible* Bridge = nullptr;
	UPROPERTY() AActor* Barrier = nullptr;
	UPROPERTY() class ACoopExitDoor* ExitDoor = nullptr;
	UPROPERTY() class ACoopExitDoor* ExitDoor2 = nullptr;
	FVector ExitCenter = FVector::ZeroVector;
	void TickExit();
	bool bDoorLatched = false, bPhaseDone = false;
	FTimerHandle FallTimer;
	int32 NextStart = 0;
	bool bBuilt = false;
};
