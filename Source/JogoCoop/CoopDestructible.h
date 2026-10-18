#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopDestructible.generated.h"

// Objeto que a Ruina pode destruir e o Neon pode reconstruir (ciclo repetivel).
// Estado autoritativo no servidor (bDestroyed replicado); cada maquina aplica o visual/colisao no OnRep.
// Para criar novos objetos: spawnar esta classe e chamar Setup() com tamanho/cor, ou herdar e trocar a malha.
UCLASS()
class JOGOCOOP_API ACoopDestructible : public AActor
{
	GENERATED_BODY()
public:
	ACoopDestructible();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Setup(FVector InSize, FLinearColor InColor); // antes do BeginPlay ou no servidor
	bool IsDestroyed() const { return bDestroyed; }

	// Servidor: retornam false (com motivo) se a acao nao for valida no estado atual.
	bool ServerBreak(FString& OutWhy);
	bool ServerRebuild(FString& OutWhy);

	UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Mesh;
	UPROPERTY(Replicated) FVector Size = FVector(220.f, 60.f, 200.f);
	UPROPERTY(Replicated) FLinearColor Color = FLinearColor(0.35f, 0.33f, 0.32f);

private:
	UPROPERTY(ReplicatedUsing=OnRep_Destroyed) bool bDestroyed = false;
	UFUNCTION() void OnRep_Destroyed();
	void ApplyState(bool bWithFx);
	void BuildChunks();

	UPROPERTY() TArray<class UStaticMeshComponent*> Chunks;
	UPROPERTY() class UMaterialInstanceDynamic* BodyMat = nullptr;
	bool bApplied = false;
};
