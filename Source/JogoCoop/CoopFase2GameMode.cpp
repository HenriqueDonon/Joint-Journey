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
#include "CoopConsole.h"
#include "CoopGate.h"
#include "CoopElevator.h"
#include "CoopIntroCam.h"
#include "CoopDestructible.h"
#include "GameFramework/GameStateBase.h"

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
	P->SetActorLocation(bF2X ? CheckSpot(Level, Cast<ACoopCharacter>(P)) : GLevelSpots[Level], false, nullptr, ETeleportType::TeleportPhysics);
	if (ACharacter* C = Cast<ACharacter>(P)) C->GetCharacterMovement()->StopMovementImmediately();
}

void ACoopFase2GameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	APlayerController* PC0 = GetWorld()->GetFirstPlayerController();
	if (!bF2XChecked) SetupF2X();
	// atalhos TEMPORARIOS de inspecao (nao fazem parte do percurso futuro); no layout funcional so com -F2Debug
	if (PC0 && (!bF2X || DebugAllowed()))
	{
		if (PC0->WasInputKeyJustPressed(EKeys::F6)) TeleportTo(PC0, 0);
		if (PC0->WasInputKeyJustPressed(EKeys::F7)) TeleportTo(PC0, 1);
		if (PC0->WasInputKeyJustPressed(EKeys::F8)) TeleportTo(PC0, 2);
		if (PC0->WasInputKeyJustPressed(EKeys::F9)) TeleportTo(PC0, 3);
	}
	if (bF2X)
	{
		if (PC0 && DebugAllowed() && PC0->WasInputKeyJustPressed(EKeys::F10)) DebugUnlockAll();
		TickF2X(DeltaSeconds);
	}
	// queda para fora do poco (Z < -300): volta ao nivel 0
	if (!bF2X) for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
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

// =================== layout funcional da Fase 2 (mapa experimental Fase2_Funcional) ===================
void ACoopFase2GameMode::SetupF2X()
{
	bF2XChecked = true;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(TEXT("F2X_Layout"))) bF2X = true;
		for (const FName& T : It->Tags) if (T.ToString().StartsWith(TEXT("F2X_Check_"))) Spots.Add(T, It->GetActorLocation());
	}
	for (TActorIterator<ACoopIntroCam> It(GetWorld()); It; ++It) { Intro = *It; break; }
	if (bF2X) UE_LOG(LogTemp, Warning, TEXT("F2X layout funcional ativo: %d pontos de retorno, intro=%d"), Spots.Num(), Intro ? 1 : 0);
	if (bF2X) SetupP1();
}

bool ACoopFase2GameMode::DebugAllowed() { static const bool b = FParse::Param(FCommandLine::Get(), TEXT("F2Debug")); return b; }

// P1: os objetos nascem no servidor a partir de marcadores do mapa (TargetPoint com tag; escala * 100 = tamanho em cm)
// e replicam para os clientes. Trava: so a Ruina destroi (nunca se reconstroi). Suporte: nasce quebrado; so o Neon reconstroi,
// e so com o material da trava (Partner destruido); depois de pronto nao pode ser destruido -> sem consumo duplicado.
void ACoopFase2GameMode::SetupP1()
{
	AActor *MT = nullptr, *MS = nullptr;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It) { if (It->ActorHasTag(TEXT("F2P1_Trava"))) MT = *It; if (It->ActorHasTag(TEXT("F2P1_Suporte"))) MS = *It; }
	if (!MT || !MS) return;
	auto Spawn = [&](AActor* M, FLinearColor Col, bool bStartBroken, const TCHAR* Name) -> ACoopDestructible*
	{
		const FTransform T(M->GetActorRotation(), M->GetActorLocation());
		ACoopDestructible* D = GetWorld()->SpawnActorDeferred<ACoopDestructible>(ACoopDestructible::StaticClass(), T);
		if (!D) return nullptr;
		D->Setup(M->GetActorScale3D() * 100.f, Col, 0.f, 0.f, bStartBroken);
		D->Tags.Add(FName(Name));
		return D;
	};
	P1Trava = Spawn(MT, FLinearColor(0.45f, 0.07f, 0.04f), false, TEXT("P1_Trava"));
	P1Suporte = Spawn(MS, FLinearColor(0.2f, 0.32f, 0.55f), true, TEXT("P1_Suporte"));
	if (!P1Trava || !P1Suporte) return;
	P1Trava->bCanRebuild = false;                 // Neon nao "desfaz" a trava
	P1Suporte->bCanBreak = false;                 // Ruina nao destroi o suporte pronto
	P1Suporte->Partner = P1Trava;                 // material: so reconstroi com a trava destruida
	P1Suporte->bAssemblyFx = true;                // montagem vertical com os destrocos da trava
	P1Trava->FinishSpawning(FTransform(MT->GetActorRotation(), MT->GetActorLocation()));
	P1Suporte->FinishSpawning(FTransform(MS->GetActorRotation(), MS->GetActorLocation()));
	UE_LOG(LogTemp, Warning, TEXT("F2X P1 montado: trava e suporte criados (elevador travado)"));
}

