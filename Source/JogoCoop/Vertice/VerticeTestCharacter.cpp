#include "VerticeTestCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"

AVerticeTestCharacter::AVerticeTestCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(40.f, 88.f);
	Boom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Boom"));
	Boom->SetupAttachment(RootComponent);
	Boom->TargetArmLength = 330.f;
	Boom->SocketOffset = FVector(0, 0, 50);
	Boom->bUsePawnControlRotation = true;
	Boom->bEnableCameraLag = true; Boom->CameraLagSpeed = 12.f;
	Cam = CreateDefaultSubobject<UCameraComponent>(TEXT("Cam"));
	Cam->SetupAttachment(Boom);
	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* M = GetCharacterMovement();
	M->bOrientRotationToMovement = true;
	M->RotationRate = FRotator(0, 540, 0);
	M->MaxWalkSpeed = WalkSpeed;
	M->JumpZVelocity = 620.f;
	M->AirControl = 0.35f;
	M->BrakingDecelerationWalking = 1600.f;
	// malha olhando para +X (modelo importado olha para +Y)
	GetMesh()->SetRelativeLocation(FVector(0, 0, -88.f));
	GetMesh()->SetRelativeRotation(FRotator(0, -90.f, 0));
}

void AVerticeTestCharacter::BeginPlay()
{
	Super::BeginPlay();
	// -NeonTeste usa o Neon; sem o parametro, o Vertice
	bNeon = FParse::Param(FCommandLine::Get(), TEXT("NeonTeste"));
	const bool bRuina = FParse::Param(FCommandLine::Get(), TEXT("RuinaTeste"));
	const FString Pasta = bRuina ? TEXT("Ruina") : bNeon ? TEXT("Neon") : TEXT("Vertice");
	const FString Pre = bRuina ? TEXT("RUI_") : bNeon ? TEXT("NEO_") : TEXT("VRT_");
	PastaDemo = Pasta + TEXT("Demo");
	auto LA = [&](const TCHAR* N) { const FString Nm = Pre + N; return LoadObject<UAnimSequence>(nullptr, *FString::Printf(TEXT("/Game/%s/Anims/%s.%s"), *Pasta, *Nm, *Nm)); };
	const FString MeshPath = FString::Printf(TEXT("/Game/%s/Mesh/SK_%s.SK_%s"), *Pasta, *Pasta, *Pasta);
	if (USkeletalMesh* SK = LoadObject<USkeletalMesh>(nullptr, *MeshPath))
		GetMesh()->SetSkeletalMesh(SK);
	AnimIdle = LA(TEXT("Idle")); AnimWalk = LA(TEXT("Walk_InPlace")); AnimRun = LA(TEXT("Run_Fwd"));
	AnimJump = LA(TEXT("Jump")); AnimFall = LA(TEXT("Fall_Loop")); AnimLand = LA(TEXT("Land"));
	UE_LOG(LogTemp, Display, TEXT("[Vertice] mesh=%s idle=%d walk=%d run=%d jump=%d fall=%d land=%d"),
		GetMesh()->GetSkeletalMeshAsset() ? TEXT("ok") : TEXT("FALTANDO"), AnimIdle != nullptr, AnimWalk != nullptr, AnimRun != nullptr, AnimJump != nullptr, AnimFall != nullptr, AnimLand != nullptr);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	if (FParse::Param(FCommandLine::Get(), TEXT("VerticeSemAnim"))) { AnimIdle = AnimWalk = AnimRun = AnimJump = AnimFall = AnimLand = nullptr; }
	PlayAnim(AnimIdle, true);
	bDemo = FParse::Param(FCommandLine::Get(), TEXT("VerticeDemo"));
	if (bDemo)
	{
		Boom->bEnableCameraLag = false; Boom->bDoCollisionTest = false;
		Cam->PostProcessSettings.bOverride_MotionBlurAmount = true; Cam->PostProcessSettings.MotionBlurAmount = 0.f;
	}
	if (GEngine && !bDemo)
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Orange, TEXT("TESTE VERTICE: WASD andar | Shift correr | Espaco pular | Mouse camera"));
}

