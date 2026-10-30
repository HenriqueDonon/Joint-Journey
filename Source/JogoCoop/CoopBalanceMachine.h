#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopBalanceMachine.generated.h"

// Estado replicado da maquina (o visual e calculado igual em todas as maquinas a partir dele e da hora do servidor)
USTRUCT()
struct FBalState
{
	GENERATED_BODY()
	UPROPERTY() uint8 L0 = 0;      // posicao de cada lingote movel: 0 = gaiola norte | 1 = berco do hub | 2 = gaiola sul
	UPROPERTY() uint8 L1 = 0;
	UPROPERTY() uint8 L2 = 0;
	UPROPERTY() uint8 S0 = 0;      // vaga na gaiola sul (ordem de chegada)
	UPROPERTY() uint8 S1 = 0;
	UPROPERTY() uint8 S2 = 0;
	UPROPERTY() float P0 = -100.f; // deslocamento da plataforma NORTE (cm; a sul faz o oposto) no inicio do trecho
	UPROPERTY() float T0 = 0.f;    // inicio do trecho (hora do servidor)
	UPROPERTY() float Target = -100.f;
	UPROPERTY() bool bBrakeFree = false;
	UPROPERTY() bool bLocked = false;
	UPROPERTY() bool bDone = false;
	UPROPERTY() uint8 Resets = 0;
	UPROPERTY() bool bReturning = false;   // rearme: plataformas voltam devagar a posicao emperrada
};

// PUZZLE 2 - CAMARA DAS DUAS MASSAS (Fase2_Funcional, setor B). Ator do nivel, replicado; o servidor e a autoridade.
// Duas plataformas suspensas ligadas por cabo (uma sobe, a outra desce). Deslocamento-alvo = 100 cm x (massa sul - massa norte).
// Norte (Ruina): freio emperrado + 3 bracadeiras que prendem 3 lingotes. Sul (Neon): 1 lingote soldado, calha de transferencia
// quebrada e pino de travamento ausente. Destrocos de cada peca destruida ficam no proprio local e sao o UNICO material:
// cada reconstrucao do Neon consome 1 pilha (a mais antiga) e a animacao leva os destrocos dela ate a peca.
UCLASS()
class JOGOCOOP_API ACoopBalanceMachine : public AActor
{
	GENERATED_BODY()
public:
	ACoopBalanceMachine();
	virtual void BeginPlay() override;
	virtual void Tick(float Dt) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool ServerReset(FString& OutWhy);          // rearme (console em cada sala); nao vale depois de concluido
	bool IsDone() const { return St.bDone; }
	float PlatformOffset() const;               // deslocamento atual da plataforma norte (cm)
	int32 MassNorth() const;
	int32 MassSouth() const;                    // inclui o lingote soldado
	int32 MaterialPiles() const { return Piles.Num(); }
	const FBalState& State() const { return St; }

	UPROPERTY(Replicated) class ACoopDestructible* Brake = nullptr;
	UPROPERTY(Replicated) class ACoopDestructible* Clamp0 = nullptr;
	UPROPERTY(Replicated) class ACoopDestructible* Clamp1 = nullptr;
	UPROPERTY(Replicated) class ACoopDestructible* Clamp2 = nullptr;
	UPROPERTY(Replicated) class ACoopDestructible* Chute = nullptr;
	UPROPERTY(Replicated) class ACoopDestructible* Pin = nullptr;

	static constexpr float LevelZ = 900.f;   // altura do topo das plataformas no nivel
	static constexpr float Speed = 60.f;     // cm/s
	static constexpr float Range = 200.f;    // deslocamento maximo
	static constexpr float PerUnit = 100.f;  // cm por unidade de diferenca de massa
	static constexpr float JamP = -100.f;    // posicao emperrada inicial
	static constexpr float PinTol = 6.f;     // tolerancia do pino
private:
	UPROPERTY(Replicated) FBalState St;
	float Now() const;
	float PAt(float T) const;
	uint8 Loc(int32 i) const { return i == 0 ? St.L0 : i == 1 ? St.L1 : St.L2; }
	void SetLoc(int32 i, uint8 V) { (i == 0 ? St.L0 : i == 1 ? St.L1 : St.L2) = V; }
	uint8 SSlot(int32 i) const { return i == 0 ? St.S0 : i == 1 ? St.S1 : St.S2; }
	void SetSSlot(int32 i, uint8 V) { (i == 0 ? St.S0 : i == 1 ? St.S1 : St.S2) = V; }
	class ACoopDestructible* ClampOf(int32 i) const { return i == 0 ? Clamp0 : i == 1 ? Clamp1 : Clamp2; }
	// servidor
	void SpawnParts();
	void Recalc();
	void ServerTick();
	void FlowLingotes();
	void Complete();
	TArray<class ACoopDestructible*> Piles;     // pilhas de destrocos disponiveis (ordem de criacao)
	bool bPrevBrake = false, bPrevClamp[3] = { false, false, false }, bPrevChute = true, bPrevPin = true, bReleased[3] = { false, false, false };
	// visual local
	void Build();
	void ApplyVisual(float Dt);
	FVector LingTarget(int32 i, float P) const;
	UPROPERTY() TArray<class UStaticMeshComponent*> PlatN;   // plataforma norte + ponteiros
	UPROPERTY() TArray<class UStaticMeshComponent*> PlatS;
	UPROPERTY() TArray<class UStaticMeshComponent*> CablesN;
	UPROPERTY() TArray<class UStaticMeshComponent*> CablesS;
	UPROPERTY() TArray<class UStaticMeshComponent*> Ling;     // 0..2 moveis, 3 soldado
	UPROPERTY() class UStaticMeshComponent* NeedleN = nullptr;
	UPROPERTY() class UStaticMeshComponent* NeedleS = nullptr;
	UPROPERTY() TArray<class UStaticMeshComponent*> Sparks;
	UPROPERTY() TArray<class UPointLightComponent*> StatusLights;
	UPROPERTY() class UMaterialInstanceDynamic* DialMat = nullptr;
	TArray<FVector> PlatNRel, PlatSRel;                       // posicoes relativas ao topo da plataforma (cm)
	TArray<FVector> LingPos;                                  // posicao visual atual (suavizada)
	bool bVisInit = false; int32 LastResets = -1;
};