void ACoopFase2GameMode::TickP1()
{
	if (bP1Done || !P1Suporte) return;
	if (!bP1MatMsg && P1Trava && P1Trava->IsDestroyed())
	{
		bP1MatMsg = true;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			if (ACoopCharacter* C = It->Get() ? Cast<ACoopCharacter>(It->Get()->GetPawn()) : nullptr) C->ClientToast(TEXT("Trava destruida: material liberado - o elevador ainda precisa do suporte"));
		UE_LOG(LogTemp, Warning, TEXT("F2X P1 trava destruida: material disponivel"));
	}
	// elevador liberado SO quando a reconstrucao do suporte terminou (montagem visual inclusa); validado no servidor
	if (P1Suporte->IsDestroyed() || !P1Suporte->HasBeenRebuilt()) return;
	const AGameStateBase* GS = GetWorld()->GetGameState();
	const float Now = GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
	if (Now < P1Suporte->RebuildDoneAt()) return;
	bP1Done = true;
	for (TActorIterator<ACoopElevator> It(GetWorld()); It; ++It) It->ServerSetLocked(false);
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		if (ACoopCharacter* C = It->Get() ? Cast<ACoopCharacter>(It->Get()->GetPawn()) : nullptr) C->ClientToast(TEXT("MECANISMO CONSERTADO: elevador LIBERADO"));
	UE_LOG(LogTemp, Warning, TEXT("F2X P1 CONCLUIDO: suporte reconstruido, elevador liberado"));
}

FVector ACoopFase2GameMode::CheckSpot(int32 Level, const ACoopCharacter* C) const
{
	static const TCHAR* L[4] = { TEXT("A"), TEXT("B"), TEXT("C"), TEXT("D") };
	Level = FMath::Clamp(Level, 0, 3);
	if (C)
	{
		const FName RoleTag(*FString::Printf(TEXT("F2X_Check_%s_%s"), L[Level], C->CoopRole == ECoopRole::Ruina ? TEXT("Ruina") : TEXT("Neon")));
		if (const FVector* V = Spots.Find(RoleTag)) return *V;
	}
	if (const FVector* V = Spots.Find(FName(*FString::Printf(TEXT("F2X_Check_%s"), L[Level])))) return *V;
	return GLevelSpots[Level];
}

void ACoopFase2GameMode::DebugUnlockAll()
{
	bDebugUnlocked = true;
	for (TActorIterator<ACoopGate> It(GetWorld()); It; ++It) It->ServerSetOpen(true);
	for (TActorIterator<ACoopElevator> It(GetWorld()); It; ++It) It->ServerSetLocked(false);
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		if (ACoopCharacter* C = It->Get() ? Cast<ACoopCharacter>(It->Get()->GetPawn()) : nullptr)
			C->ClientToast(TEXT("[DEPURACAO F10] portoes abertos e trava do elevador ignorada - NAO e solucao de puzzle"));
	UE_LOG(LogTemp, Warning, TEXT("F2X DEPURACAO: portoes abertos e elevador destravado"));
}

void ACoopFase2GameMode::EndIntroFor(APlayerController* PC)
{
	if (!PC || !InIntro.Contains(PC)) return;
	InIntro.Remove(PC);
	PC->ClientIgnoreMoveInput(false);
	if (PC->GetPawn()) PC->SetViewTargetWithBlend(PC->GetPawn(), 0.35f, VTBlend_EaseInOut, 2.f);
}

