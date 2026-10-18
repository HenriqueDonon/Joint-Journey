#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "JJVisual.h"
#include "CoopCharacter.generated.h"

UENUM(BlueprintType)
enum class ECoopRole : uint8 { Neon, Ruina, Vertice };

UCLASS()
class JOGOCOOP_API ACoopCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	ACoopCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	UFUNCTION(Client, Reliable) void ClientShowMessage(const FString& Text);
	UFUNCTION(Client, Reliable) void ClientSetupCamera();
	UFUNCTION(Client, Reliable) void ClientToast(const FString& Text);
	virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Role) ECoopRole CoopRole = ECoopRole::Neon;
	UPROPERTY(EditAnywhere) float AbilityRange = 8000.f;
	UPROPERTY(EditAnywhere) float DestructRange = 1500.f;
	UFUNCTION() void OnRep_Role();
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Mode) bool bPortalMode = false;
	UFUNCTION() void OnRep_Mode();
	bool IsAimOnProp() const { return TraceProp() != nullptr; }

private:
	UPROPERTY(VisibleAnywhere) class USpringArmComponent* Boom;
	UPROPERTY(VisibleAnywhere) class UCameraComponent* Cam;

	// Pivôs animados proceduralmente
	UPROPERTY() class USceneComponent* UpperPivot;
	UPROPERTY() class USceneComponent* HeadPivot;
	UPROPERTY() class USceneComponent* ArmLPivot;
	UPROPERTY() class USceneComponent* ArmRPivot;
	UPROPERTY() class USceneComponent* LegLPivot;
	UPROPERTY() class USceneComponent* LegRPivot;
	UPROPERTY() class USceneComponent* CapePivot;

	// Grupos de peças por material
	UPROPERTY() TArray<class UMeshComponent*> GMain;
	UPROPERTY() TArray<class UMeshComponent*> GDark;
	UPROPERTY() TArray<class UMeshComponent*> GPanel;
	UPROPERTY() TArray<class UMeshComponent*> GFace;
	UPROPERTY() TArray<class UMeshComponent*> GAccent;
	UPROPERTY() TArray<class UMeshComponent*> GCloth;
	UPROPERTY() TArray<class UMeshComponent*> GCap;
	UPROPERTY() TArray<class UMeshComponent*> GHat;
	UPROPERTY() TArray<class UMeshComponent*> GBand;
	UPROPERTY() TArray<class UMeshComponent*> GBrown;
	UPROPERTY() TArray<class UMeshComponent*> GEyesN;
	UPROPERTY() TArray<class UMeshComponent*> GEyesR;
	UPROPERTY() TArray<class UMeshComponent*> NeonOnly;
	UPROPERTY() TArray<class UMeshComponent*> VertOnly;   // peças exclusivas do Vértice
	UPROPERTY() TArray<class UMeshComponent*> GVHood;     // capuz/mochila (branco)
	UPROPERTY() TArray<class UMeshComponent*> GVAccent;   // detalhes laranja do Vértice
	UPROPERTY() TArray<class UMeshComponent*> GVDark;     // grafite do Vértice
	// Vértice: malha esqueletal importada (Tripo) com animações retargetadas
	UPROPERTY() class UAnimSequence* VAnimIdle = nullptr;
	UPROPERTY() class UAnimSequence* VAnimWalk = nullptr;
	UPROPERTY() class UAnimSequence* VAnimRun = nullptr;
	UPROPERTY() class UAnimSequence* VAnimJump = nullptr;
	UPROPERTY() class UAnimSequence* VAnimFall = nullptr;
	UPROPERTY() class UAnimSequence* VAnimCur = nullptr;
	bool bVWasFalling = false;
	uint8 PapelMalha = 255;
	void LoadPapel();
	void UpdateVerticeAnim();
	void PlayV(class UAnimSequence* A, bool bLoop, float Rate = 1.f);
	void TestVertice();                                   // F3: alterna entre Neon/Ruina/Vertice (teste solo, 1 janela)
	UFUNCTION(Server, Reliable) void ServerSetRole(ECoopRole NewRole);
	void TestBridge();                                    // F4: liga/desliga ponte completa (so mapa Fase1_Teste)
	UFUNCTION(Server, Reliable) void ServerToggleTestBridge();
	UPROPERTY() TArray<class UMeshComponent*> RuinaOnly;

	// Arma
	UPROPERTY() class USceneComponent* GunPivot;
	UPROPERTY() class UProceduralMeshComponent* GunBody;
	UPROPERTY() class UProceduralMeshComponent* GunBarrel;
	UPROPERTY() class UProceduralMeshComponent* GunTip;
	UPROPERTY() class UProceduralMeshComponent* ProngA;
	UPROPERTY() class UProceduralMeshComponent* ProngB;

	TArray<TPair<class UProceduralMeshComponent*, FJJShape>> Pending;
	bool bBuilt = false;
	void BuildAll();
	float AnimPhase = 0.f, AnimSpeed = 0.f, AirBlend = 0.f, AnimTime = 0.f;
	void AnimateBody(float DT);
	void ApplyGunLook();
	void FireA(); void FireB(); void ToggleMode(); void FirePortal(bool bOrange);
	UFUNCTION(Server, Reliable) void ServerPlacePortal(bool bOrange, FVector_NetQuantize10 Pt, FVector_NetQuantizeNormal N);
	UFUNCTION(Server, Reliable) void ServerSetMode(bool bPortal);

	void MoveB(float V); void MoveL(float V); void MoveF(float V); void MoveR(float V); void LookX(float V); void LookY(float V);
	void Primary(); void Secondary(); void ShowHelp();
	class ACoopProp* TraceProp() const;
	// Poder novo (tecla F): Ruina destroi / Neon reconstroi ACoopDestructible. Isolado dos poderes antigos.
	void DestructPower();
	class ACoopDestructible* TraceDestructible() const;
	UFUNCTION(Server, Reliable) void ServerDestructPower(class ACoopDestructible* Target);
	UFUNCTION(Server, Reliable) void ServerUseAbility(class ACoopProp* Target, bool bSecondary);
};
