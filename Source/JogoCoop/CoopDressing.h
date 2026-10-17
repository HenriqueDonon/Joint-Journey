#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopDressing.generated.h"

// Decoração só visual (sem colisão) de uma sala. Cada máquina monta a sua cópia a partir de Seed/Variant.
// Lado Neon (y<0): verde e roxo moderno. Lado Ruína (y>0): concreto com cristais roxos. Paredes brancas desgastadas.
UCLASS()
class JOGOCOOP_API ACoopDressing : public AActor
{
	GENERATED_BODY()
public:
	ACoopDressing();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UPROPERTY(Replicated) int32 Seed = 1;
	UPROPERTY(Replicated) int32 Variant = 1;
private:
	void Build();
};
