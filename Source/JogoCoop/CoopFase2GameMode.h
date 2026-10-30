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
	// ---- layout FUNCIONAL (mapa experimental com marcador F2X_Layout); no Fase2_Blockout nada disso e ativado ----
	void HandleInteract(class ACoopCharacter* C);     // tecla E (servidor): pula a apresentacao ou usa o console mais proximo
	void DebugUnlockAll();                             // F10 (DEPURACAO): abre portoes e destrava o elevador; NAO e solucao de puzzle
	bool IsFunctionalLayout() const { return bF2X; }
	bool bDebugUnlocked = false;
	// ---- PUZZLE 1: Elevador de Emergencia (trava -> material -> suporte -> elevador liberado) ----
	UPROPERTY() class ACoopDestructible* P1Trava = nullptr;
	UPROPERTY() class ACoopDestructible* P1Suporte = nullptr;
	bool bP1Done = false, bP1MatMsg = false;
	static bool DebugAllowed();   // F6..F10 so com -F2Debug (nunca no fluxo normal)
private:
	void TeleportTo(APlayerController* PC, int32 Level);
	int32 NextStart = 0;
	// layout funcional
	bool bF2XChecked = false, bF2X = false;
	void SetupF2X();
	void TickF2X(float Dt);
	void SetupP1();
	void TickP1();
	FVector CheckSpot(int32 Level, const class ACoopCharacter* C) const;
	void EndIntroFor(APlayerController* PC);
	UPROPERTY() class ACoopIntroCam* Intro = nullptr;
	UPROPERTY() TArray<APlayerController*> InIntro;
	bool bIntroStarted = false; float F2XTime = 0.f;
	TMap<TWeakObjectPtr<APawn>, float> FallStart;     // Z do inicio da queda
	TMap<TWeakObjectPtr<APawn>, int32> MaxLevel;      // maior nivel em que pisou
	TMap<FName, FVector> Spots;                       // F2X_Check_A / _B_Ruina / _B_Neon / _C / _D
};
