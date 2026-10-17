#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopPortal.generated.h"

// Portal azul/laranja colocado em paredes/chão. Quem entra num sai no outro.
UCLASS()
class JOGOCOOP_API ACoopPortal : public AActor
{
	GENERATED_BODY()
public:
	ACoopPortal();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void Setup(bool bInOrange, FVector Loc, FVector Normal); // servidor

	UPROPERTY(ReplicatedUsing=OnRep_Setup) bool bOrange = false;
	UPROPERTY(ReplicatedUsing=OnRep_Setup) FVector_NetQuantize10 RepLoc;
	UPROPERTY(ReplicatedUsing=OnRep_Setup) FVector_NetQuantizeNormal RepNormal;
	UFUNCTION() void OnRep_Setup();
	UFUNCTION() void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY() USceneComponent* Root;
	UPROPERTY() UStaticMeshComponent* Rim;
	UPROPERTY() UStaticMeshComponent* Inner;
	UPROPERTY() class UBoxComponent* Trig;
	UPROPERTY() class UPointLightComponent* Light;
};
