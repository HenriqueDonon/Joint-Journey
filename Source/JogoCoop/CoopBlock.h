#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopBlock.generated.h"

// Bloco colidível replicado (chão, paredes). Tamanho em cm.
UCLASS()
class JOGOCOOP_API ACoopBlock : public AActor
{
	GENERATED_BODY()
public:
	ACoopBlock();
	void Setup(FVector InSize, FLinearColor InColor, bool bInGhost = false, FLinearColor InEmissive = FLinearColor::Black, float InRough = 0.9f, float InMetal = 0.f);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	UPROPERTY(ReplicatedUsing=OnRep_Setup) FVector Size = FVector(100);
	UPROPERTY(ReplicatedUsing=OnRep_Setup) FLinearColor Color = FLinearColor::Gray;
	UPROPERTY(ReplicatedUsing=OnRep_Setup) FLinearColor Emissive = FLinearColor::Black;
	UPROPERTY(ReplicatedUsing=OnRep_Setup) float Rough = 0.9f;
	UPROPERTY(ReplicatedUsing=OnRep_Setup) float Metal = 0.f;
	UPROPERTY(ReplicatedUsing=OnRep_Setup) bool bGhost = false;
	// Slot de modelo 3D: se existir /Game/Fase1/<Model>/SM_<Model>, ele vira o visual (origem = centro da base, 1uu=1cm).
	// A colisao continua sendo a do bloco-placeholder. bNoCollide = decoracao sem colisao (replicado).
	UPROPERTY(ReplicatedUsing=OnRep_Setup) FName Model = NAME_None;
	UPROPERTY(ReplicatedUsing=OnRep_Setup) bool bNoCollide = false;
	UPROPERTY(ReplicatedUsing=OnRep_Setup) FVector ModelScale = FVector(1.f); // escala extra do modelo (tiles/lava)
	UPROPERTY() UStaticMeshComponent* ModelComp = nullptr;
	void SetModelScale(FVector S) { ModelScale = S; OnRep_Setup(); }
	void SetOpts(FName InModel, bool bInNoCollide) { Model = InModel; bNoCollide = bInNoCollide; OnRep_Setup(); }
	UFUNCTION() void OnRep_Setup();
};
