#include "CoopCharacter.h"
#include "CoopGameUserSettings.h"
#include "CoopDestructible.h"
#include "CoopFase1GameMode.h"
#include "CoopProp.h"
#include "CoopBurst.h"
#include "CoopPortal.h"
#include "CoopBlock.h"
#include "EngineUtils.h"
#include "JJVisual.h"
#include "ProceduralMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"
#include "InputCoreTypes.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"

ACoopCharacter::ACoopCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	Boom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Boom"));
	Boom->SetupAttachment(RootComponent);
	Boom->TargetArmLength = 300.f;
	Boom->SocketOffset = FVector(0, 70, 70);
	Boom->bUsePawnControlRotation = true;
	Cam = CreateDefaultSubobject<UCameraComponent>(TEXT("Cam"));
	Cam->SetupAttachment(Boom);
	Cam->PostProcessBlendWeight = 1.f;
	Cam->PostProcessSettings.bOverride_BloomIntensity = true;
	Cam->PostProcessSettings.BloomIntensity = 0.25f;
	Cam->PostProcessSettings.bOverride_BloomThreshold = true;
	Cam->PostProcessSettings.BloomThreshold = 1.0f;
	Cam->PostProcessSettings.bOverride_AutoExposureBias = true;
	Cam->PostProcessSettings.AutoExposureBias = -0.3f;
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = 0.6f;

	USceneComponent* Cap = GetCapsuleComponent();
	const FRotator Z0 = FRotator::ZeroRotator;
	auto Pv = [&](const TCHAR* N, USceneComponent* Par, FVector Loc) {
		USceneComponent* S = CreateDefaultSubobject<USceneComponent>(N);
		S->SetupAttachment(Par);
		S->SetRelativeLocation(Loc);
		return S;
	};
	auto Mk = [&](const TCHAR* N, USceneComponent* Par, FVector Loc, FRotator Rot, FJJShape Sh, TArray<UMeshComponent*>& G) {
		UProceduralMeshComponent* C = CreateDefaultSubobject<UProceduralMeshComponent>(N);
		C->SetupAttachment(Par);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetRelativeLocation(Loc);
		C->SetRelativeRotation(Rot);
		C->SetCastShadow(false);
		Pending.Add(TPair<UProceduralMeshComponent*, FJJShape>(C, Sh));
		G.Add(C);
		return C;
	};
	TArray<UMeshComponent*> Dummy;

	// tronco: peito largo -> abdômen estreito (silhueta orgânica)
	UpperPivot = Pv(TEXT("UpperPivot"), Cap, FVector::ZeroVector);
	Mk(TEXT("Chest"), UpperPivot, FVector(0, 0, 17), Z0, JJShape(23, 28, 18, 0.7f, 0.75f, 1.1f, 20, 12), GMain);
	Mk(TEXT("Abdomen"), UpperPivot, FVector(0, 0, -6), Z0, JJShape(17, 21, 13, 0.8f, 0.85f, 0.9f, 16, 8), GDark);
	Mk(TEXT("ChestRing"), UpperPivot, FVector(22.6f, 0, 17), FRotator(90, 0, 0), JJShape(9, 9, 1.2f, 0.35f, 1, 1, 20, 6), GAccent);
	Mk(TEXT("ChestInner"), UpperPivot, FVector(23.6f, 0, 17), FRotator(90, 0, 0), JJShape(5.8f, 5.8f, 1.2f, 0.35f, 1, 1, 16, 6), GDark);
	Mk(TEXT("Pelvis"), Cap, FVector(0, 0, -24), Z0, JJShape(18, 21, 9, 0.9f, 0.9f, 1, 14, 8), GDark);
	Mk(TEXT("Neck"), UpperPivot, FVector(0, 0, 38), Z0, JJShape(7, 7, 6, 0.6f, 0.6f, 1, 10, 6), GDark);
	Mk(TEXT("Scarf"), UpperPivot, FVector(0, 0, 41), Z0, JJShape(18, 19, 6, 0.7f, 0.8f, 1, 16, 8), GCloth);
	CapePivot = Pv(TEXT("CapePivot"), UpperPivot, FVector(-20, 0, 36));
	Mk(TEXT("Cape"), CapePivot, FVector(-3, 0, -32), Z0, JJShape(1.8f, 26, 32, 0.45f, 0.5f, 0.8f, 10, 10), GCloth);

	// braços: junta do ombro escura (transição suave), ombreira, braço, cotovelo, antebraço grosso, mão robusta
	ArmLPivot = Pv(TEXT("ArmLPivot"), UpperPivot, FVector(0, -33, 24));
	ArmRPivot = Pv(TEXT("ArmRPivot"), UpperPivot, FVector(0, 33, 24));
	Mk(TEXT("ShJointL"), ArmLPivot, FVector::ZeroVector, Z0, JJShape(11, 11, 11, 1, 1, 1, 12, 8), GDark);
	Mk(TEXT("ShJointR"), ArmRPivot, FVector::ZeroVector, Z0, JJShape(11, 11, 11, 1, 1, 1, 12, 8), GDark);
	Mk(TEXT("PadL"), ArmLPivot, FVector(0, -3, 3), Z0, JJShape(14, 13, 13, 1, 1, 1, 14, 8), GMain);
	Mk(TEXT("PadR"), ArmRPivot, FVector(0, 3, 3), Z0, JJShape(14, 13, 13, 1, 1, 1, 14, 8), GMain);
	Mk(TEXT("ShRingL"), ArmLPivot, FVector(0, -16.2f, 3), FRotator(0, 0, 90), JJShape(7.5f, 7.5f, 1.2f, 0.35f, 1, 1, 16, 6), GAccent);
	Mk(TEXT("ShRingR"), ArmRPivot, FVector(0, 16.2f, 3), FRotator(0, 0, 90), JJShape(7.5f, 7.5f, 1.2f, 0.35f, 1, 1, 16, 6), GAccent);
	Mk(TEXT("ShCoreL"), ArmLPivot, FVector(0, -17.2f, 3), FRotator(0, 0, 90), JJShape(4.5f, 4.5f, 1.2f, 0.35f, 1, 1, 12, 6), GDark);
	Mk(TEXT("ShCoreR"), ArmRPivot, FVector(0, 17.2f, 3), FRotator(0, 0, 90), JJShape(4.5f, 4.5f, 1.2f, 0.35f, 1, 1, 12, 6), GDark);
	Mk(TEXT("UArmL"), ArmLPivot, FVector(0, 0, -14), Z0, JJShape(8.8f, 8.8f, 12, 0.6f, 1, 1, 12, 8), GPanel);
	Mk(TEXT("UArmR"), ArmRPivot, FVector(0, 0, -14), Z0, JJShape(8.8f, 8.8f, 12, 0.6f, 1, 1, 12, 8), GPanel);
	Mk(TEXT("ElbowL"), ArmLPivot, FVector(0, 0, -27), Z0, JJShape(9.8f, 9.8f, 9.8f, 1, 1, 1, 12, 8), GDark);
	Mk(TEXT("ElbowR"), ArmRPivot, FVector(0, 0, -27), Z0, JJShape(9.8f, 9.8f, 9.8f, 1, 1, 1, 12, 8), GDark);
	Mk(TEXT("ArmL"), ArmLPivot, FVector(0, 0, -42), Z0, JJShape(11.5f, 11.5f, 15.5f, 0.6f, 1, 0.85f, 14, 10), GMain);
	Mk(TEXT("ArmR"), ArmRPivot, FVector(0, 0, -42), Z0, JJShape(11.5f, 11.5f, 15.5f, 0.6f, 1, 0.85f, 14, 10), GMain);
	Mk(TEXT("BraceL"), ArmLPivot, FVector(0, 0, -53), Z0, JJShape(12.4f, 12.4f, 1.4f, 0.35f, 1, 1, 16, 6), GAccent);
	Mk(TEXT("BraceR"), ArmRPivot, FVector(0, 0, -53), Z0, JJShape(12.4f, 12.4f, 1.4f, 0.35f, 1, 1, 16, 6), GAccent);
	Mk(TEXT("HandL"), ArmLPivot, FVector(0, 0, -66), Z0, JJShape(12, 13, 11, 0.85f, 0.85f, 1, 14, 10), GDark);
	Mk(TEXT("HandR"), ArmRPivot, FVector(0, 0, -66), Z0, JJShape(12, 13, 11, 0.85f, 0.85f, 1, 14, 10), GDark);
	Mk(TEXT("FingL"), ArmLPivot, FVector(9, 0, -68), Z0, JJShape(5.5f, 10.5f, 8.5f, 0.85f, 0.85f, 1, 10, 8), GDark);
	Mk(TEXT("FingR"), ArmRPivot, FVector(9, 0, -68), Z0, JJShape(5.5f, 10.5f, 8.5f, 0.85f, 0.85f, 1, 10, 8), GDark);

	// pernas: coxa, joelho esférico, canela, bota arredondada com aro luminoso na sola
	LegLPivot = Pv(TEXT("LegLPivot"), Cap, FVector(0, -14, -28));
	LegRPivot = Pv(TEXT("LegRPivot"), Cap, FVector(0, 14, -28));
	Mk(TEXT("ThighL"), LegLPivot, FVector(0, 0, -10), Z0, JJShape(10.8f, 10.8f, 12, 0.6f, 1, 1.08f, 14, 8), GMain);
	Mk(TEXT("ThighR"), LegRPivot, FVector(0, 0, -10), Z0, JJShape(10.8f, 10.8f, 12, 0.6f, 1, 1.08f, 14, 8), GMain);
	Mk(TEXT("KneeL"), LegLPivot, FVector(1.5f, 0, -21), Z0, JJShape(10, 10, 10, 1, 1, 1, 12, 8), GDark);
	Mk(TEXT("KneeR"), LegRPivot, FVector(1.5f, 0, -21), Z0, JJShape(10, 10, 10, 1, 1, 1, 12, 8), GDark);
	Mk(TEXT("ShinL"), LegLPivot, FVector(0, 0, -32), Z0, JJShape(9.6f, 9.6f, 11.5f, 0.6f, 1, 0.92f, 14, 8), GPanel);
	Mk(TEXT("ShinR"), LegRPivot, FVector(0, 0, -32), Z0, JJShape(9.6f, 9.6f, 11.5f, 0.6f, 1, 0.92f, 14, 8), GPanel);
	Mk(TEXT("AnkleL"), LegLPivot, FVector(0, 0, -43.5f), Z0, JJShape(10.5f, 10.5f, 1.4f, 0.35f, 1, 1, 14, 6), GAccent);
	Mk(TEXT("AnkleR"), LegRPivot, FVector(0, 0, -43.5f), Z0, JJShape(10.5f, 10.5f, 1.4f, 0.35f, 1, 1, 14, 6), GAccent);
	Mk(TEXT("BootL"), LegLPivot, FVector(8, 0, -50), Z0, JJShape(19, 13.5f, 10, 0.8f, 0.9f, 1, 18, 10), GMain);
	Mk(TEXT("BootR"), LegRPivot, FVector(8, 0, -50), Z0, JJShape(19, 13.5f, 10, 0.8f, 0.9f, 1, 18, 10), GMain);
	Mk(TEXT("SoleL"), LegLPivot, FVector(8, 0, -58.8f), Z0, JJShape(19.2f, 14, 1.2f, 0.35f, 0.85f, 1, 16, 6), GAccent);
	Mk(TEXT("SoleR"), LegRPivot, FVector(8, 0, -58.8f), Z0, JJShape(19.2f, 14, 1.2f, 0.35f, 0.85f, 1, 16, 6), GAccent);

	// cabeça esférica grande + máscara
	HeadPivot = Pv(TEXT("HeadPivot"), UpperPivot, FVector(0, 0, 40));
	Mk(TEXT("Head"), HeadPivot, FVector(0, 0, 22), Z0, JJShape(30, 30, 30, 1, 1, 1, 28, 18), GMain);
	Mk(TEXT("Face"), HeadPivot, FVector(12, 0, 22), Z0, JJShape(20, 25, 23, 1, 1, 1, 18, 12), GFace);
	// Neon: olhos ovais amigáveis + boné
	Mk(TEXT("EyeNL"), HeadPivot, FVector(31, -9.5f, 24), Z0, JJShape(2.5f, 4.8f, 8, 1, 1, 1, 10, 8), GEyesN);
	Mk(TEXT("EyeNR"), HeadPivot, FVector(31, 9.5f, 24), Z0, JJShape(2.5f, 4.8f, 8, 1, 1, 1, 10, 8), GEyesN);
	Mk(TEXT("CapDome"), HeadPivot, FVector(0, 0, 35), Z0, JJShape(31.5f, 31.5f, 19, 1, 1, 1, 22, 12), GCap);
	Mk(TEXT("CapBrim"), HeadPivot, FVector(30, 0, 31), FRotator(-8, 0, 0), JJShape(19, 24, 2, 0.5f, 0.7f, 1, 14, 6), GCap);
	// Ruína: olhos bravos, bigode bem visível, chapéu arredondado
	Mk(TEXT("EyeRL"), HeadPivot, FVector(31, -10, 26), FRotator(0, 0, -25), JJShape(2.2f, 8.5f, 3.2f, 0.6f, 0.6f, 1, 10, 6), GEyesR);
	Mk(TEXT("EyeRR"), HeadPivot, FVector(31, 10, 26), FRotator(0, 0, 25), JJShape(2.2f, 8.5f, 3.2f, 0.6f, 0.6f, 1, 10, 6), GEyesR);
	Mk(TEXT("MustL"), HeadPivot, FVector(30.5f, -9, 9), FRotator(0, 0, 65), JJShape(3.4f, 3.4f, 11, 0.8f, 1, 0.55f, 10, 8), GBrown);
	Mk(TEXT("MustR"), HeadPivot, FVector(30.5f, 9, 9), FRotator(0, 0, 115), JJShape(3.4f, 3.4f, 11, 0.8f, 1, 0.55f, 10, 8), GBrown);
	Mk(TEXT("MustC"), HeadPivot, FVector(31.5f, 0, 10), Z0, JJShape(4.5f, 5.5f, 3.6f, 1, 1, 1, 8, 6), GBrown);
	Mk(TEXT("HatBrim"), HeadPivot, FVector(0, 0, 38), Z0, JJShape(46, 46, 2.4f, 0.4f, 1, 1, 26, 6), GHat);
	Mk(TEXT("HatCrown"), HeadPivot, FVector(0, 0, 52), Z0, JJShape(25, 24, 15, 0.6f, 0.8f, 0.92f, 20, 10), GHat);
	Mk(TEXT("HatBand"), HeadPivot, FVector(0, 0, 43), Z0, JJShape(25.8f, 25, 3.2f, 0.4f, 0.7f, 1, 18, 6), GBand);

	// ===== VÉRTICE: capuz com ponta, olho ciclope âmbar, antenas, mochila tecnológica =====
	Mk(TEXT("VHood"), HeadPivot, FVector(-8, 0, 26), Z0, JJShape(34, 35, 37, 1, 1, 1, 26, 16), GVHood);
	Mk(TEXT("VHoodTip"), HeadPivot, FVector(-8, 0, 62), FRotator(-15, 0, 0), JJShape(12, 12, 14, 1, 1, 0.25f, 14, 8), GVHood);
	Mk(TEXT("VHoodMark"), HeadPivot, FVector(19, 0, 52), FRotator(-45, 0, 0), JJShape(1.4f, 7, 6, 0.5f, 0.5f, 0.2f, 10, 6), GVAccent);
	Mk(TEXT("VEyeRing"), HeadPivot, FVector(31, 0, 22), FRotator(90, 0, 0), JJShape(12, 12, 2.5f, 0.5f, 1, 1, 22, 6), GVDark);
	Mk(TEXT("VEyeGlow"), HeadPivot, FVector(32.6f, 0, 22), FRotator(90, 0, 0), JJShape(8, 8, 2.5f, 0.6f, 1, 1, 20, 6), GVAccent);
	Mk(TEXT("VEyeCore"), HeadPivot, FVector(34.2f, 0, 22), FRotator(90, 0, 0), JJShape(3.5f, 3.5f, 1.5f, 0.8f, 1, 1, 12, 6), GVHood);
	Mk(TEXT("VAntL"), HeadPivot, FVector(-6, -27, 50), FRotator(0, 0, -15), JJShape(1.3f, 1.3f, 9, 0.7f, 1, 0.8f, 8, 6), GVDark);
	Mk(TEXT("VAntR"), HeadPivot, FVector(-6, 27, 50), FRotator(0, 0, 15), JJShape(1.3f, 1.3f, 9, 0.7f, 1, 0.8f, 8, 6), GVDark);
	Mk(TEXT("VAntTipL"), HeadPivot, FVector(-6, -29.5f, 59), Z0, JJShape(2.2f, 2.2f, 2.2f, 1, 1, 1, 8, 6), GVAccent);
	Mk(TEXT("VAntTipR"), HeadPivot, FVector(-6, 29.5f, 59), Z0, JJShape(2.2f, 2.2f, 2.2f, 1, 1, 1, 8, 6), GVAccent);
	Mk(TEXT("VPack"), UpperPivot, FVector(-25, 0, 14), Z0, JJShape(8, 16, 17, 0.6f, 0.7f, 1, 14, 10), GVHood);
	Mk(TEXT("VPackLens"), UpperPivot, FVector(-33.2f, 0, 14), FRotator(90, 0, 0), JJShape(6, 6, 1.4f, 0.4f, 1, 1, 16, 6), GVAccent);
	Mk(TEXT("VPackRod"), UpperPivot, FVector(-24, 12, 40), Z0, JJShape(1.6f, 1.6f, 14, 0.7f, 1, 1, 8, 6), GVDark);
	VertOnly.Append(GVHood); VertOnly.Append(GVAccent); VertOnly.Append(GVDark);

	GAccent.Append(GEyesN); GAccent.Append(GEyesR); GAccent.Append(GBand);
	NeonOnly.Append(GEyesN); NeonOnly.Append(GCap);
	RuinaOnly.Append(GEyesR); RuinaOnly.Append(GHat); RuinaOnly.Append(GBrown); RuinaOnly.Append(GBand);

	// arma estilo Portal (arredondada)
	// malha do Vértice (só aparece no papel Vértice); modelo importado olha para +Y
	GetMesh()->SetRelativeLocation(FVector(0, 0, -88.f));
	GetMesh()->SetRelativeRotation(FRotator(0, -90.f, 0));
	GetMesh()->SetVisibility(false);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	GunPivot = Pv(TEXT("GunPivot"), Cap, FVector(30, 34, 24));
	GunBody = Mk(TEXT("GunBody"), GunPivot, FVector(22, 0, 0), Z0, JJShape(24, 6.5f, 8, 0.5f, 0.5f, 1, 12, 8), Dummy);
	GunBarrel = Mk(TEXT("GunBarrel"), GunPivot, FVector(55, 0, 0), FRotator(90, 0, 0), JJShape(4.5f, 4.5f, 16, 0.6f, 1, 1, 12, 8), Dummy);
	GunTip = Mk(TEXT("GunTip"), GunPivot, FVector(76, 0, 0), Z0, JJShape(6.5f, 6.5f, 6.5f, 1, 1, 1, 12, 8), Dummy);
	ProngA = Mk(TEXT("ProngA"), GunPivot, FVector(62, 0, 10), Z0, JJShape(14, 2.5f, 2, 0.5f, 0.5f, 1, 8, 4), Dummy);
	ProngB = Mk(TEXT("ProngB"), GunPivot, FVector(62, 0, -10), Z0, JJShape(14, 2.5f, 2, 0.5f, 0.5f, 1, 8, 4), Dummy);
}

