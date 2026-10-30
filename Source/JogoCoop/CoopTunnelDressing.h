#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopTunnelDressing.generated.h"

// Revestimento do tunel pos-fase no Fase 1 oficial /Game/Maps/Fase1 (antiga Arquitetura) (mesma linguagem do galpao: aco escuro, nervuras, faixa ciano,
// faixas de perigo, vigas e canos no teto, luz fria + acentos ambar). Colocado no mapa na posicao do marcador F1L_Porta
// (piso no centro da porta); monta so visual (sem colisao) em BeginPlay em cada maquina. Nao altera o tunel de gameplay.
UCLASS()
class JOGOCOOP_API ACoopTunnelDressing : public AActor
{
	GENERATED_BODY()
public:
	ACoopTunnelDressing();
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, Category="Tunel") float X0 = 84.f;      // inicio do tunel (relativo ao ator)
	UPROPERTY(EditAnywhere, Category="Tunel") float Len = 1710.f;
	UPROPERTY(EditAnywhere, Category="Tunel") float HalfW = 420.f;
	UPROPERTY(EditAnywhere, Category="Tunel") float H = 440.f;
private:
	void Box(class UStaticMesh* M, class UMaterialInterface* Mat, FVector C, FVector S, FRotator R = FRotator::ZeroRotator, bool bShadow = true);
};
