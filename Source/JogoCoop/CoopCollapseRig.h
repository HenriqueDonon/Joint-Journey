#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopCollapseRig.generated.h"

// Estado replicado do colapso (o servidor so muda o estagio; todo o visual e calculado igual em cada maquina a partir da hora do servidor)
USTRUCT()
struct FRigState
{
	GENERATED_BODY()
	UPROPERTY() uint8 Stage = 0;     // 0 estavel | 1 instavel (contagem) | 2 desabando | 3 estabilizando (coluna reconstruida)
	UPROPERTY() float T0 = 0.f;      // inicio da instabilidade (hora do servidor)
	UPROPERTY() float Dur = 12.f;    // tempo para reconstruir (vem do GameMode)
	UPROPERTY() float T1 = 0.f;      // inicio da queda / da estabilizacao
	UPROPERTY() float StressAtT1 = 0.f; // nivel de tensao quando estabilizou (para decair suave)
};

// ESTRUTURA SUSTENTADA PELA COLUNA + COLAPSO PROGRESSIVO (layout 2 da Fase 1 / Fase1_Arquitetura).
// Ator colocado no mapa (existe no servidor e nos clientes). Monta em BeginPlay: deck do mezanino sobre a coluna, capitel,
// torre de transferencia ate uma secao pesada do teto, vigas, tirantes e ligacao com as trelicas principais.
// Efeitos (vibracao, cedencia, rachaduras, poeira, faiscas, pecas e detritos caindo, queda final) sao deterministicos
// a partir de FRigState -> sincronizados sem replicar transformacoes. Nao tem colisao (so visual; nao prende jogadores).
UCLASS()
class JOGOCOOP_API ACoopCollapseRig : public AActor
{
	GENERATED_BODY()
public:
	ACoopCollapseRig();
	virtual void BeginPlay() override;
	virtual void Tick(float Dt) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;

	// geometria (cm, coordenadas do mundo). Ator posicionado na base da coluna.
	UPROPERTY(EditAnywhere, Category="Rig") float ColumnTopZ = 616.f;
	UPROPERTY(EditAnywhere, Category="Rig") FVector2D DeckMin = FVector2D(0, -1410);
	UPROPERTY(EditAnywhere, Category="Rig") FVector2D DeckMax = FVector2D(1167, -330);
	UPROPERTY(EditAnywhere, Category="Rig") FVector2D BayMin = FVector2D(-150, -1395);
	UPROPERTY(EditAnywhere, Category="Rig") FVector2D BayMax = FVector2D(1400, -150);
	UPROPERTY(EditAnywhere, Category="Rig") float BayZ = 1300.f;
	UPROPERTY(EditAnywhere, Category="Rig") float BayThick = 110.f;
	UPROPERTY(EditAnywhere, Category="Rig") float TrussZ = 1540.f;
	UPROPERTY(EditAnywhere, Category="Rig") float FloorZ = 0.f;
	UPROPERTY(EditAnywhere, Category="Rig") float WallY = -1400.f;
	// camera da cena de derrota (vista ampla da area principal) e altura final da secao desabada
	UPROPERTY(EditAnywhere, Category="Rig") FVector LoseCamPos = FVector(1750, 1100, 900);
	UPROPERTY(EditAnywhere, Category="Rig") FVector LoseCamTarget = FVector(450, -100, 200);
	UPROPERTY(EditAnywhere, Category="Rig") float RestH = 140.f;
	UPROPERTY(VisibleAnywhere) class UCameraComponent* LoseCam;

	// API do servidor (GameMode)
	void ServerBeginInstability(float Duration);
	void ServerStabilize();
	void ServerBeginFall();
	void ServerReset();
	float ImpactDelay() const { return Lead + BayFallTime(); }   // tempo do inicio da queda ate o impacto principal
	float BayFallTime() const;

private:
	UPROPERTY(ReplicatedUsing=OnRep_State) FRigState St;
	UFUNCTION() void OnRep_State() {}
	float Now() const;
	static constexpr float Lead = 0.15f;
	struct FPiece { class UStaticMeshComponent* C = nullptr; FVector Rest; FRotator RestRot; FVector RestScale; int32 Group = 0; float Detach = 2.f; FVector Land; float Phase = 0.f; };
	TArray<FPiece> P;           // grupos: 0 ancora (fica) | 1 deck/capitel | 2 torre (comprime) | 3 secao do teto+vigas | 4 secundaria (solta antes) | 5 rachadura | 6 trelicas do vao (2a onda)
	// particulas leves como componentes simples (os materiais do glTF nao suportam instancias)
	UPROPERTY() TArray<class UStaticMeshComponent*> DustC;
	UPROPERTY() TArray<class UStaticMeshComponent*> SparkC;
	UPROPERTY() TArray<class UStaticMeshComponent*> DebrisC;
	void SetP(class UStaticMeshComponent* C, const FTransform& X);
	UPROPERTY() class UPointLightComponent* SparkLight = nullptr;
	UPROPERTY() class UPointLightComponent* ImpactLight = nullptr;
	UPROPERTY() TArray<class UPointLightComponent*> ServiceLights;
	UPROPERTY() TArray<class USpotLightComponent*> BayLamps; TArray<int32> BayLampPiece;
	FVector CamRest = FVector::ZeroVector; FRotator CamRot = FRotator::ZeroRotator;
	TArray<FVector> Joints;      // pontos de tensao (poeira/faiscas)
	TArray<FVector> DebrisFrom; TArray<float> DebrisAt; TArray<float> DebrisLandZ;
	FVector Pivot = FVector::ZeroVector;
	float PrevStress = 0.f;
	class UStaticMeshComponent* Add(int32 Group, class UStaticMesh* M, class UMaterialInterface* Mat, FVector Loc, FVector Size, FRotator Rot = FRotator::ZeroRotator, bool bModelBase = false);
	void Build();
	float Stress(float T) const;     // 0..1 tensao visual
	void ApplyVisual(float T, float Dt);
};
