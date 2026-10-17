#include "CoopCharacter.h"
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

	// tronco (superfícies arredondadas, mais largo nos ombros)
	UpperPivot = Pv(TEXT("UpperPivot"), Cap, FVector::ZeroVector);
	Mk(TEXT("Torso"), UpperPivot, FVector(0, 0, 8), Z0, JJShape(22, 27, 27, 0.65f, 0.7f, 1.12f, 18, 12), GMain);
	Mk(TEXT("ChestRing"), UpperPivot, FVector(21.8f, 0, 12), FRotator(90, 0, 0), JJShape(10, 10, 1.3f, 0.35f, 1, 1, 20, 6), GAccent);
	Mk(TEXT("ChestInner"), UpperPivot, FVector(22.9f, 0, 12), FRotator(90, 0, 0), JJShape(6.2f, 6.2f, 1.3f, 0.35f, 1, 1, 16, 6), GDark);
	Mk(TEXT("Pelvis"), Cap, FVector(0, 0, -24), Z0, JJShape(18, 22, 9, 1, 1, 1, 14, 8), GDark);
	Mk(TEXT("Neck"), UpperPivot, FVector(0, 0, 38), Z0, JJShape(7, 7, 6, 0.6f, 0.6f, 1, 10, 6), GDark);
	Mk(TEXT("Scarf"), UpperPivot, FVector(0, 0, 41), Z0, JJShape(18, 19, 6, 0.7f, 0.8f, 1, 16, 8), GCloth);
	CapePivot = Pv(TEXT("CapePivot"), UpperPivot, FVector(-20, 0, 36));
	Mk(TEXT("Cape"), CapePivot, FVector(-3, 0, -32), Z0, JJShape(1.8f, 26, 32, 0.45f, 0.5f, 0.8f, 10, 10), GCloth);

	// braços: ombro redondo, antebraço grosso, mão robusta
	ArmLPivot = Pv(TEXT("ArmLPivot"), UpperPivot, FVector(0, -36, 24));
	ArmRPivot = Pv(TEXT("ArmRPivot"), UpperPivot, FVector(0, 36, 24));
	Mk(TEXT("PadL"), ArmLPivot, FVector(0, 0, 2), Z0, JJShape(15, 15, 14, 1, 1, 1, 14, 8), GMain);
	Mk(TEXT("PadR"), ArmRPivot, FVector(0, 0, 2), Z0, JJShape(15, 15, 14, 1, 1, 1, 14, 8), GMain);
	Mk(TEXT("ShRingL"), ArmLPivot, FVector(0, -14.6f, 2), FRotator(0, 0, 90), JJShape(8, 8, 1.3f, 0.35f, 1, 1, 16, 6), GAccent);
	Mk(TEXT("ShRingR"), ArmRPivot, FVector(0, 14.6f, 2), FRotator(0, 0, 90), JJShape(8, 8, 1.3f, 0.35f, 1, 1, 16, 6), GAccent);
	Mk(TEXT("ShInL"), ArmLPivot, FVector(0, -15.6f, 2), FRotator(0, 0, 90), JJShape(4.8f, 4.8f, 1.3f, 0.35f, 1, 1, 12, 6), GDark);
	Mk(TEXT("ShInR"), ArmRPivot, FVector(0, 15.6f, 2), FRotator(0, 0, 90), JJShape(4.8f, 4.8f, 1.3f, 0.35f, 1, 1, 12, 6), GDark);
	Mk(TEXT("ArmL"), ArmLPivot, FVector(0, 0, -26), Z0, JJShape(10.5f, 10.5f, 22, 0.55f, 1, 0.82f, 14, 10), GMain);
	Mk(TEXT("ArmR"), ArmRPivot, FVector(0, 0, -26), Z0, JJShape(10.5f, 10.5f, 22, 0.55f, 1, 0.82f, 14, 10), GMain);
	Mk(TEXT("BraceL"), ArmLPivot, FVector(0, 0, -42), Z0, JJShape(12.5f, 12.5f, 1.6f, 0.35f, 1, 1, 16, 6), GAccent);
	Mk(TEXT("BraceR"), ArmRPivot, FVector(0, 0, -42), Z0, JJShape(12.5f, 12.5f, 1.6f, 0.35f, 1, 1, 16, 6), GAccent);
	Mk(TEXT("HandL"), ArmLPivot, FVector(0, 0, -58), Z0, JJShape(12.5f, 13.5f, 12, 0.8f, 0.8f, 1, 14, 10), GDark);
	Mk(TEXT("HandR"), ArmRPivot, FVector(0, 0, -58), Z0, JJShape(12.5f, 13.5f, 12, 0.8f, 0.8f, 1, 14, 10), GDark);
	Mk(TEXT("FingL"), ArmLPivot, FVector(9, 0, -61), Z0, JJShape(6, 11, 9, 0.8f, 0.8f, 1, 10, 8), GDark);
	Mk(TEXT("FingR"), ArmRPivot, FVector(9, 0, -61), Z0, JJShape(6, 11, 9, 0.8f, 0.8f, 1, 10, 8), GDark);

	// pernas grossas, joelho esférico, botas arredondadas
	LegLPivot = Pv(TEXT("LegLPivot"), Cap, FVector(0, -14, -28));
	LegRPivot = Pv(TEXT("LegRPivot"), Cap, FVector(0, 14, -28));
	Mk(TEXT("LegL"), LegLPivot, FVector(0, 0, -24), Z0, JJShape(10.5f, 10.5f, 21, 0.6f, 1, 1.1f, 14, 10), GMain);
	Mk(TEXT("LegR"), LegRPivot, FVector(0, 0, -24), Z0, JJShape(10.5f, 10.5f, 21, 0.6f, 1, 1.1f, 14, 10), GMain);
	Mk(TEXT("KneeL"), LegLPivot, FVector(0, 0, -23), Z0, JJShape(11.5f, 11.5f, 11, 1, 1, 1, 12, 8), GDark);
	Mk(TEXT("KneeR"), LegRPivot, FVector(0, 0, -23), Z0, JJShape(11.5f, 11.5f, 11, 1, 1, 1, 12, 8), GDark);
	Mk(TEXT("AnkleL"), LegLPivot, FVector(0, 0, -44), Z0, JJShape(11, 11, 1.5f, 0.35f, 1, 1, 14, 6), GAccent);
	Mk(TEXT("AnkleR"), LegRPivot, FVector(0, 0, -44), Z0, JJShape(11, 11, 1.5f, 0.35f, 1, 1, 14, 6), GAccent);
	Mk(TEXT("BootL"), LegLPivot, FVector(7, 0, -50), Z0, JJShape(18, 13.5f, 10, 0.75f, 0.8f, 1, 16, 10), GMain);
	Mk(TEXT("BootR"), LegRPivot, FVector(7, 0, -50), Z0, JJShape(18, 13.5f, 10, 0.75f, 0.8f, 1, 16, 10), GMain);
	Mk(TEXT("SoleL"), LegLPivot, FVector(7, 0, -58.8f), Z0, JJShape(18.5f, 14, 1.6f, 0.35f, 0.8f, 1, 16, 6), GAccent);
	Mk(TEXT("SoleR"), LegRPivot, FVector(7, 0, -58.8f), Z0, JJShape(18.5f, 14, 1.6f, 0.35f, 0.8f, 1, 16, 6), GAccent);

	// cabeça esférica grande + máscara escura
	HeadPivot = Pv(TEXT("HeadPivot"), UpperPivot, FVector(0, 0, 40));
	Mk(TEXT("Head"), HeadPivot, FVector(0, 0, 22), Z0, JJShape(30, 30, 30, 1, 1, 1, 24, 16), GMain);
	Mk(TEXT("Face"), HeadPivot, FVector(12, 0, 22), Z0, JJShape(20, 25, 23, 1, 1, 1, 16, 12), GDark);
	// Neon: olhos ovais amigáveis + boné
	Mk(TEXT("EyeNL"), HeadPivot, FVector(31, -9.5f, 24), Z0, JJShape(2.5f, 4.8f, 8, 1, 1, 1, 10, 8), GEyesN);
	Mk(TEXT("EyeNR"), HeadPivot, FVector(31, 9.5f, 24), Z0, JJShape(2.5f, 4.8f, 8, 1, 1, 1, 10, 8), GEyesN);
	Mk(TEXT("CapDome"), HeadPivot, FVector(0, 0, 35), Z0, JJShape(31.5f, 31.5f, 19, 1, 1, 1, 20, 10), GCap);
	Mk(TEXT("CapBrim"), HeadPivot, FVector(30, 0, 31), FRotator(-8, 0, 0), JJShape(19, 24, 2, 0.5f, 0.7f, 1, 14, 6), GCap);
	// Ruína: olhos bravos, bigode, chapéu fedora
	Mk(TEXT("EyeRL"), HeadPivot, FVector(31, -10, 26), FRotator(0, 0, -25), JJShape(2.2f, 8.5f, 3.2f, 0.6f, 0.6f, 1, 10, 6), GEyesR);
	Mk(TEXT("EyeRR"), HeadPivot, FVector(31, 10, 26), FRotator(0, 0, 25), JJShape(2.2f, 8.5f, 3.2f, 0.6f, 0.6f, 1, 10, 6), GEyesR);
	Mk(TEXT("MustL"), HeadPivot, FVector(30, -8, 10), FRotator(0, 0, 65), JJShape(2.8f, 2.8f, 9, 0.7f, 1, 0.6f, 8, 6), GBrown);
	Mk(TEXT("MustR"), HeadPivot, FVector(30, 8, 10), FRotator(0, 0, 115), JJShape(2.8f, 2.8f, 9, 0.7f, 1, 0.6f, 8, 6), GBrown);
	Mk(TEXT("MustC"), HeadPivot, FVector(31.5f, 0, 10), Z0, JJShape(4, 5, 3.2f, 1, 1, 1, 8, 6), GBrown);
	Mk(TEXT("HatBrim"), HeadPivot, FVector(0, 0, 38), Z0, JJShape(46, 46, 2.2f, 0.35f, 1, 1, 24, 6), GHat);
	Mk(TEXT("HatCrown"), HeadPivot, FVector(0, 0, 52), Z0, JJShape(25, 24, 15, 0.45f, 0.6f, 0.92f, 18, 10), GHat);
	Mk(TEXT("HatBand"), HeadPivot, FVector(0, 0, 43), Z0, JJShape(25.8f, 25, 3.4f, 0.4f, 0.6f, 1, 18, 6), GBand);

	GAccent.Append(GEyesN); GAccent.Append(GEyesR); GAccent.Append(GBand);
	NeonOnly.Append(GEyesN); NeonOnly.Append(GCap);
	RuinaOnly.Append(GEyesR); RuinaOnly.Append(GHat); RuinaOnly.Append(GBrown); RuinaOnly.Append(GBand);

	// arma estilo Portal (arredondada)
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
	const FLinearColor Acc = bN ? FLinearColor(0.1f, 0.95f, 0.85f) : FLinearColor(0.7f, 0.3f, 1.f);
	auto Apply = [](TArray<UMeshComponent*>& G, UMaterialInstanceDynamic* M) { for (UMeshComponent* C : G) if (C) C->SetMaterial(0, M); };
	Apply(GMain, bN ? JJ::Mat(this, FLinearColor(0.86f, 0.85f, 0.8f), Blk, 0.45f, 0.1f) : JJ::Mat(this, FLinearColor(0.012f, 0.012f, 0.018f), Blk, 0.3f, 0.7f));
	Apply(GDark, bN ? JJ::Mat(this, FLinearColor(0.02f, 0.03f, 0.04f), Blk, 0.2f, 0.8f) : JJ::Mat(this, FLinearColor(0.035f, 0.035f, 0.045f), Blk, 0.5f, 0.4f));
	Apply(GAccent, JJ::Mat(this, Acc * 0.3f, Acc * 3.f, 0.2f, 0.f));
	Apply(GCloth, bN ? JJ::Mat(this, FLinearColor(0.04f, 0.5f, 0.55f), Blk, 0.7f, 0.f) : JJ::Mat(this, FLinearColor(0.05f, 0.03f, 0.09f), Blk, 0.8f, 0.f));
	Apply(GCap, JJ::Mat(this, FLinearColor(0.05f, 0.45f, 0.55f), Blk, 0.6f, 0.f));
	Apply(GHat, JJ::Mat(this, FLinearColor(0.03f, 0.025f, 0.03f), Blk, 0.8f, 0.f));
	Apply(GBrown, JJ::Mat(this, FLinearColor(0.35f, 0.2f, 0.1f), Blk, 0.7f, 0.f));
	for (UMeshComponent* C : NeonOnly) if (C) C->SetVisibility(bN);
	for (UMeshComponent* C : RuinaOnly) if (C) C->SetVisibility(!bN);
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
	GunPivot->SetWorldLocation(ArmRPivot->GetComponentLocation() + Aim.Vector() * 40.f);
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
	CoopRole = (Idx == 0) ? ECoopRole::Neon : ECoopRole::Ruina;
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
	I->BindKey(EKeys::F1, IE_Pressed, this, &ACoopCharacter::ShowHelp);
	I->BindAxisKey(EKeys::MouseX, this, &ACoopCharacter::LookX);
	I->BindAxisKey(EKeys::MouseY, this, &ACoopCharacter::LookY);
}