void ACoopCharacter::BuildAll()
{
	if (bBuilt) return;
	bBuilt = true;
	for (TPair<UProceduralMeshComponent*, FJJShape>& P : Pending) JJ::BuildSuper(P.Key, P.Value);
	Pending.Empty();
	for (UMeshComponent* C : GMain) if (C) C->SetCastShadow(true);
	for (UMeshComponent* C : GPanel) if (C) C->SetCastShadow(true);
}

void ACoopCharacter::BeginPlay()
{
	Super::BeginPlay();
	BuildAll();
	OnRep_Role();
	if (IsLocallyControlled()) ShowHelp();
}

void ACoopCharacter::ClientShowMessage_Implementation(const FString& Text)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 12.f, FColor::Yellow, Text);
}

void ACoopCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AnimateBody(DeltaTime);
	UpdateVerticeAnim();
	if (!IsLocallyControlled()) return;
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		const float F = (PC->IsInputKeyDown(EKeys::W) ? 1.f : 0.f) - (PC->IsInputKeyDown(EKeys::S) ? 1.f : 0.f);
		const float R = (PC->IsInputKeyDown(EKeys::D) ? 1.f : 0.f) - (PC->IsInputKeyDown(EKeys::A) ? 1.f : 0.f);
		if (F) MoveF(F);
		if (R) MoveR(R);
	}
}

