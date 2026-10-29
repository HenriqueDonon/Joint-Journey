#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopElevator.generated.h"

// Comando replicado do elevador: destino (topo/base) e instante (hora do SERVIDOR) em que o movimento comeca.
USTRUCT()
struct FElevCmd
{
	GENERATED_BODY()
	UPROPERTY() bool bUp = false;
	UPROPERTY() float T0 = -1000.f;
};

// ELEVADOR (Fase 2, prototipo isolado): plataforma que sobe/desce Rise cm transportando os jogadores.
// - So o SERVIDOR decide (acionamento: todos os jogadores a bordo por DwellSeconds; rearma quando a plataforma fica vazia).
// - Replica apenas o comando {destino, T0}. Posicao = funcao deterministica da hora do servidor (GameState), calculada
//   igualmente em servidor e clientes: movimento suave e identico, sem replicar posicao a cada frame.
// - Plataforma kinematica (Movable): os personagens a usam como base de movimento (CharacterMovement) nas duas pontas.
UCLASS()
class JOGOCOOP_API ACoopElevator : public AActor
{
	GENERATED_BODY()
public:
	ACoopElevator();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Platform;
	UPROPERTY(EditAnywhere, Category="Elevador") float Rise = 600.f;          // curso (cm)
	UPROPERTY(EditAnywhere, Category="Elevador") float TravelSeconds = 5.f;   // duracao do curso
	UPROPERTY(EditAnywhere, Category="Elevador") float DwellSeconds = 1.5f;   // tempo com todos a bordo antes de partir
	UPROPERTY(EditAnywhere, Category="Elevador") float LeadSeconds = 0.4f;    // antecedencia do T0 (cliente recebe antes de comecar)
	UPROPERTY(EditAnywhere, Category="Elevador") FVector2D HalfArea = FVector2D(110.f, 110.f); // area "a bordo" (cm)

	bool IsMoving() const;
	bool IsTargetTop() const { return Cmd.bUp; }
	float CurrentOffsetZ() const;     // 0..Rise
	FVector GetHome() const { return Home; }
	int32 NumAboard() const;          // jogadores sobre a plataforma (valido no servidor)

private:
	UPROPERTY(ReplicatedUsing=OnRep_Cmd) FElevCmd Cmd;
	UFUNCTION() void OnRep_Cmd() {}
	FVector Home = FVector::ZeroVector;
	float DwellAcc = 0.f;
	bool bArmed = true;
	float NowSrv() const;
	float Alpha() const;
	void ServerLogic(float Dt);
};
