#include "CoopElevTest.h"
#include "CoopElevator.h"
#include "CoopCharacter.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"

static const int32 GPlan[4] = { 0, 1, 0, 2 };   // ciclos: certa, errada, certa (repeticao), "fica a bordo"
static const TCHAR* GPlanName[3] = { TEXT("ROTA CERTA"), TEXT("ROTA ERRADA"), TEXT("NEON FICA A BORDO") };

void ACoopElevTest::BeginPlay()
{
	Super::BeginPlay();
	bAuto = FParse::Param(FCommandLine::Get(), TEXT("ElevAuto"));
	bSwap = FParse::Param(FCommandLine::Get(), TEXT("ElevSwap"));
	FParse::Value(FCommandLine::Get(), TEXT("ElevTag="), Tag);
	int32 Lag = 0;
	if (bAuto && FParse::Value(FCommandLine::Get(), TEXT("ElevLag="), Lag) && Lag > 0 && GEngine)
		GEngine->Exec(GetWorld(), *FString::Printf(TEXT("Net PktLag=%d"), Lag));
}

int32 ACoopElevTest::PlanType() const { return GPlan[FMath::Clamp(Cycles, 0, 3)]; }

void ACoopElevTest::SetPh(int32 P, const TCHAR* Why)
{
	const AGameStateBase* GS = GetWorld()->GetGameState();
	Log += FString::Printf(TEXT("EVT srvT=%.3f fase %d->%d (%s) ciclo=%d\n"), GS ? GS->GetServerWorldTimeSeconds() : 0.f, Ph, P, Why, Cycles);
	Ph = P; PhT = 0.f;
}

