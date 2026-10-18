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
	void Setup(FLinearColor InColor, float InLumens, float InRadius);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UPROPERTY(VisibleAnywhere) class UPointLightComponent* Light;
	UPROPERTY(ReplicatedUsing=OnRep_Lamp) FLinearColor Col = FLinearColor(1.f, 0.95f, 0.85f);
	UPROPERTY(ReplicatedUsing=OnRep_Lamp) float Lumens = 9000.f;
	UPROPERTY(ReplicatedUsing=OnRep_Lamp) float Radius = 1000.f;
	UFUNCTION() void OnRep_Lamp();
};