void ACoopCharacter::OnRep_Role()
{
	BuildAll();
	const bool bN = CoopRole == ECoopRole::Neon;
	const FLinearColor Blk(0.f, 0.f, 0.f);
	auto Apply = [](TArray<UMeshComponent*>& G, UMaterialInstanceDynamic* M) { for (UMeshComponent* C : G) if (C) C->SetMaterial(0, M); };
	const FLinearColor AccCol = bN ? FLinearColor(0.05f, 0.9f, 0.9f) : FLinearColor(0.6f, 0.25f, 1.f);
	// materiais opacos, foscos/acetinados; emissivo só nos detalhes pequenos
	Apply(GMain, bN ? JJ::Mat(this, FLinearColor(0.6f, 0.6f, 0.62f), Blk, 0.5f, 0.1f) : JJ::Mat(this, FLinearColor(0.03f, 0.03f, 0.035f), Blk, 0.38f, 0.85f));
	Apply(GPanel, bN ? JJ::Mat(this, FLinearColor(0.36f, 0.37f, 0.4f), Blk, 0.45f, 0.35f) : JJ::Mat(this, FLinearColor(0.07f, 0.07f, 0.08f), Blk, 0.8f, 0.1f));
	Apply(GDark, bN ? JJ::Mat(this, FLinearColor(0.05f, 0.055f, 0.065f), Blk, 0.4f, 0.6f) : JJ::Mat(this, FLinearColor(0.1f, 0.1f, 0.115f), Blk, 0.5f, 0.7f));
	Apply(GFace, bN ? JJ::Mat(this, FLinearColor(0.015f, 0.02f, 0.03f), Blk, 0.25f, 0.3f) : JJ::Mat(this, FLinearColor(0.012f, 0.012f, 0.016f), Blk, 0.3f, 0.2f));
	Apply(GAccent, JJ::Mat(this, FLinearColor(0.02f, 0.02f, 0.03f), AccCol * 2.f, 0.4f, 0.f));
	Apply(GCloth, bN ? JJ::Mat(this, FLinearColor(0.02f, 0.3f, 0.34f), Blk, 0.85f, 0.f) : JJ::Mat(this, FLinearColor(0.07f, 0.04f, 0.12f), Blk, 0.9f, 0.f));
	Apply(GCap, JJ::Mat(this, FLinearColor(0.03f, 0.3f, 0.38f), Blk, 0.6f, 0.f));
	Apply(GHat, JJ::Mat(this, FLinearColor(0.025f, 0.022f, 0.028f), Blk, 0.75f, 0.f));
	Apply(GBrown, JJ::Mat(this, FLinearColor(0.42f, 0.26f, 0.13f), Blk, 0.8f, 0.f));
	for (UMeshComponent* C : NeonOnly) if (C) C->SetVisibility(bN);
	for (UMeshComponent* C : RuinaOnly) if (C) C->SetVisibility(CoopRole == ECoopRole::Ruina);
	const bool bV = CoopRole == ECoopRole::Vertice;
	// Neon e Vértice usam as malhas importadas: escondem o corpo procedural (a arma continua)
	LoadPapel();
	const bool bVMesh = GetMesh()->GetSkeletalMeshAsset() != nullptr;
	TArray<UProceduralMeshComponent*> Parts; GetComponents<UProceduralMeshComponent>(Parts);
	for (UProceduralMeshComponent* C : Parts)
		if (C && C != GunBody && C != GunBarrel && C != GunTip && C != ProngA && C != ProngB) C->SetVisibility(!bVMesh);
	for (UMeshComponent* C : VertOnly) if (C) C->SetVisibility(bV && !bVMesh);
	if (bVMesh) { for (UMeshComponent* C : NeonOnly) if (C) C->SetVisibility(false); for (UMeshComponent* C : RuinaOnly) if (C) C->SetVisibility(false); }
	GetMesh()->SetVisibility(bVMesh);
	if (bVMesh) { VAnimCur = nullptr; UpdateVerticeAnim(); }
	if (bV)
	{
		// Vértice: armadura branca acetinada, grafite, detalhes laranja/âmbar, cachecol laranja
		const FLinearColor Amber(1.f, 0.42f, 0.04f);
		UMaterialInstanceDynamic* White = JJ::Mat(this, FLinearColor(0.6f, 0.58f, 0.55f), Blk, 0.5f, 0.1f);
		UMaterialInstanceDynamic* Graph = JJ::Mat(this, FLinearColor(0.06f, 0.06f, 0.065f), Blk, 0.45f, 0.6f);
		UMaterialInstanceDynamic* Glow = JJ::Mat(this, FLinearColor(0.03f, 0.015f, 0.f), Amber * 2.f, 0.4f, 0.f);
		Apply(GMain, White); Apply(GVHood, White);
		Apply(GPanel, JJ::Mat(this, FLinearColor(0.36f, 0.35f, 0.34f), Blk, 0.45f, 0.35f));
		Apply(GDark, Graph); Apply(GVDark, Graph);
		Apply(GFace, JJ::Mat(this, FLinearColor(0.015f, 0.015f, 0.018f), Blk, 0.3f, 0.2f));
		Apply(GAccent, Glow); Apply(GVAccent, Glow);
		Apply(GCloth, JJ::Mat(this, FLinearColor(0.6f, 0.22f, 0.03f), Blk, 0.85f, 0.f));
	}
	ApplyGunLook();
}