void AVerticeTestCharacter::SetupPlayerInputComponent(UInputComponent* I)
{
	I->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ACharacter::Jump);
	I->BindKey(EKeys::SpaceBar, IE_Released, this, &ACharacter::StopJumping);
	I->BindKey(EKeys::LeftShift, IE_Pressed, this, &AVerticeTestCharacter::RunOn);
	I->BindKey(EKeys::LeftShift, IE_Released, this, &AVerticeTestCharacter::RunOff);
	I->BindAxisKey(EKeys::MouseX, this, &AVerticeTestCharacter::LookX);
	I->BindAxisKey(EKeys::MouseY, this, &AVerticeTestCharacter::LookY);
}
void AVerticeTestCharacter::LookX(float V) { if (!bDemo) AddControllerYawInput(V); }
void AVerticeTestCharacter::LookY(float V) { if (!bDemo) AddControllerPitchInput(-V); }
void AVerticeTestCharacter::RunOn() { bRunKey = true; }
void AVerticeTestCharacter::RunOff() { bRunKey = false; }

void AVerticeTestCharacter::PlayAnim(UAnimSequence* A, bool bLoop, float Rate)
{
	if (!A) return;
	if (A != Current) { GetMesh()->PlayAnimation(A, bLoop); Current = A; }
	GetMesh()->SetPlayRate(Rate);
}

void AVerticeTestCharacter::UpdateAnim(float DT)
{
	const UCharacterMovementComponent* M = GetCharacterMovement();
	const float Sp = GetVelocity().Size2D();
	const bool bFalling = M->IsFalling();
	if (bFalling)
	{
		if (GetVelocity().Z > 50.f && (Current == AnimJump || !bWasFalling)) PlayAnim(AnimJump, false);
		else if (Current != AnimJump || GetVelocity().Z < -50.f) PlayAnim(AnimFall, true);
	}
	else if (bWasFalling) { LandT = 0.f; }   // VRT_Land é aditiva: não toca em modo de nó único
	else if (LandT > 0.f && Sp < 50.f) { LandT -= DT; }
	else if (Sp < 15.f) PlayAnim(AnimIdle, true);
	else if (Sp < (WalkSpeed + RunSpeed) * 0.5f) PlayAnim(AnimWalk, true, FMath::Clamp(Sp / WalkSpeed, 0.5f, 1.5f));
	else PlayAnim(AnimRun, true, FMath::Clamp(Sp / RunSpeed, 0.6f, 1.4f));
	bWasFalling = bFalling;
}

void AVerticeTestCharacter::Tick(float DT)
{
	Super::Tick(DT);
	GetCharacterMovement()->MaxWalkSpeed = bRunKey ? RunSpeed : WalkSpeed;
	if (bDemo) DemoTick(DT);
	else if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		const FRotator Yaw(0, PC->GetControlRotation().Yaw, 0);
		const float F = (PC->IsInputKeyDown(EKeys::W) ? 1.f : 0.f) - (PC->IsInputKeyDown(EKeys::S) ? 1.f : 0.f);
		const float R = (PC->IsInputKeyDown(EKeys::D) ? 1.f : 0.f) - (PC->IsInputKeyDown(EKeys::A) ? 1.f : 0.f);
		if (F) AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), F);
		if (R) AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), R);
	}
	UpdateAnim(DT);
}

