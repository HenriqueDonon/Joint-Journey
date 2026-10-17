#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopProp.generated.h"

// Cubo manipulável: Neon inverte a gravidade, Ruína congela e rebobina. Estado replicado + efeitos visuais.
UCLASS()
class JOGOCOOP_API ACoopProp : public AActor
{
	GENERATED_BODY()
public:
	ACoopProp();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Chamar apenas no servidor
	void FlipGravity();
	void ToggleFreeze();
	void StartRewind();
	bool IsReversed() const { return bReversed; }
	bool IsFrozen() const { return bFrozen; }

	UFUNCTION(NetMulticast, Unreliable) void MulticastBurst(FLinearColor Col, FVector From);

	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	UPROPERTY(EditAnywhere) float RewindSeconds = 5.f;
	UPROPERTY(EditAnywhere) float ReverseAccel = 400.f; // aceleração líquida para cima (cm/s²) com gravidade invertida

	// 0 normal, 1 gravidade invertida, 2 congelado, 3 rebobinando
	UPROPERTY(ReplicatedUsing=OnRep_State) uint8 State = 0;
	UFUNCTION() void OnRep_State();

	// Posição/rotação enviadas pelo servidor; os clientes só interpolam (sem simular física)
	UPROPERTY(Replicated) FVector_NetQuantize10 RepPos;
	UPROPERTY(Replicated) float RepYaw = 0.f;
	UPROPERTY(Replicated) bool bHasRep = false;

private:
	bool bReversed = false, bFrozen = false, bRewinding = false;
	float AwakeUntil = 0.f;
	FVector SpawnLoc = FVector::ZeroVector;
	TArray<FTransform> History;

	UPROPERTY() UMaterialInstanceDynamic* BodyMat = nullptr;
	UPROPERTY() TArray<UStaticMeshComponent*> Arrows;
	UPROPERTY() TArray<UStaticMeshComponent*> Shards;
	UPROPERTY() TArray<UStaticMeshComponent*> Orbit;
	UPROPERTY() USceneComponent* OrbitPivot = nullptr;

	void SetPhysics(bool bOn);
	void RefreshState();
	void ApplyVis();
	void UpdateFx(float DT);
};