static FRotator YawOf(const AController* C) { return FRotator(0, C->GetControlRotation().Yaw, 0); }
void ACoopCharacter::MoveF(float V) { if (Controller && V) AddMovementInput(FRotationMatrix(YawOf(Controller)).GetUnitAxis(EAxis::X), V); }
void ACoopCharacter::MoveR(float V) { if (Controller && V) AddMovementInput(FRotationMatrix(YawOf(Controller)).GetUnitAxis(EAxis::Y), V); }
void ACoopCharacter::MoveB(float V) { MoveF(-V); }
void ACoopCharacter::MoveL(float V) { MoveR(-V); }
void ACoopCharacter::LookX(float V) { AddControllerYawInput(V); }
void ACoopCharacter::LookY(float V) { AddControllerPitchInput(-V); }

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

void ACoopCharacter::ShowHelp()
{
	if (!GEngine) return;
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("=== JOINT JOURNEY ==="));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, TEXT("WASD mover | Espaco pular | Mouse mirar | Botao esq/E poder | Botao dir/Q secundario"));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, TEXT("R alterna ARMA DE PODERES / ARMA DE PORTAIS (esq = azul, dir = laranja)"));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, ECoopRole::Neon == CoopRole ? FColor::Cyan : FColor::Orange,
		CoopRole == ECoopRole::Neon ? TEXT("Voce e o NEON: poder inverte a gravidade do cubo mirado") : TEXT("Voce e a RUINA: poder congela o cubo, Q rebobina 5s"));
}

void ACoopCharacter::ApplyGunLook()
{
	if (!GunBody) return;
	const FLinearColor G(0.1f, 0.9f, 0.4f), P(0.7f, 0.3f, 1.f), Blue(0.1f, 0.5f, 1.f), Orange(1.f, 0.45f, 0.05f);
	const FLinearColor RC = CoopRole == ECoopRole::Neon ? G : P;
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
