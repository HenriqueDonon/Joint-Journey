#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzleDoor.generated.h"

UCLASS()
class JOGOCOOP_API APuzzleDoor : public AActor
{
	GENERATED_BODY()
public:
	APuzzleDoor();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetOpen(bool bOpen);
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	UPROPERTY(EditAnywhere) float OpenHeight = 300.f;
	UPROPERTY(ReplicatedUsing=OnRep_Open) bool bOpenVis = false;
	UFUNCTION() void OnRep_Open();
private:
	FVector Closed = FVector::ZeroVector;
	bool bWantOpen = false;
	UPROPERTY() UMaterialInstanceDynamic* Mat = nullptr;
};