void ACoopCharacter::AnimateBody(float DT)
{
	if (!UpperPivot || !LegLPivot) return;
	const FVector V = GetVelocity();
	const FVector V2(V.X, V.Y, 0.f);
	const float Sp = V2.Size();
	const float Fwd = FVector::DotProduct(V2, GetActorForwardVector());
	const bool bAir = GetCharacterMovement() && GetCharacterMovement()->IsFalling();
	AnimSpeed = FMath::FInterpTo(AnimSpeed, FMath::Clamp(Sp / 600.f, 0.f, 1.f), DT, 10.f);
	AirBlend = FMath::FInterpTo(AirBlend, bAir ? 1.f : 0.f, DT, 12.f);
	AnimPhase += DT * (Sp / 55.f) * (Fwd < -20.f ? -1.f : 1.f);
	AnimTime += DT;
	const float Ground = 1.f - AirBlend;
	const float Amp = 42.f * AnimSpeed * Ground;
	const float S = FMath::Sin(AnimPhase);
	const float Sway = FMath::Sin(AnimTime * 1.6f);
	const float AimPitch = FRotator::NormalizeAxis(GetBaseAimRotation().Pitch);

	LegLPivot->SetRelativeRotation(FRotator(Amp * S + AirBlend * 28.f, 0, 0));
	LegRPivot->SetRelativeRotation(FRotator(-Amp * S - AirBlend * 12.f, 0, 0));
	ArmLPivot->SetRelativeRotation(FRotator(-Amp * 0.9f * S - AirBlend * 55.f + Sway * 3.f * Ground, 0, -4.f - AirBlend * 25.f));
	ArmRPivot->SetRelativeRotation(FRotator(90.f + FMath::Clamp(AimPitch, -60.f, 60.f), 0, 0));

	const float Bob = FMath::Abs(S) * 3.5f * AnimSpeed * Ground + Sway * 1.2f * (1.f - AnimSpeed) * Ground + AirBlend * 2.f;
	UpperPivot->SetRelativeLocation(FVector(0, 0, Bob));
	UpperPivot->SetRelativeRotation(FRotator(-AnimSpeed * 7.f * Ground - AirBlend * 5.f, 0, 0));
	HeadPivot->SetRelativeRotation(FRotator(FMath::Clamp(AimPitch, -45.f, 45.f) * 0.5f - UpperPivot->GetRelativeRotation().Pitch * 0.6f, 0, 0));
	const float Flutter = FMath::Sin(AnimTime * 7.f) * 4.f * AnimSpeed;
	CapePivot->SetRelativeRotation(FRotator(FMath::Clamp(-(Fwd / 600.f) * 38.f - 6.f - AirBlend * 25.f + Flutter, -75.f, 8.f), 0, 0));

	// arma segue a mira, na mão direita
	const FRotator Aim = GetBaseAimRotation();
	GunPivot->SetWorldLocation(ArmRPivot->GetComponentLocation() + Aim.Vector() * 46.f);
	GunPivot->SetWorldRotation(Aim);
}

