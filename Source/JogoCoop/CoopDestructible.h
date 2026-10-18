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

	// InDrop/InSpreadX: onde caem os escombros (queda em cm e espalhamento em X; 0 = padrao). bStartDestroyed: nasce destruido.
	void Setup(FVector InSize, FLinearColor InColor, float InDrop = 0.f, float InSpreadX = 0.f, bool bStartDestroyed = false);
	bool IsDestroyed() const { return bDestroyed; }

	// Servidor: retornam false (com motivo) se a acao nao for valida no estado atual.
	bool ServerBreak(FString& OutWhy);
	bool ServerRebuild(FString& OutWhy);
	void ServerForce(bool bBreak); // teste (F4): ignora regras

	// Material compartilhado: so reconstroi se o Partner estiver destruido (coluna <-> ponte).
	UPROPERTY() ACoopDestructible* Partner = nullptr;

	UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Mesh;
	UPROPERTY(Replicated) FVector Size = FVector(220.f, 60.f, 200.f);
	UPROPERTY(Replicated) FLinearColor Color = FLinearColor(0.35f, 0.33f, 0.32f);
	UPROPERTY(Replicated) FName Model = NAME_None; // /Game/Fase1/<Model>/SM_<Model> (origem = centro da base)
	UPROPERTY() class UStaticMeshComponent* ModelComp = nullptr;
	UPROPERTY(Replicated) float DebrisDrop = 0.f;
	UPROPERTY(Replicated) float DebrisSpreadX = 0.f;

private:
	UPROPERTY(ReplicatedUsing=OnRep_Destroyed) bool bDestroyed = false;
	UFUNCTION() void OnRep_Destroyed();
	void ApplyState(bool bWithFx);
	void BuildChunks();

	UPROPERTY() TArray<class UStaticMeshComponent*> Chunks;
	UPROPERTY() class UMaterialInstanceDynamic* BodyMat = nullptr;
	bool bApplied = false;
};
