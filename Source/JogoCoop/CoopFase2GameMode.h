#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CoopFase2GameMode.generated.h"

// FASE 2 - BLOCKOUT do Poco dos Contrapesos (mapa /Game/Maps/Fase2_Blockout). So geometria: SEM puzzles/elevadores.
// Aqui ficam apenas: pawn/HUD padrao, escolha dos PlayerStart do mapa, recuperacao de queda e atalhos TEMPORARIOS
// de inspecao (F6..F9 = teleporta para os niveis 0/600/1200/1800) + modo de teste -Fase2Shot (capturas e log).
UCLASS()
class JOGOCOOP_API ACoopFase2GameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ACoopFase2GameMode();
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void Tick(float DeltaSeconds) override;
private:
	void TeleportTo(APlayerController* PC, int32 Level);
	int32 NextStart = 0;
};
