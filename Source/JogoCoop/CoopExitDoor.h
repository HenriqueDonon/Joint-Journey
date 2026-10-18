#pragma once
#include "CoreMinimal.h"
#include "CoopBlock.h"
#include "CoopExitDoor.generated.h"

// Folha da porta de saida: bloco com colisao que desliza na lateral (servidor anima, movimento replicado) quando aberta.
UCLASS()
class JOGOCOOP_API ACoopExitDoor : public ACoopBlock
{
	GENERATED_BODY()
public:
	ACoopExitDoor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void SetOpen(bool b) { if (HasAuthority()) bOpen = b; }
	void SetSlide(FVector Dir, float Dist) { SlideDir = Dir; SlideDist = Dist; }
	bool IsOpen() const { return bOpen; }
private:
	FVector Home = FVector::ZeroVector;
	float Open01 = 0.f, SlideDist = 0.f;
	FVector SlideDir = FVector(0, 1, 0);
	bool bOpen = false;
};
