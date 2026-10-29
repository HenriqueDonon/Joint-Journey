#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopElevTest.generated.h"

// SO PARA TESTE (mapa Fase2_ElevTeste): com -ElevAuto, o personagem LOCAL de cada instancia faz sozinho o roteiro:
// embarca, sobe ao andar de separacao, testa a rota (certa/errada/fica a bordo), retorna, desce, sai. Grava log + capturas
// em AutoShots/elev/<ElevTag>. -ElevSwap: host vira Ruina e cliente vira Neon (papeis invertidos). Sem -ElevAuto nao faz nada.
UCLASS()
class JOGOCOOP_API ACoopElevTest : public AActor
{
	GENERATED_BODY()
public:
	ACoopElevTest() { PrimaryActorTick.bCanEverTick = true; }
	virtual void BeginPlay() override;
	virtual void Tick(float Dt) override;
private:
	bool bAuto = false, bSwap = false, bSwapped = false; FString Tag = TEXT("solo");
	int32 Ph = 0, Cycles = 0, Shots = 0;
	float PhT = 0.f, LogT = 0.f, ShotT = 0.f, SaveT = 0.f;
	float RouteMaxAbsY = 0.f;
	FString Log, Results;
	TWeakObjectPtr<class ACoopElevator> Elev;
	float MinRel = 1e9f, MaxRel = -1e9f, MaxDrift = 0.f; FVector RefRel = FVector::ZeroVector; bool bHaveRef = false;
	void SetPh(int32 P, const TCHAR* Why);
	void Save();
	int32 PlanType() const;   // 0 rota certa | 1 rota errada | 2 Neon fica a bordo, Ruina vai e volta
};
