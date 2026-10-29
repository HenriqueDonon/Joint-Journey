#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopRoleBarrier.generated.h"

UENUM(BlueprintType)
enum class ECoopBarrierRole : uint8 { Neon = 0, Ruina = 1, Vertice = 2 };   // mesma ordem de ECoopRole

// BARREIRA SELETIVA reutilizavel (independente do elevador): volume translucido que BLOQUEIA fisicamente todos os personagens
// exceto o papel autorizado. A regra vem do papel replicado (ACoopCharacter::CoopRole), nao de quem e host/cliente.
// Funcionamento: a malha bloqueia o canal Pawn; em CADA maquina (servidor e clientes) o ator manda a capsula do personagem
// autorizado ignora-la ao mover (IgnoreActorWhenMoving), entao a predicao do cliente e a simulacao do servidor concordam.
// Uso: coloque o ator no mapa, escolha AllowedRole, Size (cm) e (opcional) cor propria; nada mais.
UCLASS()
class JOGOCOOP_API ACoopRoleBarrier : public AActor
{
	GENERATED_BODY()
public:
	ACoopRoleBarrier();
	virtual void OnConstruction(const FTransform& T) override;
	virtual void BeginPlay() override;
	virtual void Tick(float Dt) override;

	UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Mesh;
	UPROPERTY(EditAnywhere, Category="Barreira") ECoopBarrierRole AllowedRole = ECoopBarrierRole::Ruina;
	UPROPERTY(EditAnywhere, Category="Barreira") FVector Size = FVector(250.f, 20.f, 300.f);   // cm; origem do ator = centro
	UPROPERTY(EditAnywhere, Category="Barreira") bool bAutoTint = true;                        // roxo p/ Ruina, ciano p/ Neon
	UPROPERTY(EditAnywhere, Category="Barreira") FLinearColor Tint = FLinearColor(0.7f, 0.2f, 1.f);
	UPROPERTY(EditAnywhere, Category="Barreira") float Opacity = 0.35f;
private:
	UPROPERTY() class UMaterialInstanceDynamic* Mid = nullptr;
	void ApplyLook();
};
