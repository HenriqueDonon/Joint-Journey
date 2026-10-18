#pragma once
#include "CoreMinimal.h"
#include "CoopBlock.h"
#include "CoopCeiling.generated.h"

// Peca de teto: bloco visual SEM colisao que pode despencar (servidor anima, movimento replicado) e voltar.
UCLASS()
class JOGOCOOP_API ACoopCeiling : public ACoopBlock
{
	GENERATED_BODY()
public:
	ACoopCeiling();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void StartFall(float Duration, float Dist);
	void ResetPos();
private:
	FVector Home = FVector::ZeroVector;
	float FallDur = 1.f, FallDist = 500.f, FallT = 0.f;
	bool bFalling = false;
};