void ACoopCharacter::ClientSetupCamera_Implementation()
{
	if (Controller) Controller->SetControlRotation(FRotator(-18.f, 0.f, 0.f));
}

void ACoopCharacter::PossessedBy(AController* C)
{
	Super::PossessedBy(C);
	int32 Idx = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It, ++Idx)
		if (It->Get() == C) break;
	CoopRole = (Idx == 0) ? ECoopRole::Neon : (Idx == 1) ? ECoopRole::Ruina : ECoopRole::Vertice;
	OnRep_Role();
	ClientSetupCamera();
}

void ACoopCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopCharacter, CoopRole);
	DOREPLIFETIME(ACoopCharacter, bPortalMode);
}

void ACoopCharacter::SetupPlayerInputComponent(UInputComponent* I)
{
	I->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ACharacter::Jump);
	I->BindKey(EKeys::SpaceBar, IE_Released, this, &ACharacter::StopJumping);
	I->BindKey(EKeys::E, IE_Pressed, this, &ACoopCharacter::Primary);
	I->BindKey(EKeys::Q, IE_Pressed, this, &ACoopCharacter::Secondary);
	I->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACoopCharacter::FireA);
	I->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ACoopCharacter::FireB);
	I->BindKey(EKeys::R, IE_Pressed, this, &ACoopCharacter::ToggleMode);
	I->BindKey(EKeys::F3, IE_Pressed, this, &ACoopCharacter::TestVertice);
	I->BindKey(EKeys::F1, IE_Pressed, this, &ACoopCharacter::ShowHelp);
	I->BindKey(EKeys::F, IE_Pressed, this, &ACoopCharacter::DestructPower);
	I->BindKey(EKeys::F4, IE_Pressed, this, &ACoopCharacter::TestBridge);
	I->BindAxisKey(EKeys::MouseX, this, &ACoopCharacter::LookX);
	I->BindAxisKey(EKeys::MouseY, this, &ACoopCharacter::LookY);
}

