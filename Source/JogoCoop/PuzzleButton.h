#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzleButton.generated.h"

class APuzzleDoor;

// Botão de pressão. Abre as portas ligadas enquanto alguém (jogador ou cubo) está em cima.
// Com bLatch, fica pressionado para sempre depois do primeiro toque.
UCLASS()
class JOGOCOOP_API APuzzleButton : public AActor
{
	GENERATED_BODY()
public:
	APuzzleButton();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UPROPERTY(VisibleAnywhere) class UBoxComponent* Trigger;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	UPROPERTY(EditAnywhere) TArray<APuzzleDoor*> Doors;
	UPROPERTY(EditAnywhere) bool bLatch = false;
	UPROPERTY(ReplicatedUsing=OnRep_Pressed) bool bPressedVis = false;
	UFUNCTION() void OnRep_Pressed();
protected:
	virtual void BeginPlay() override;
private:
	int32 Count = 0;
	bool bLatched = false;
	UPROPERTY() UMaterialInstanceDynamic* Mat = nullptr;
	UFUNCTION() void OnBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() void OnEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	void Update();
};
