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
	UPROPERTY(EditAnywhere, Category="Elevador") float LeadSeconds = 0.6f;    // antecedencia do T0 (cliente recebe antes; portas fecham antes de partir)
	UPROPERTY(EditAnywhere, Category="Elevador") FVector2D HalfArea = FVector2D(90.f, 90.f);   // area "a bordo" (cm): centro do personagem; a capsula (r=34) nao encosta nas portas (Y/X 125)

	// ---- PORTAS (simples, sem animacao elaborada): paineis sobem para abrir; so abrem com o elevador PARADO ----
	UPROPERTY(EditAnywhere, Category="Portas") bool bSideDoorsOpenAtTop = true;     // portas esquerda/direita (+Y/-Y): abrem parado no andar de separacao (topo)
	UPROPERTY(EditAnywhere, Category="Portas") bool bFrontGateOpenAtBottom = true;  // portao de entrada (-X): abre parado embaixo
	UPROPERTY(EditAnywhere, Category="Portas") float DoorSeconds = 0.35f;           // tempo de abrir/fechar
	UPROPERTY(EditAnywhere, Category="Portas") float DoorOpenDelay = 0.3f;          // atraso apos parar para comecar a abrir
	float SideDoorAlpha() const { return SideAlpha; }

	// ---- CONTROLE EXPLICITO (Fase 2 funcional): sem partida automatica; so parte por pedido validado no servidor (console) ----
	UPROPERTY(EditAnywhere, Category="Controle") bool bExplicitControl = false;   // false = modo antigo (todos a bordo por DwellSeconds) do mapa de teste
	UPROPERTY(EditAnywhere, Category="Controle") bool bLandingDoors = false;      // portas fixas de patamar (embaixo -X, em cima +-Y): fecham o poco sem a cabine
	// Trava do mecanismo (Puzzle 1): enquanto true, nenhum pedido e aceito. O Puzzle 1 (futuro) chama ServerSetLocked(false).
	UPROPERTY(EditAnywhere, Replicated, Category="Controle") bool bLockedP1 = false;
	void ServerSetLocked(bool b);
	bool ServerRequestGo(bool bUp, FString& OutWhy);   // pedido de viagem (valida: parado, destravado, outro andar, porta livre)
	bool IsParkedAt(bool bTop) const { return !IsMoving() && Cmd.bUp == bTop; }
	float FrontGateAlpha() const { return FrontAlpha; }

	bool IsMoving() const;
	bool IsTargetTop() const { return Cmd.bUp; }
	float CurrentOffsetZ() const;     // 0..Rise
	FVector GetHome() const { return Home; }
	int32 NumAboard() const;          // jogadores sobre a plataforma (valido no servidor)

private:
	UPROPERTY(ReplicatedUsing=OnRep_Cmd) FElevCmd Cmd;
	UFUNCTION() void OnRep_Cmd() {}
	UPROPERTY() class UStaticMeshComponent* DoorN = nullptr;   // +Y
	UPROPERTY() class UStaticMeshComponent* DoorS = nullptr;   // -Y
	UPROPERTY() class UStaticMeshComponent* GateW = nullptr;   // -X (entrada)
	UPROPERTY() class UStaticMeshComponent* WallE = nullptr;   // +X (fixa)
	UPROPERTY() class UStaticMeshComponent* LandA = nullptr;   // patamar inferior (-X), fixo no mundo
	UPROPERTY() class UStaticMeshComponent* LandN = nullptr;   // patamar superior (+Y)
	UPROPERTY() class UStaticMeshComponent* LandS = nullptr;   // patamar superior (-Y)
	float LandBotAlpha = 1.f, LandTopAlpha = 0.f;
	float SideAlpha = 0.f, FrontAlpha = 1.f;
	void UpdateDoors(float Dt);
	FVector Home = FVector::ZeroVector;
	float DwellAcc = 0.f;
	bool bArmed = true;
	float NowSrv() const;
	float Alpha() const;
	void ServerLogic(float Dt);
};
