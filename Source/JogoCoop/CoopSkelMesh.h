#pragma once
#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "CoopSkelMesh.generated.h"

// Malha esqueletal que, depois da animacao, gira o braco direito (upperarm_r e filhos) para apontar em AimDir.
// AimAlpha 0 = animacao normal, 1 = braco totalmente apontado (usado ao atirar).
UCLASS()
class JOGOCOOP_API UCoopSkelMesh : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:
	float AimAlpha = 0.f;
	FVector AimDir = FVector::ForwardVector; // espaco do mundo
	virtual void FinalizeBoneTransform() override;
};
