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
private:
	void BuildLevel();
	void CheckFall();
	void SpawnBarrier();
	UPROPERTY() TArray<AActor*> Starts;
	UPROPERTY() class ACoopDestructible* Column = nullptr;
	UPROPERTY() class ACoopDestructible* Bridge = nullptr;
	UPROPERTY() AActor* Barrier = nullptr;
	FTimerHandle FallTimer;
	int32 NextStart = 0;
	bool bBuilt = false;
};
