#pragma once
#include "CoreMinimal.h"
#include "CoopBlock.h"
#include "CoopExitDoor.generated.h"

// Porta de saida: bloco com colisao que sobe (servidor anima, movimento replicado) quando aberta.
UCLASS()
class JOGOCOOP_API ACoopExitDoor : public ACoopBlock
{
	GENERATED_BODY()
public:
	ACoopExitDoor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void SetOpen(bool b) { if (HasAuthority()) bOpen = b; }
	bool IsOpen() const { return bOpen; }
private:
	FVector Home = FVector::ZeroVector;
	float Open01 = 0.f;
	bool bOpen = false;
};
