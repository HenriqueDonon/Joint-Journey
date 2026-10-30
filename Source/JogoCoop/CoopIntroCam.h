#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopIntroCam.generated.h"

// APRESENTACAO INICIAL (Fase 2): camera curta (~4,5 s) que parte perto dos personagens, sobe mostrando a altura do poco
// e termina na porta de saida. Ator do nivel, replicado: o servidor so define a hora de inicio (T0); a pose e calculada igual
// em todas as maquinas a partir da hora do servidor. O GameMode troca a vista dos jogadores e devolve o controle no fim
// (ou antes, se o jogador pular com E). Roda uma vez por sessao.
UCLASS()
class JOGOCOOP_API ACoopIntroCam : public AActor
{
	GENERATED_BODY()
public:
	ACoopIntroCam();
	virtual void Tick(float Dt) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
	UPROPERTY(VisibleAnywhere) class UCameraComponent* Cam;
	UPROPERTY(EditAnywhere, Category="Intro") TArray<FVector> Pos;      // pontos da camera (mundo)
	UPROPERTY(EditAnywhere, Category="Intro") TArray<FVector> Look;     // alvos correspondentes
	UPROPERTY(EditAnywhere, Category="Intro") float Duration = 4.5f;
	UPROPERTY(Replicated) float T0 = -1.f;
	void ServerStart(float Lead = 0.3f);
	bool IsRunning() const;
	bool IsDone() const;
	float NowSrv() const;
};
