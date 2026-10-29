#include "CoopFase2GameMode.h"
#include "CoopGameState.h"
#include "CoopCharacter.h"
#include "CoopHUD.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"

// pontos de pouso (cm) dos 4 niveis: Z = superficie + folga; X/Y em area livre de cada setor
static const FVector GLevelSpots[4] = { FVector(300, -600, 110), FVector(500, 0, 710), FVector(500, 0, 1310), FVector(1900, 0, 1910) };

ACoopFase2GameMode::ACoopFase2GameMode()
{
	DefaultPawnClass = ACoopCharacter::StaticClass();
	HUDClass = ACoopHUD::StaticClass();
	GameStateClass = ACoopGameState::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
}

AActor* ACoopFase2GameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> S;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It) S.Add(*It);
	if (S.Num() == 0) return Super::ChoosePlayerStart_Implementation(Player);
	S.Sort([](const AActor& A, const AActor& B) { return A.GetName() < B.GetName(); });
	return S[NextStart++ % S.Num()];
}

void ACoopFase2GameMode::TeleportTo(APlayerController* PC, int32 Level)
{
	APawn* P = PC ? PC->GetPawn() : nullptr;
	if (!P || Level < 0 || Level > 3) return;
	P->SetActorLocation(GLevelSpots[Level], false, nullptr, ETeleportType::TeleportPhysics);
	if (ACharacter* C = Cast<ACharacter>(P)) C->GetCharacterMovement()->StopMovementImmediately();
}

void ACoopFase2GameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	APlayerController* PC0 = GetWorld()->GetFirstPlayerController();
	// atalhos TEMPORARIOS de inspecao (nao fazem parte do percurso futuro)
	if (PC0)
	{
		if (PC0->WasInputKeyJustPressed(EKeys::F6)) TeleportTo(PC0, 0);
		if (PC0->WasInputKeyJustPressed(EKeys::F7)) TeleportTo(PC0, 1);
		if (PC0->WasInputKeyJustPressed(EKeys::F8)) TeleportTo(PC0, 2);
		if (PC0->WasInputKeyJustPressed(EKeys::F9)) TeleportTo(PC0, 3);
	}
	// queda para fora do poco (Z < -300): volta ao nivel 0
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		if (APawn* P = It->Get() ? It->Get()->GetPawn() : nullptr)
			if (P->GetActorLocation().Z < -300.f) TeleportTo(It->Get(), 0);

	// ---- modo de teste automatico (-Fase2Shot): capturas fixas + 4 niveis com o personagem + log de colisao ----
	static const bool bShot = FParse::Param(FCommandLine::Get(), TEXT("Fase2Shot"));
	if (!bShot || !PC0 || !PC0->GetPawn()) return;
	static float T = 0.f; static int32 S = 0; static int32 N = 0; static FString Log; static ACameraActor* Cam = nullptr;
	T += DeltaSeconds;
	auto Shot = [&](const TCHAR* Name) { FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/fase2/%s.png"), Name), false, false); };
	auto View = [&](FVector L, FRotator R, float Fov = 90.f)
	{
		if (!Cam) Cam = GetWorld()->SpawnActor<ACameraActor>(L, R);
		Cam->SetActorLocationAndRotation(L, R);
		Cam->GetCameraComponent()->SetFieldOfView(Fov);
		PC0->SetViewTargetWithBlend(Cam, 0.f);
	};
	auto Probe = [&](const TCHAR* Tag)
	{
		const FVector L = PC0->GetPawn()->GetActorLocation();
		ACharacter* C = Cast<ACharacter>(PC0->GetPawn());
		Log += FString::Printf(TEXT("%s: pos=(%.0f,%.0f,%.0f) noChao=%d capsulaR=%.0f meiaAltura=%.0f\n"), Tag, L.X, L.Y, L.Z,
			C ? (int32)C->GetCharacterMovement()->IsMovingOnGround() : -1, C ? C->GetCapsuleComponent()->GetScaledCapsuleRadius() : 0.f, C ? C->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 0.f);
	};
	static const FVector CamOff[4] = { FVector(1200, 0, 450), FVector(1800, 0, 900), FVector(1800, -450, 1500), FVector(1000, 0, 2050) };
	auto SectorView = [&](int32 L)
	{
		const FVector Tg = GLevelSpots[L];
		View(CamOff[L], FRotationMatrix::MakeFromX(Tg - CamOff[L]).Rotator(), 80.f);
	};
	auto Hide = [&](bool b) { for (TActorIterator<AActor> It(GetWorld()); It; ++It) if (It->ActorHasTag(TEXT("CorteSul"))) It->SetActorHiddenInGame(b); };
	struct FStep { float T; int32 Id; };
	static const FStep Steps[] = { {3.f,0},{3.5f,1},{4.5f,2},{5.f,3},{6.f,4},{6.5f,5},{7.5f,6},{8.f,7},{9.f,8},{9.5f,9},{10.5f,10},{11.f,11},{12.f,12},{12.5f,13},{13.5f,14},{14.f,15},{15.5f,16},{16.f,17},{17.f,18},{18.f,19},{19.f,20} };
	if (S < UE_ARRAY_COUNT(Steps) && T > Steps[S].T)
	{
		switch (Steps[S].Id)
		{
		case 0: PC0->GetPawn()->SetActorLocation(FVector(300, -600, 110)); break;
		case 1: Probe(TEXT("inicio_nivel0")); View(FVector(1200, 0, 3100), FRotator(-90, 0, 0), 90); break;
		case 2: Shot(TEXT("01_vista_superior")); break;
		case 3: Hide(true); View(FVector(1200, -3300, 1100), FRotator(0, 90, 0), 80); break;
		case 4: Shot(TEXT("02_secao_lateral")); break;
		case 5: Hide(false); View(FVector(60, -850, 2100), FRotator(-33, 38, 0), 95); break;
		case 6: Shot(TEXT("03_vista_ampla")); break;
		case 7: TeleportTo(PC0, 0); SectorView(0); break;
		case 8: Probe(TEXT("nivel0")); Shot(TEXT("04_setor_A_inferior")); break;
		case 9: TeleportTo(PC0, 1); SectorView(1); break;
		case 10: Probe(TEXT("nivel600")); Shot(TEXT("05_setor_B_intermediario")); break;
		case 11: TeleportTo(PC0, 2); SectorView(2); break;
		case 12: Probe(TEXT("nivel1200")); Shot(TEXT("06_setor_C_superior")); break;
		case 13: TeleportTo(PC0, 3); SectorView(3); break;
		case 14: Probe(TEXT("nivel1800")); Shot(TEXT("07_setor_D_saida")); break;
		case 15: PC0->GetPawn()->SetActorLocation(FVector(1380, 200, 700)); break;   // solta no poco (cai sobre o placeholder do contrapeso/piso)
		case 16: Probe(TEXT("queda_no_poco(apoiado em placeholder ou piso)")); PC0->GetPawn()->SetActorLocation(FVector(1100, -200, 700)); break;
		case 17: Probe(TEXT("queda_2_no_poco(sobre elevador de carga Z=300+88)")); PC0->GetPawn()->SetActorLocation(FVector(1000, 200, 900)); break;
		case 18: Probe(TEXT("queda_3_no_poco(vao livre -> piso nivel 0 Z=88)")); FFileHelper::SaveStringToFile(Log, *(FPaths::ProjectDir() / TEXT("AutoShots/fase2/teste_fase2.txt"))); break;
		case 19: GEngine->Exec(GetWorld(), TEXT("quit")); break;
		}
		++S;
	}
}
