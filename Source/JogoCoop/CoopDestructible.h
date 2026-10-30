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
	void ServerForce(bool bBreak, bool bFx = true); // teste (F4)/reinicio: ignora regras; bFx=false: sem efeito/animacao

	// Material compartilhado: so reconstroi se o Partner estiver destruido (coluna <-> ponte).
	UPROPERTY(Replicated) ACoopDestructible* Partner = nullptr;
	// Animacao de montagem na reconstrucao (layout 2): pecas saem dos destrocos do Partner e se encaixam
	// (horizontal = ponte: segmentos em sequencia | vertical = coluna: aneis empilhando de baixo para cima)
	UPROPERTY(Replicated) bool bAssemblyFx = false;
	void TakeDebris(TArray<FVector>& Out);   // local: entrega as posicoes dos destrocos visiveis e os esconde
	virtual void Tick(float Dt) override;

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
	// Fragmentos reais do modelo (cortados no Blender, /Game/Fase1/<Model>Frag/...): so visual, fisica local em cada maquina
	UPROPERTY() TArray<class UStaticMesh*> FragMeshes;
	UPROPERTY() TArray<class UStaticMeshComponent*> Frags;
	UPROPERTY() TArray<class UBoxComponent*> FragBodies; // corpo fisico (caixa) de cada fragmento; a malha vai presa nele
	FTimerHandle FragTimer;
	void SpawnFrags();
	void SettleFrags();
	void ClearFrags();
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	UPROPERTY() class UMaterialInstanceDynamic* BodyMat = nullptr;
	bool bApplied = false;
	UPROPERTY(Replicated) bool bQuietChange = false;
	struct FAsm { class UStaticMeshComponent* C = nullptr; FVector From, To, Scale; float Delay = 0.f; };
	TArray<FAsm> Asm;
	UPROPERTY() class UStaticMeshComponent* AsmBeam = nullptr;
	UPROPERTY() class UPointLightComponent* AsmLight = nullptr;
	float AsmT0 = -1.f, AsmEnd = 0.f, AsmFlight = 0.45f; bool bAsmHoriz = true;
	void StartAssembly();
	void ClearAssembly();
	void TickAssembly();
};
