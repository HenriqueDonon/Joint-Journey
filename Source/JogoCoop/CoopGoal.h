#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopGoal.generated.h"

// Meta da sala: completa quando os 2 jogadores estão dentro.
UCLASS()
class JOGOCOOP_API ACoopGoal : public AActor
{
	GENERATED_BODY()
public:
	ACoopGoal();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UPROPERTY(VisibleAnywhere) class UBoxComponent* Box;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Pad;
	UPROPERTY(Replicated) int32 RoomIndex = 1;
	UPROPERTY(ReplicatedUsing=OnRep_Complete) bool bComplete = false;
	UFUNCTION() void OnRep_Complete();
private:
	TSet<AActor*> Inside;
	UPROPERTY() UMaterialInstanceDynamic* Mat = nullptr;
	UFUNCTION() void OnBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() void OnEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
