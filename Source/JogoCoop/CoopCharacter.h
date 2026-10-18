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
	ACoopCharacter(const FObjectInitializer& ObjectInitializer);
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
	UPROPERTY(EditAnywhere) float DestructRange = 3000.f;
	UFUNCTION() void OnRep_Role();
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Mode) bool bPortalMode = false;
	UFUNCTION() void OnRep_Mode();
	bool IsAimOnProp() const { return TraceProp() != nullptr; }
	// Esmagado pelo teto (Fase 1): boneco achatado e parado ate a fase reiniciar
	UPROPERTY(ReplicatedUsing=OnRep_Crushed) bool bCrushed = false;
	UFUNCTION() void OnRep_Crushed();
	void SetCrushed(bool b);
	// Camera livre (tecla C): a camera gira em volta sem virar o boneco (para se ver de frente)
	UPROPERTY(ReplicatedUsing=OnRep_FreeCam) bool bFreeCam = false;
	UFUNCTION() void OnRep_FreeCam();

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
	UPROPERTY() TArray<class UMeshComponent*> GunParts;   // todas as pecas da arma (nunca escondidas pelo papel)
	UPROPERTY() TArray<class UMeshComponent*> GunWhite;
	UPROPERTY() TArray<class UMeshComponent*> GunDark;
	UPROPERTY() TArray<class UMeshComponent*> GunGlow;
	UPROPERTY() class UPointLightComponent* FaceLight;     // luz suave no rosto da Ruina (atravessa a aba do chapeu)
	FVector MeshScale0 = FVector(1.f);
	bool bSquashed = false;
	void ToggleFreeCam();
	UFUNCTION(Server, Reliable) void ServerSetFreeCam(bool b);

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
	// Tiro do poder (botao esquerdo): levanta o braco e dispara o raio em qualquer direcao; so tem efeito em objeto destrutivel
	void PowerShot();
	UFUNCTION(NetMulticast, Unreliable) void MulticastShot(FVector_NetQuantize10 End, FLinearColor Col, bool bHit);
	float ShootT = 0.f, ArmAlpha = 0.f;
	FVector ShotEnd = FVector::ZeroVector;
	float BeamAt = -1.f; FVector BeamEnd = FVector::ZeroVector; FLinearColor BeamCol = FLinearColor::White; bool BeamHit = false;
	class ACoopProp* TraceProp() const;
	// Poder novo (tecla F): Ruina destroi / Neon reconstroi ACoopDestructible. Isolado dos poderes antigos.
	class ACoopDestructible* TraceDestructible() const;
	UFUNCTION(Server, Reliable) void ServerPowerShot(class ACoopDestructible* Target, FVector_NetQuantize10 End);
	UFUNCTION(Server, Reliable) void ServerUseAbility(class ACoopProp* Target, bool bSecondary);
};
