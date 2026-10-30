#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopF2Test.generated.h"

// TESTES AUTOMATICOS da Fase 2 funcional (so com -F2Fluxo / -F2Vistas; nada disso roda no jogo normal).
// Fluxo: cada instancia (host = Neon, cliente = Ruina) dirige o proprio personagem pelo caminho REAL do jogo
// (movimento + tecla E via ServerInteract). Unico atalho de teste: o host destrava o elevador e abre os portoes pelas
// funcoes de DEPURACAO (P1/P2/P3 ainda nao existem) - registrado no log.
UCLASS()
class JOGOCOOP_API ACoopF2Flow : public AActor
{
	GENERATED_BODY()
public:
	ACoopF2Flow() { PrimaryActorTick.bCanEverTick = true; }
	virtual void Tick(float Dt) override;
private:
	int32 St = 0, Shots = 0, Wp = 0; float StT = 0.f, ShotT = 0.f, LogT = 0.f, Total = 0.f; FString Tag, Log; bool bDone = false;
	float Rec = 0.f; bool bFlag = false;
	void Go(int32 S, const FString& Why);
	void Save();
};

UCLASS()
class JOGOCOOP_API ACoopF2Views : public AActor
{
	GENERATED_BODY()
public:
	ACoopF2Views() { PrimaryActorTick.bCanEverTick = true; }
	virtual void Tick(float Dt) override;
private:
	float T = 0.f; int32 S = 0;
	UPROPERTY() class ACameraActor* Cam = nullptr;
};