static FRotator YawOf(const AController* C) { return FRotator(0, C->GetControlRotation().Yaw, 0); }
void ACoopCharacter::MoveF(float V) { if (Controller && V) AddMovementInput(FRotationMatrix(YawOf(Controller)).GetUnitAxis(EAxis::X), V); }
void ACoopCharacter::MoveR(float V) { if (Controller && V) AddMovementInput(FRotationMatrix(YawOf(Controller)).GetUnitAxis(EAxis::Y), V); }
void ACoopCharacter::MoveB(float V) { MoveF(-V); }
void ACoopCharacter::MoveL(float V) { MoveR(-V); }
void ACoopCharacter::LookX(float V) { const UCoopGameUserSettings* S = UCoopGameUserSettings::Get(); AddControllerYawInput(V * (S ? S->MouseSensitivity : 1.f)); }
void ACoopCharacter::LookY(float V) { const UCoopGameUserSettings* S = UCoopGameUserSettings::Get(); AddControllerPitchInput(-V * (S ? S->MouseSensitivity : 1.f)); }

ACoopProp* ACoopCharacter::TraceProp() const
{
	if (!Controller) return nullptr;
	FVector Loc; FRotator Rot;
	Controller->GetPlayerViewPoint(Loc, Rot);
	const FVector Dir = Rot.Vector();
	FCollisionQueryParams P(NAME_None, false, this);
	FHitResult Hit;
	if (GetWorld()->SweepSingleByChannel(Hit, Loc, Loc + Dir * AbilityRange, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(60.f), P))
		if (ACoopProp* Pr = Cast<ACoopProp>(Hit.GetActor())) return Pr;
	// assistência de mira: cubo visível dentro de ~6 graus do centro da tela
	ACoopProp* Best = nullptr;
	float BestDot = FMath::Cos(FMath::DegreesToRadians(6.f));
	for (TActorIterator<ACoopProp> It(GetWorld()); It; ++It)
	{
		const FVector To = It->GetActorLocation() - Loc;
		const float D = To.Size();
		if (D < 1.f || D > AbilityRange) continue;
		const float Dot = FVector::DotProduct(Dir, To / D);
		if (Dot < BestDot) continue;
		FHitResult H2;
		if (GetWorld()->LineTraceSingleByChannel(H2, Loc, It->GetActorLocation(), ECC_Visibility, P) && H2.GetActor() != *It) continue;
		BestDot = Dot; Best = *It;
	}
	return Best;
}

void ACoopCharacter::Primary()
{
	if (ACoopProp* P = TraceProp()) ServerUseAbility(P, false);
	else ClientToast(TEXT("Sem alvo: mire no cubo"));
}
void ACoopCharacter::Secondary()
{
	if (ACoopProp* P = TraceProp()) ServerUseAbility(P, true);
	else ClientToast(TEXT("Sem alvo: mire no cubo"));
}

void ACoopCharacter::ClientToast_Implementation(const FString& Text)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(101, 2.5f, FColor::Green, Text);
}

void ACoopCharacter::ServerUseAbility_Implementation(ACoopProp* T, bool bSecondary)
{
	if (!T || FVector::Dist(T->GetActorLocation(), GetActorLocation()) > AbilityRange * 1.5f) return;
	const FVector From = GunTip ? GunTip->GetComponentLocation() : GetActorLocation() + FVector(0, 0, 40);
	if (CoopRole == ECoopRole::Vertice) { ClientToast(TEXT("VERTICE: poder em desenvolvimento")); return; }
	if (CoopRole == ECoopRole::Neon)
	{
		if (!bSecondary)
		{
			T->FlipGravity();
			ClientToast(T->IsReversed() ? TEXT("Gravidade INVERTIDA") : TEXT("Gravidade NORMAL"));
			T->MulticastBurst(FLinearColor(0.1f, 1.f, 0.3f), From);
		}
	}
	else
	{
		if (bSecondary) { T->StartRewind(); ClientToast(TEXT("Rebobinando...")); T->MulticastBurst(FLinearColor(0.f, 0.8f, 1.f), From); }
		else { T->ToggleFreeze(); ClientToast(T->IsFrozen() ? TEXT("Congelado") : TEXT("Descongelado")); T->MulticastBurst(FLinearColor(0.6f, 0.3f, 1.f), From); }
	}
}

