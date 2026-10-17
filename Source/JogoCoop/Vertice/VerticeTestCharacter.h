#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VerticeTestCharacter.generated.h"

// Personagem de TESTE do Vértice (mapa /Game/Vertice/Maps/Teste_Vertice). Não usado pelo jogo principal.
// Controles iguais aos do jogo (WASD, mouse, Espaço) + Shift para correr.
// Com -VerticeDemo na linha de comando, executa uma demonstração automática e grava capturas em Saved/VerticeDemo.
UCLASS()
class JOGOCOOP_API AVerticeTestCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	AVerticeTestCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* Input) override;

	UPROPERTY(EditAnywhere) float WalkSpeed = 220.f;
	UPROPERTY(EditAnywhere) float RunSpeed = 480.f;

private:
	UPROPERTY(VisibleAnywhere) class USpringArmComponent* Boom;
	UPROPERTY(VisibleAnywhere) class UCameraComponent* Cam;
	UPROPERTY() class UAnimSequence* AnimIdle = nullptr;
	UPROPERTY() class UAnimSequence* AnimWalk = nullptr;
	UPROPERTY() class UAnimSequence* AnimRun = nullptr;
	UPROPERTY() class UAnimSequence* AnimJump = nullptr;
	UPROPERTY() class UAnimSequence* AnimFall = nullptr;
	UPROPERTY() class UAnimSequence* AnimLand = nullptr;
	UPROPERTY() class UAnimSequence* Current = nullptr;

	bool bRunKey = false, bDemo = false, bNeon = false;
	FString PastaDemo = TEXT("VerticeDemo");
	float DemoT = 0.f, NextFrame = 0.f, LandT = 0.f;
	int32 FrameIdx = 0, StableFrames = 0;
	TSet<FString> ShotsDone;
	bool bWasFalling = false, bJ1 = false, bJ2 = false;

	void LookX(float V); void LookY(float V);
	void RunOn(); void RunOff();
	void PlayAnim(class UAnimSequence* A, bool bLoop, float Rate = 1.f);
	void UpdateAnim(float DT);
	void DemoTick(float DT);
	void Shot(const FString& Name);
};
