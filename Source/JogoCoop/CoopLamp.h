#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopLamp.generated.h"

// Luz pontual replicada, SEM sombras (iluminacao de preenchimento barata).
UCLASS()
class JOGOCOOP_API ACoopLamp : public AActor
{
	GENERATED_BODY()
public:
	ACoopLamp();
	virtual void Tick(float DeltaTime) override;
	void Setup(FLinearColor InColor, float InLumens, float InRadius, float InFlicker = 0.f);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UPROPERTY(VisibleAnywhere) class UPointLightComponent* Light;
	UPROPERTY(ReplicatedUsing=OnRep_Lamp) FLinearColor Col = FLinearColor(1.f, 0.95f, 0.85f);
	UPROPERTY(ReplicatedUsing=OnRep_Lamp) float Lumens = 9000.f;
	UPROPERTY(ReplicatedUsing=OnRep_Lamp) float Radius = 1000.f;
	UPROPERTY(Replicated) float Flicker = 0.f; // 0 = estavel; 0.3 = chama
	UFUNCTION() void OnRep_Lamp();
};
