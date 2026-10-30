#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoopConsole.generated.h"

UENUM(BlueprintType)
enum class ECoopConsoleAction : uint8 { ElevatorToggle = 0, ElevatorCallBottom = 1, ElevatorCallTop = 2, MachineReset = 3 };

// CONSOLE de uso explicito (tecla E perto dele). O personagem pede ao servidor (ServerInteract); o servidor valida e executa.
// Fase 2: painel da cabine (ElevatorToggle: alterna andar), chamada no patamar inferior/superior.
// Reutilizavel: novas acoes entram no enum + Use().
UCLASS()
class JOGOCOOP_API ACoopConsole : public AActor
{
	GENERATED_BODY()
public:
	ACoopConsole();
	virtual void BeginPlay() override;
	virtual void Tick(float Dt) override;
	// dica contextual (HUD) e estado real: 0 = bloqueado (vermelho) | 1 = sem efeito agora (cinza, sem [E]) | 2 = disponivel (verde, [E])
	FString Prompt(int32& OutState) const;
	UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Screen;
	UPROPERTY(EditAnywhere, Category="Console") ECoopConsoleAction Action = ECoopConsoleAction::ElevatorToggle;
	UPROPERTY(EditAnywhere, Category="Console") class ACoopElevator* Elevator = nullptr;
	UPROPERTY(EditAnywhere, Category="Console") class ACoopBalanceMachine* Machine = nullptr;   // MachineReset: rearme da maquina do P2
	UPROPERTY(EditAnywhere, Category="Console") float UseRadius = 230.f;
	UPROPERTY(EditAnywhere, Category="Console") bool bAttachToElevator = false;   // painel da cabine: anda junto
	// Servidor: executa a acao. Retorna false com motivo (mostrado ao jogador).
	bool ServerUse(class ACoopCharacter* By, FString& OutWhy);
private:
	UPROPERTY() class UMaterialInstanceDynamic* ScreenMat = nullptr;
	UPROPERTY() class UPointLightComponent* Lamp = nullptr;
	int32 LastState = -1;
};