void ACoopElevTest::Save()
{
	FFileHelper::SaveStringToFile(Log + TEXT("\n==== RESULTADOS ====\n") + Results, *(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/elev/log_%s.txt"), *Tag)));
}

void ACoopElevTest::Tick(float Dt)
{
	Super::Tick(Dt);
	if (!bAuto) return;
	UWorld* W = GetWorld();
	APlayerController* PC = W->GetFirstPlayerController();
	ACoopCharacter* Me = PC ? Cast<ACoopCharacter>(PC->GetPawn()) : nullptr;
	if (!Elev.IsValid()) for (TActorIterator<ACoopElevator> It(W); It; ++It) { Elev = *It; break; }
	AGameStateBase* GS = W->GetGameState();
	if (!Me || !Elev.IsValid() || !GS) return;
	PhT += Dt; LogT += Dt; ShotT += Dt; SaveT += Dt;
	ACoopElevator* E = Elev.Get();
	const float Srv = GS->GetServerWorldTimeSeconds();
	const bool bHost = Tag == TEXT("host");
	const bool bNeon = Me->CoopRole == ECoopRole::Neon;
	const FVector Home = E->GetHome();
	const TCHAR* RoleName = bNeon ? TEXT("Neon") : TEXT("Ruina");
	// lado de cada um na cabine: do lado da porta que vai usar (Ruina +Y/roxa, Neon -Y/ciano; no ciclo "errado" invertido)
	const float OwnSide = bNeon ? -1.f : 1.f;
	const float Side = (PlanType() == 1) ? -OwnSide : OwnSide;
	const FVector2D Off(0.f, 60.f * Side);

	auto Walk = [&](FVector2D Target)->float
	{
		const FVector2D L(Me->GetActorLocation().X, Me->GetActorLocation().Y);
		const FVector2D D = Target - L;
		if (D.Size() > 20.f)
		{
			Me->AddMovementInput(FVector(D.X, D.Y, 0.f).GetSafeNormal(), 1.f);
			PC->SetControlRotation(FRotator(-12.f, FMath::RadiansToDegrees(FMath::Atan2(D.Y, D.X)), 0.f));   // camera olha para onde anda
		}
		return D.Size();
	};

	if (Ph >= 1 && LogT >= 0.25f)
	{
		LogT = 0.f;
		FString L = FString::Printf(TEXT("srvT=%.3f ph=%d elevZ=%.1f mov=%d portasLat=%.2f portao=%.2f |"), Srv, Ph, E->GetActorLocation().Z, E->IsMoving() ? 1 : 0, E->SideDoorAlpha(), E->FrontGateAlpha());
		for (TActorIterator<ACoopCharacter> It(W); It; ++It)
		{
			const FVector P = It->GetActorLocation(), R = P - E->GetActorLocation();
			UPrimitiveComponent* B = It->GetMovementBase();
			L += FString::Printf(TEXT(" [%s %s pos=(%.0f,%.0f,%.0f) rel=(%.0f,%.0f,%.0f) base=%s]"), It->CoopRole == ECoopRole::Neon ? TEXT("Neon") : TEXT("Ruina"),
				It->IsLocallyControlled() ? TEXT("local") : TEXT("remoto"), P.X, P.Y, P.Z, R.X, R.Y, R.Z, B && B->GetOwner() ? *B->GetOwner()->GetName() : TEXT("-"));
		}
		Log += L + TEXT("\n");
		if (Ph == 3)
		{
			const FVector R = Me->GetActorLocation() - E->GetActorLocation();
			MinRel = FMath::Min(MinRel, R.Z); MaxRel = FMath::Max(MaxRel, R.Z);
			if (!bHaveRef) { RefRel = R; bHaveRef = true; }
			MaxDrift = FMath::Max(MaxDrift, FVector2D(R.X - RefRel.X, R.Y - RefRel.Y).Size());
		}
	}
	if (Ph >= 1 && ShotT >= 0.25f) { ShotT = 0.f; FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/elev/%s/f_%05d.jpg"), *Tag, Shots++), false, false); }
	if (SaveT > 3.f) { SaveT = 0.f; Save(); }

	switch (Ph)
	{
	case 0:
		if (bSwap && !bSwapped && PhT > 2.f)
		{
			// usa a RPC de teste ja existente (F3) via reflexao, sem alterar o personagem
			if (UFunction* F = Me->FindFunction(TEXT("ServerSetRole"))) { struct { ECoopRole NewRole; } Parm{ bHost ? ECoopRole::Ruina : ECoopRole::Neon }; Me->ProcessEvent(F, &Parm); }
			bSwapped = true;
		}
		if (GS->PlayerArray.Num() >= 2 && PhT > 5.f) SetPh(1, TEXT("2 jogadores presentes"));
		break;
	case 1: // embarca (embaixo pelo portao -X; em cima pela porta da propria rota)
		if (Walk(FVector2D(Home.X, Home.Y) + Off) < 20.f) SetPh(2, TEXT("a bordo, aguardando partida"));
		else if (PhT > 40.f) SetPh(9, TEXT("TIMEOUT embarcando"));
		break;
	case 2:
		if (E->IsMoving()) { SetPh(3, TEXT("elevador partiu")); bHaveRef = false; }
		else if (PhT > 30.f) SetPh(9, TEXT("TIMEOUT: elevador nao partiu"));
		break;
	case 3:
		if (!E->IsMoving()) SetPh(4, TEXT("elevador chegou (parado)"));
		break;
	case 4: // espera as portas do andar abrirem
		if (E->IsTargetTop() ? E->SideDoorAlpha() > 0.98f : E->FrontGateAlpha() > 0.98f)
		{
			if (E->IsTargetTop()) Results += FString::Printf(TEXT("ciclo %d [%s] %s: portas laterais abertas %.1fs apos parar (portao entrada=%.2f)\n"), Cycles + 1, GPlanName[PlanType()], RoleName, PhT, E->FrontGateAlpha());
			if (PhT > 0.5f) SetPh(E->IsTargetTop() ? 11 : 5, E->IsTargetTop() ? TEXT("separacao: seguir rota") : TEXT("sair pelo portao"));
		}
		else if (PhT > 10.f) SetPh(9, TEXT("TIMEOUT portas nao abriram"));
		break;
	case 11: // rota no andar de separacao
	{
		const int32 T = PlanType();
		if (T == 2 && bNeon) { Results += FString::Printf(TEXT("ciclo %d [%s] Neon: ficou a bordo\n"), Cycles + 1, GPlanName[T]); SetPh(14, TEXT("fica a bordo")); break; }
		const float S = (T == 1) ? -OwnSide : OwnSide;   // T==1: tenta a porta do OUTRO
		Walk(FVector2D(Home.X, Home.Y + S * 700.f));
		const float AY = FMath::Abs(Me->GetActorLocation().Y - Home.Y);
		RouteMaxAbsY = FMath::Max(RouteMaxAbsY, AY);
		const TCHAR* Bar = (S > 0) ? TEXT("ROXA(Ruina)") : TEXT("CIANO(Neon)");
		if (T != 1 && AY >= 650.f) { Results += FString::Printf(TEXT("ciclo %d [%s] %s: ATRAVESSOU barreira %s (|Y| max %.0f) -> OK\n"), Cycles + 1, GPlanName[T], RoleName, Bar, RouteMaxAbsY); SetPh(12, TEXT("passou")); }
		else if (T != 1 && PhT > 15.f) { Results += FString::Printf(TEXT("ciclo %d [%s] %s: NAO passou barreira %s (|Y| max %.0f) -> FALHA\n"), Cycles + 1, GPlanName[T], RoleName, Bar, RouteMaxAbsY); SetPh(12, TEXT("nao passou")); }
		else if (T == 1 && PhT > 6.f)
		{
			const bool bBlocked = RouteMaxAbsY < 200.f;   // plano da barreira em |Y|=215; centro da capsula para em ~181
			Results += FString::Printf(TEXT("ciclo %d [%s] %s: barreira %s %s (|Y| max %.0f, plano da barreira 215) -> %s\n"), Cycles + 1, GPlanName[T], RoleName, Bar,
				bBlocked ? TEXT("BLOQUEOU") : TEXT("DEIXOU PASSAR"), RouteMaxAbsY, bBlocked ? TEXT("OK") : TEXT("FALHA"));
			SetPh(12, TEXT("teste de bloqueio concluido"));
		}
		break;
	}
	case 12: if (PhT > 1.5f) SetPh(13, TEXT("retornar ao elevador")); break;
	case 13:
		if (Walk(FVector2D(Home.X, Home.Y) + Off) < 20.f)
		{
			Results += FString::Printf(TEXT("ciclo %d [%s] %s: RETORNOU ao elevador (|Y| max na rota %.0f)\n"), Cycles + 1, GPlanName[PlanType()], RoleName, RouteMaxAbsY);
			RouteMaxAbsY = 0.f; SetPh(14, TEXT("de volta a bordo"));
		}
		else if (PhT > 20.f) { Results += FString::Printf(TEXT("ciclo %d %s: NAO conseguiu retornar -> FALHA\n"), Cycles + 1, RoleName); SetPh(9, TEXT("TIMEOUT retorno")); }
		break;
	case 14: // aguarda partida para descer
		if (E->IsMoving()) { Results += FString::Printf(TEXT("ciclo %d %s: elevador desceu %.1fs depois (portas laterais=%.2f ao partir)\n"), Cycles + 1, RoleName, PhT, E->SideDoorAlpha()); SetPh(3, TEXT("descendo")); bHaveRef = false; }
		else if (PhT > 40.f) { Results += FString::Printf(TEXT("ciclo %d %s: elevador NAO desceu -> FALHA\n"), Cycles + 1, RoleName); SetPh(9, TEXT("TIMEOUT descida")); }
		break;
	case 5: // embaixo: sai pelo portao (-X)
		if (Walk(FVector2D(Home.X - 420.f, Home.Y) + Off) < 20.f) SetPh(6, TEXT("fora do elevador"));
		else if (PhT > 40.f) SetPh(9, TEXT("TIMEOUT saindo"));
		break;
	case 6:
		if (PhT > 2.f)
		{
			++Cycles; Log += FString::Printf(TEXT("EVT ciclo %d completo\n"), Cycles);
			if (Cycles >= 4) SetPh(7, TEXT("4 ciclos concluidos")); else SetPh(1, TEXT("novo embarque"));
		}
		break;
	case 7:
		Results += FString::Printf(TEXT("RESUMO(%s=%s): relZ na viagem min=%.1f max=%.1f | deriva XY max=%.1f | ciclos=%d | capturas=%d\n"), *Tag, RoleName, MinRel, MaxRel, MaxDrift, Cycles, Shots);
		Save(); SetPh(8, TEXT("fim"));
		break;
	case 8:
		if (PhT > (bHost ? 14.f : 5.f)) { Save(); GEngine->Exec(W, TEXT("quit")); Ph = 10; }
		break;
	case 9:
		if (PhT < Dt + 0.001f) Results += TEXT("RESULTADO: FALHA/TIMEOUT\n");
		Save();
		if (PhT > 6.f) { GEngine->Exec(W, TEXT("quit")); Ph = 10; }
		break;
	}
}