ACoopDestructible* ACoopCharacter::TraceDestructible() const
{
	// mira por angulo (funciona tambem com o objeto destruido, que nao tem colisao)
	if (!Controller) return nullptr;
	FVector Loc; FRotator Rot;
	Controller->GetPlayerViewPoint(Loc, Rot);
	const FVector Dir = Rot.Vector();
	FCollisionQueryParams P(NAME_None, false, this);
	ACoopDestructible* Best = nullptr;
	float BestDot = FMath::Cos(FMath::DegreesToRadians(25.f)); // cone mais largo: mais facil de mirar
	for (TActorIterator<ACoopDestructible> It(GetWorld()); It; ++It)
	{
		const FVector To = It->GetActorLocation() - Loc;
		const float D = To.Size();
		if (D < 1.f || D > DestructRange + 400.f) continue;
		const float Dot = FVector::DotProduct(Dir, To / D);
		if (Dot < BestDot) continue;
		FHitResult H;
		if (GetWorld()->LineTraceSingleByChannel(H, Loc, It->GetActorLocation(), ECC_Visibility, P) && H.GetActor() != *It) continue;
		BestDot = Dot; Best = *It;
	}
	return Best;
}

void ACoopCharacter::DestructPower()
{
	if (CoopRole == ECoopRole::Vertice) { ClientToast(TEXT("VERTICE nao usa este poder")); return; }
	if (ACoopDestructible* T = TraceDestructible()) ServerDestructPower(T);
	else ClientToast(CoopRole == ECoopRole::Ruina ? TEXT("Mire num objeto destrutivel") : TEXT("Mire num objeto destruido"));
}

void ACoopCharacter::ServerDestructPower_Implementation(ACoopDestructible* T)
{
	// validacao no servidor: personagem, alcance, linha de visao e estado
	if (!T) return;
	if (CoopRole != ECoopRole::Ruina && CoopRole != ECoopRole::Neon) { ClientToast(TEXT("Este personagem nao tem esse poder")); return; }
	const FVector Eye = GetActorLocation() + FVector(0, 0, BaseEyeHeight);
	if (FVector::Dist(Eye, T->GetActorLocation()) > DestructRange) { ClientToast(TEXT("Longe demais - chegue mais perto")); return; }
	FCollisionQueryParams P(NAME_None, false, this);
	FHitResult H;
	if (GetWorld()->LineTraceSingleByChannel(H, Eye, T->GetActorLocation(), ECC_Visibility, P) && H.GetActor() != T)
	{ ClientToast(TEXT("Sem linha de visao")); return; }
	FString Why;
	const bool bOk = (CoopRole == ECoopRole::Ruina) ? T->ServerBreak(Why) : T->ServerRebuild(Why);
	if (bOk) ClientToast(CoopRole == ECoopRole::Ruina ? TEXT("RUINA: objeto DESTRUIDO") : TEXT("NEON: objeto RECONSTRUIDO"));
	else ClientToast(Why);
}

void ACoopCharacter::ShowHelp()
{
	if (!GEngine) return;
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("=== JOINT JOURNEY ==="));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, TEXT("WASD mover | Espaco pular | Mouse mirar | Botao esq/E poder | Botao dir/Q secundario"));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, TEXT("R alterna ARMA DE PODERES / ARMA DE PORTAIS (esq = azul, dir = laranja)"));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, ECoopRole::Neon == CoopRole ? FColor::Cyan : FColor::Orange,
		CoopRole == ECoopRole::Neon ? TEXT("Voce e o NEON: poder inverte a gravidade do cubo mirado") : CoopRole == ECoopRole::Vertice ? TEXT("Voce e o VERTICE (poder em breve)") : TEXT("Voce e a RUINA: poder congela o cubo, Q rebobina 5s"));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange, TEXT("F3 (teste): alterna NEON -> RUINA -> VERTICE"));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Magenta, TEXT("F: NOVO PODER - Ruina destroi / Neon reconstroi o bloco roxo"));
}

void ACoopCharacter::ApplyGunLook()
{
	if (!GunBody) return;
	const FLinearColor G(0.1f, 0.9f, 0.4f), P(0.7f, 0.3f, 1.f), Blue(0.1f, 0.5f, 1.f), Orange(1.f, 0.45f, 0.05f);
	const FLinearColor RC = CoopRole == ECoopRole::Neon ? G : CoopRole == ECoopRole::Vertice ? Orange : P;
	UMaterialInstanceDynamic* W = JJ::Mat(this, FLinearColor(0.8f, 0.8f, 0.84f), FLinearColor::Black, 0.3f, 0.5f);
	GunBody->SetMaterial(0, W); GunBarrel->SetMaterial(0, W);
	GunTip->SetMaterial(0, JJ::Mat(this, FLinearColor::Black, bPortalMode ? FLinearColor::White * 3.f : RC * 4.f, 0.2f));
	ProngA->SetMaterial(0, JJ::Mat(this, FLinearColor::Black, (bPortalMode ? Blue : RC) * 4.f, 0.2f));
	ProngB->SetMaterial(0, JJ::Mat(this, FLinearColor::Black, (bPortalMode ? Orange : RC) * 4.f, 0.2f));
}

void ACoopCharacter::OnRep_Mode() { ApplyGunLook(); }

void ACoopCharacter::ToggleMode() { ServerSetMode(!bPortalMode); }

void ACoopCharacter::ServerSetMode_Implementation(bool bPortal)
{
	bPortalMode = bPortal;
	OnRep_Mode();
	ClientToast(bPortal ? TEXT("ARMA DE PORTAIS (esq azul / dir laranja)") : TEXT("ARMA DE PODERES"));
}

void ACoopCharacter::FireA() { if (bPortalMode) FirePortal(false); else Primary(); }
void ACoopCharacter::FireB() { if (bPortalMode) FirePortal(true); else Secondary(); }

void ACoopCharacter::FirePortal(bool bOrange)
{
	if (!Controller) return;
	FVector Loc; FRotator Rot;
	Controller->GetPlayerViewPoint(Loc, Rot);
	FHitResult Hit;
	FCollisionQueryParams P(NAME_None, false, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Loc, Loc + Rot.Vector() * 10000.f, ECC_Visibility, P) && Cast<ACoopBlock>(Hit.GetActor()))
		ServerPlacePortal(bOrange, Hit.ImpactPoint, Hit.ImpactNormal);
	else
		ClientToast(TEXT("Superficie invalida para portal"));
}

