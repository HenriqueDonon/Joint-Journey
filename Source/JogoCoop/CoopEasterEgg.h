#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopEasterEgg.generated.h"

// Cigarro de seda enrolado a mao, pontinha torcida. Ao chegar perto, mostra uma mensagem.
UCLASS()
class JOGOCOOP_API ACoopEasterEgg : public AActor
{
	GENERATED_BODY()
public:
	ACoopEasterEgg();
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere) FString Message;
private:
	UPROPERTY() USceneComponent* Root;
	UPROPERTY() UStaticMeshComponent* Paper;
	UPROPERTY() UStaticMeshComponent* TipA;
	UPROPERTY() UStaticMeshComponent* TipB;
	UPROPERTY() UStaticMeshComponent* Filling;
	UPROPERTY() class UBoxComponent* Trigger;
	TSet<AActor*> Shown;
	UStaticMeshComponent* MakePart(const TCHAR* Name, FVector Loc, FRotator Rot, FVector Scale);
	void Tint(UStaticMeshComponent* C, FLinearColor Col);
	UFUNCTION() void OnBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
