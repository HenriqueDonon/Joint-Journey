#include "CoopIntroCam.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ACoopIntroCam::ACoopIntroCam()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; bAlwaysRelevant = true;
	SetReplicateMovement(false);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Raiz"));
	Cam = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Cam->SetupAttachment(RootComponent);
	Cam->SetFieldOfView(80.f);
}

float ACoopIntroCam::NowSrv() const
{
	const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	return GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}
void ACoopIntroCam::ServerStart(float Lead) { if (HasAuthority()) { T0 = NowSrv() + Lead; ForceNetUpdate(); } }
bool ACoopIntroCam::IsRunning() const { return T0 >= 0.f && NowSrv() < T0 + Duration; }
bool ACoopIntroCam::IsDone() const { return T0 >= 0.f && NowSrv() >= T0 + Duration; }

void ACoopIntroCam::Tick(float Dt)
{
	Super::Tick(Dt);
	const int32 N = FMath::Min(Pos.Num(), Look.Num());
	if (N < 2) return;
	const float U = FMath::Clamp((NowSrv() - FMath::Max(T0, 0.f)) / FMath::Max(Duration, 0.1f), 0.f, 1.f);
	const float S = U * U * (3.f - 2.f * U) * (N - 1);
	const int32 I = FMath::Min((int32)S, N - 2); const float A = S - I;
	auto P = [&](int32 i) { return Pos[FMath::Clamp(i, 0, N - 1)]; };
	auto L = [&](int32 i) { return Look[FMath::Clamp(i, 0, N - 1)]; };
	const FVector Cp = FMath::CubicCRSplineInterp(P(I - 1), P(I), P(I + 1), P(I + 2), 0.f, 1.f, 2.f, 3.f, 1.f + A);
	const FVector Lp = FMath::Lerp(L(I), L(I + 1), A * A * (3.f - 2.f * A));
	Cam->SetWorldLocationAndRotation(Cp, (Lp - Cp).Rotation());
}

void ACoopIntroCam::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopIntroCam, T0);
}
