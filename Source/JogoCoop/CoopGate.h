#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopGate.generated.h"

// PORTAO DE PROGRESSO (Fase 2): bloqueia uma passagem ate ser liberado pelo servidor (futuro: conclusao de P1/P2/P3).
// Estado replicado; o painel sobe ao abrir. GateId identifica quem o libera (ex.: P2_Norte, P2_Sul, P3_Saida).
UCLASS()
class JOGOCOOP_API ACoopGate : public AActor
{
	GENERATED_BODY()
public:
	ACoopGate();
	virtual void BeginPlay() override;
	virtual void Tick(float Dt) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
	UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Panel;
	UPROPERTY(EditAnywhere, Category="Portao") FName GateId = NAME_None;
	UPROPERTY(EditAnywhere, Category="Portao") FVector Size = FVector(20.f, 300.f, 320.f);   // cm, origem = centro da base
	UPROPERTY(EditAnywhere, Replicated, Category="Portao") bool bOpen = false;
	void ServerSetOpen(bool b);
private:
	float A = 0.f;
};