void AVerticeTestCharacter::Shot(const FString& Name)
{
	if (ShotsDone.Contains(Name)) return;
	ShotsDone.Add(Name);
	const FString P = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / PastaDemo / (Name + TEXT(".png")));
	FScreenshotRequest::RequestScreenshot(P, false, false);
	const FBoxSphereBounds B = GetMesh()->Bounds;
	UE_LOG(LogTemp, Display, TEXT("[Vertice] captura %s t=%.2f vel=%.0f anim=%s ator=%s cam=%s malha=%s ext=%s vis=%d"), *Name, DemoT, GetVelocity().Size2D(), Current ? *Current->GetName() : TEXT("-"),
		*GetActorLocation().ToString(), *Cam->GetComponentLocation().ToString(), *B.Origin.ToString(), *B.BoxExtent.ToString(), GetMesh()->IsVisible());
	UE_LOG(LogTemp, Display, TEXT("[Vertice]   ossos: pelvis=%s head=%s hand_l=%s foot_r=%s escala_head=%s"), *GetMesh()->GetSocketLocation("pelvis").ToString(), *GetMesh()->GetSocketLocation("head").ToString(),
		*GetMesh()->GetSocketLocation("hand_l").ToString(), *GetMesh()->GetSocketLocation("foot_r").ToString(), *GetMesh()->GetSocketTransform("head").GetScale3D().ToString());
}

// Demonstração automática: parado -> andando -> correndo -> virando -> pulando, com câmera em vários ângulos
void AVerticeTestCharacter::DemoTick(float DT)
{
	if (StableFrames < 20) { StableFrames = (DT < 0.1f) ? StableFrames + 1 : 0; DT = 0.f; }
	DemoT += DT;
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	const float t = DemoT - 4.f;   // 4 s para carregar shaders
	auto SetCam = [&](float Yaw, float Pitch) { PC->SetControlRotation(FRotator(Pitch, Yaw, 0)); };
	auto Move = [&](float Yaw) { AddMovementInput(FRotator(0, Yaw, 0).Vector(), 1.f); };
	if (t < 0.f) { SetCam(180.f, -12.f); return; }
	if (t < 3.f)       { SetCam(180.f, -12.f); if (t > 1.5f) Shot(TEXT("01_parado_frente")); }
	else if (t < 5.f)  { SetCam(140.f, -12.f); if (t > 4.2f) Shot(TEXT("02_parado_tres_quartos")); }
	else if (t < 6.5f) { SetCam(0.f, -12.f); if (t > 6.0f) Shot(TEXT("03_parado_costas")); }
	else if (t < 11.f) { bRunKey = false; Move(90.f); SetCam(-60.f, -10.f); if (t > 8.5f) Shot(TEXT("04_andando_a")); if (t > 9.3f) Shot(TEXT("05_andando_b")); if (t > 10.2f) Shot(TEXT("06_andando_lado")); }
	else if (t < 15.f) { bRunKey = true; Move(90.f); SetCam(-60.f, -8.f); if (t > 13.f) Shot(TEXT("07_correndo_a")); if (t > 13.6f) Shot(TEXT("08_correndo_b")); if (t > 14.4f) Shot(TEXT("09_correndo_frente")); }
	else if (t < 18.f) { bRunKey = true; Move(90.f + (t - 15.f) * 120.f); SetCam(-60.f, -12.f); if (t > 16.4f) Shot(TEXT("10_virando")); }
	else if (t < 22.f)
	{
		bRunKey = true; Move(90.f); SetCam(-60.f, -6.f);
		if (t > 18.2f && !bJ1) { bJ1 = true; Jump(); }
		if (t > 18.35f) Shot(TEXT("11_pulo_subindo"));
		if (t > 18.75f) Shot(TEXT("12_pulo_topo"));
		if (t > 19.2f) Shot(TEXT("13_pulo_descendo"));
		if (t > 20.2f && !bJ2) { bJ2 = true; Jump(); }
		if (t > 20.7f) Shot(TEXT("14_pulo_2"));
	}
	else if (t < 24.f) { bRunKey = false; SetCam(160.f, -10.f); if (t > 23.f) Shot(TEXT("15_final_frente")); }
	else UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	// sequência para vídeo (1 quadro a cada 0,2 s)
	if (t >= 6.5f && t < 22.f && DemoT >= NextFrame)
	{
		NextFrame = DemoT + 0.2f;
		Shot(FString::Printf(TEXT("video/f_%03d"), FrameIdx++));
	}
}
