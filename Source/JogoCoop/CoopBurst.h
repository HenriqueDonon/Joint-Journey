#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopBurst.generated.h"

// Efeito local (não replicado): faíscas que explodem + raio do jogador até o cubo. Some sozinho.
UCLASS()
class JOGOCOOP_API ACoopBurst : public AActor
{
	GENERATED_BODY()
public:
	ACoopBurst();
	void Init(FLinearColor Col, FVector BeamFrom, int32 NumSparks = 28);
	virtual void Tick(float DeltaTime) override;
private:
	float Age = 0.f;
	UPROPERTY() TArray<UStaticMeshComponent*> Parts;
	UPROPERTY() UStaticMeshComponent* Beam = nullptr;
	TArray<FVector> Dirs;
};