void ACoopCharacter::ServerPlacePortal_Implementation(bool bOrange, FVector_NetQuantize10 Pt, FVector_NetQuantizeNormal N)
{
	FVector Loc = Pt;
	const FVector Nrm = FVector(N).GetSafeNormal();
	if (Nrm.IsNearlyZero() || FVector::Dist(Loc, GetActorLocation()) > 11000.f) return;
	if (FMath::Abs(Nrm.Z) < 0.5f) Loc.Z = FMath::Clamp(Loc.Z, 118.f, 282.f);
	ACoopPortal* Found = nullptr;
	for (TActorIterator<ACoopPortal> It(GetWorld()); It; ++It)
		if (It->bOrange == bOrange) Found = *It;
	if (!Found) Found = GetWorld()->SpawnActor<ACoopPortal>(ACoopPortal::StaticClass(), Loc, FRotator::ZeroRotator);
	if (Found) Found->Setup(bOrange, Loc, Nrm);
}

void ACoopCharacter::TestVertice()
{
	// ciclo de teste solo: Neon -> Ruina -> Vertice -> Neon ... (sem precisar abrir 2/3 janelas)
	const ECoopRole Next = (CoopRole == ECoopRole::Neon) ? ECoopRole::Ruina : (CoopRole == ECoopRole::Ruina) ? ECoopRole::Vertice : ECoopRole::Neon;
	ServerSetRole(Next);
}

void ACoopCharacter::ServerSetRole_Implementation(ECoopRole NewRole)
{
	CoopRole = NewRole;
	OnRep_Role();
	ClientToast(NewRole == ECoopRole::Neon ? TEXT("Teste: agora voce e o NEON (F3 para trocar)") : NewRole == ECoopRole::Ruina ? TEXT("Teste: agora voce e a RUINA (F3 para trocar)") : TEXT("Teste: agora voce e o VERTICE (F3 para trocar)"));
}

void ACoopCharacter::LoadPapel()
{
	// carrega a malha e as animacoes do papel atual (Neon, Ruina e Vertice tem malha propria)
	if (PapelMalha == (uint8)CoopRole) return;
	const bool bN = CoopRole == ECoopRole::Neon, bR = CoopRole == ECoopRole::Ruina;
	const FString Pasta = bR ? TEXT("Ruina") : bN ? TEXT("Neon") : TEXT("Vertice");
	const FString Pre = bR ? TEXT("RUI_") : bN ? TEXT("NEO_") : TEXT("VRT_");
	USkeletalMesh* SK = LoadObject<USkeletalMesh>(nullptr, *FString::Printf(TEXT("/Game/%s/Mesh/SK_%s.SK_%s"), *Pasta, *Pasta, *Pasta));
	PapelMalha = (uint8)CoopRole; // marca como tentado mesmo se falhar, pra nao travar no papel anterior
	if (!SK)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red, FString::Printf(TEXT("ERRO: nao achei /Game/%s/Mesh/SK_%s"), *Pasta, *Pasta));
		GetMesh()->SetSkeletalMesh(nullptr); // cai pro corpo procedural em vez de ficar preso na malha anterior
		return;
	}
	GetMesh()->SetSkeletalMesh(SK);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	auto LA = [&](const TCHAR* N) { const FString A = Pre + N; return LoadObject<UAnimSequence>(nullptr, *FString::Printf(TEXT("/Game/%s/Anims/%s.%s"), *Pasta, *A, *A)); };
	VAnimIdle = LA(TEXT("Idle")); VAnimWalk = LA(TEXT("Walk_InPlace")); VAnimRun = LA(TEXT("Run_Fwd"));
	VAnimJump = LA(TEXT("Jump")); VAnimFall = LA(TEXT("Fall_Loop"));
	VAnimCur = nullptr;
}

void ACoopCharacter::PlayV(UAnimSequence* A, bool bLoop, float Rate)
{
	if (!A) return;
	if (A != VAnimCur) { GetMesh()->PlayAnimation(A, bLoop); VAnimCur = A; }
	GetMesh()->SetPlayRate(Rate);
}

// roda em todas as máquinas (servidor e clientes) a partir da velocidade replicada: animação sincronizada no online
void ACoopCharacter::UpdateVerticeAnim()
{
	if (!GetMesh()->IsVisible()) return;
	const FVector V = GetVelocity();
	const float Sp = V.Size2D();
	const bool bFalling = GetCharacterMovement() && GetCharacterMovement()->IsFalling();
	const float Dir = FVector::DotProduct(FVector(V.X, V.Y, 0.f), GetActorForwardVector()) < -20.f ? -1.f : 1.f;
	if (bFalling)
	{
		if (V.Z > 50.f && (VAnimCur == VAnimJump || !bVWasFalling)) PlayV(VAnimJump, false);
		else if (VAnimCur != VAnimJump || V.Z < -50.f) PlayV(VAnimFall, true);
	}
	else if (Sp < 15.f) PlayV(VAnimIdle, true);
	else if (Sp < 340.f) PlayV(VAnimWalk, true, Dir * FMath::Clamp(Sp / 220.f, 0.5f, 1.5f));
	else PlayV(VAnimRun, true, Dir * FMath::Clamp(Sp / 480.f, 0.6f, 1.4f));
	bVWasFalling = bFalling;
}

void ACoopCharacter::TestBridge() { ServerToggleTestBridge(); }

void ACoopCharacter::ServerToggleTestBridge_Implementation()
{
	if (ACoopFase1GameMode* GM = GetWorld()->GetAuthGameMode<ACoopFase1GameMode>())
		ClientToast(GM->ToggleTestBridge() ? TEXT("TESTE: ponte COMPLETA ativada (F4 desfaz)") : TEXT("TESTE: ponte quebrada de novo"));
	else ClientToast(TEXT("F4 so funciona no mapa Fase1_Teste"));
}