void ACoopFase2GameMode::HandleInteract(ACoopCharacter* C)
{
	APlayerController* PC = C ? Cast<APlayerController>(C->GetController()) : nullptr;
	if (!PC) return;
	if (InIntro.Contains(PC)) { EndIntroFor(PC); C->ClientToast(TEXT("Apresentacao pulada")); return; }
	ACoopConsole* Best = nullptr; float BestD = 1e9f;
	for (TActorIterator<ACoopConsole> It(GetWorld()); It; ++It)
	{
		const float D = FVector::Dist(It->GetActorLocation(), C->GetActorLocation());
		if (D < It->UseRadius && D < BestD) { BestD = D; Best = *It; }
	}
	if (!Best) { C->ClientToast(TEXT("Nada para usar aqui")); return; }
	FString Why;
	if (Best->ServerUse(C, Why)) C->ClientToast(TEXT("Elevador: comando aceito"));
	else C->ClientToast(Why);
	UE_LOG(LogTemp, Warning, TEXT("F2X console %s por %s: %s"), *Best->GetName(), *C->GetName(), Why.IsEmpty() ? TEXT("aceito") : *Why);
}

void ACoopFase2GameMode::TickF2X(float Dt)
{
	F2XTime += Dt;
	TickP1();
	// apresentacao: uma vez por sessao, quando os dois jogadores estao presentes (ou 30 s jogando sozinho)
	const int32 NP = GetWorld()->GetNumPlayerControllers();
	static const bool bNoIntro = FParse::Param(FCommandLine::Get(), TEXT("F2SemIntro"));
	static const bool bIntroSolo = FParse::Param(FCommandLine::Get(), TEXT("F2Intro"));   // teste: grava a apresentacao com 1 jogador
	if (Intro && !bIntroStarted && !bNoIntro && F2XTime > 2.f && (NP >= 2 || F2XTime > (bIntroSolo ? 3.f : 30.f)))
	{
		bIntroStarted = true;
		Intro->ServerStart(0.3f);
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			if (APlayerController* PC = It->Get()) { InIntro.Add(PC); PC->ClientIgnoreMoveInput(true); PC->SetViewTargetWithBlend(Intro, 0.3f); }
		UE_LOG(LogTemp, Warning, TEXT("F2X apresentacao iniciada (%d jogadores)"), NP);
	}
	if (Intro && bIntroStarted && Intro->IsDone() && InIntro.Num() > 0)
	{
		TArray<APlayerController*> L = InIntro;
		for (APlayerController* PC : L) EndIntroFor(PC);
		UE_LOG(LogTemp, Warning, TEXT("F2X apresentacao terminou"));
	}
	// recuperacao de queda: queda livre de mais de 3,5 m -> volta ao ponto de retorno do nivel de onde caiu
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		ACharacter* Ch = PC ? Cast<ACharacter>(PC->GetPawn()) : nullptr;
		if (!Ch) continue;
		const float Z = Ch->GetActorLocation().Z;
		const bool bFalling = Ch->GetCharacterMovement()->IsFalling();
		int32& ML = MaxLevel.FindOrAdd(Ch);
		if (!bFalling)
		{
			const int32 Lv = FMath::Clamp(FMath::RoundToInt((Z - 90.f) / 600.f), 0, 3);
			ML = FMath::Max(ML, Lv);
			if (float* S = FallStart.Find(Ch))
			{
				if (*S - Z > 350.f)
				{
					const int32 From = FMath::Clamp(FMath::RoundToInt((*S - 90.f) / 600.f), 0, 3);
					TeleportTo(PC, From);
					if (ACoopCharacter* CC = Cast<ACoopCharacter>(Ch)) CC->ClientToast(TEXT("Queda: voce voltou ao ultimo ponto seguro"));
					UE_LOG(LogTemp, Warning, TEXT("F2X queda de %.0f cm -> retorno ao nivel %d"), *S - Z, From);
				}
				FallStart.Remove(Ch);
			}
		}
		else if (!FallStart.Contains(Ch)) FallStart.Add(Ch, Z);
		else FallStart[Ch] = FMath::Max(FallStart[Ch], Z);   // pulo: conta do ponto mais alto
		if (Z < -400.f) { TeleportTo(PC, ML); FallStart.Remove(Ch); }
	}
}
