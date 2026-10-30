#include "CoopFase1GameMode.h"
#include "CoopGameState.h"
#include "CoopCharacter.h"
#include "CoopHUD.h"
#include "CoopBlock.h"
#include "CoopSun.h"
#include "CoopDestructible.h"
#include "CoopCeiling.h"
#include "CoopLamp.h"
#include "CoopFog.h"
#include "CoopEasterEgg.h"
#include "CoopExitDoor.h"
#include "CoopCollapseRig.h"
#include "Misc/PackageName.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "EngineUtils.h"
#include "UnrealClient.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Misc/FileHelper.h"
#include "Engine/StaticMesh.h"

// Escala da sala: X/Y x1.5, altura x1.1 (degrau da saida 40*1.1=44 < MaxStepHeight 45). Tempo e alcance dos poderes escalam junto.
static constexpr float KXY = 1.5f, KZ = 1.1f, KLUM = 2.25f;
static constexpr float CollapseSecs = 12.f; // 8s * 1.5
static FVector Sc(const FVector& V) { return FVector(V.X * KXY, V.Y * KXY, V.Z * KZ); }

// Mapa editavel (Fase1_Editavel): a decoracao com modelo ja esta no nivel como atores (tag F1Decor) -> o codigo nao a cria de novo.
// Atores F1Ref sao so referencia para o Editor (plataformas, paredes, teto...) e sao removidos ao jogar.
static bool GBaked = false;
static bool HasModel(const TCHAR* Slot)
{
	static TMap<FString, bool> Cache;
	if (const bool* B = Cache.Find(Slot)) return *B;
	const FString P = FString::Printf(TEXT("/Game/Fase1/%s/SM_%s.SM_%s"), Slot, Slot, Slot);
	const bool bHas = LoadObject<UStaticMesh>(nullptr, *P, nullptr, LOAD_NoWarn | LOAD_Quiet) != nullptr;
	Cache.Add(Slot, bHas);
	return bHas;
}
static void Lbl(AActor* A, const TCHAR* N)
{
#if WITH_EDITOR
	if (A) A->SetActorLabel(N);
#endif
}

ACoopFase1GameMode::ACoopFase1GameMode()
{
	DefaultPawnClass = ACoopCharacter::StaticClass();
	HUDClass = ACoopHUD::StaticClass();
	GameStateClass = ACoopGameState::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
}

// Teto desabando esmaga quem estiver embaixo (checado todo frame no servidor, sem atravessar o boneco)
static constexpr float RoofBottom = 568.f, RoofDrop = 545.f, RoofFallSecs = 0.8f;
static void DumpLayoutFile(UWorld* W)
{
	FString J = TEXT("[\n"); int32 N = 0;
	for (TActorIterator<AActor> It(W); It; ++It)
	{
		AActor* A = *It;
		if (A->IsA<APawn>() || A->IsA<ACameraActor>() || A->IsA<ACoopSun>() || A->IsHidden()) continue;
		const bool bDecor = A->Tags.Contains(TEXT("F1Decor")), bCol = A->Tags.Contains(TEXT("F1Col"));
		const ACoopBlock* Bk = Cast<ACoopBlock>(A);
		const FString Slot = (Bk && Bk->Model != NAME_None) ? Bk->Model.ToString() : A->GetClass()->GetName().Replace(TEXT("Coop"), TEXT(""));
		TArray<UStaticMeshComponent*> Comps; A->GetComponents(Comps);
		for (UStaticMeshComponent* C : Comps)
		{
			if (!C || !C->GetStaticMesh() || !C->IsVisible()) continue;
			const bool bEdit = bDecor && Bk && C == Bk->ModelComp;
			const FString Folder = bEdit ? FString::Printf(TEXT("Decor/%s"), *Slot) : (A->IsA<ACoopCeiling>() ? FString(TEXT("Referencia/Teto")) : FString::Printf(TEXT("Referencia/%s"), *Slot));
			const FVector L = C->GetComponentLocation(), S = C->GetComponentScale(); const FRotator R = C->GetComponentRotation();
			J += FString::Printf(TEXT("%s{\"n\":\"%s\",\"f\":\"%s\",\"m\":\"%s\",\"d\":%d,\"c\":%d,\"l\":[%.2f,%.2f,%.2f],\"r\":[%.3f,%.3f,%.3f],\"s\":[%.4f,%.4f,%.4f]}\n"),
				N++ ? TEXT(",") : TEXT(""), *Slot, *Folder, *C->GetStaticMesh()->GetPathName(), bEdit ? 1 : 0, (bEdit && bCol) ? 1 : 0, L.X, L.Y, L.Z, R.Pitch, R.Yaw, R.Roll, S.X, S.Y, S.Z);
		}
	}
	J += TEXT("]\n");
	FFileHelper::SaveStringToFile(J, *(FPaths::ProjectDir() / TEXT("MapaFase1/layout.json")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	UE_LOG(LogTemp, Warning, TEXT("DUMPLAYOUT %d"), N);
}

void ACoopFase1GameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	{
		static const bool bDump = FParse::Param(FCommandLine::Get(), TEXT("DumpLayout"));
		static float DT = 0.f; static bool bDone = false;
		if (bDump && !bDone && bBuilt && (DT += DeltaSeconds) > 4.f) { bDone = true; DumpLayoutFile(GetWorld()); GEngine->Exec(GetWorld(), TEXT("quit")); }
	}
	// video da coluna (so com -ColunaVideo): 1 captura por frame; quebra em 2s, reconstroi em 6.5s, termina em 9.5s
	{
		static const bool bVid = FParse::Param(FCommandLine::Get(), TEXT("ColunaVideo"));
		static float VT = 0.f; static int32 VN = 0; static int32 VS = 0; static bool bBroke = false, bRebuilt = false; static ACameraActor* VCam = nullptr;
		if (bVid && bBuilt)
		{
			VT += DeltaSeconds;
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (VS == 0 && VT > 1.f)
			{
				if (PC && PC->GetPawn()) PC->GetPawn()->SetActorLocation(FVector(-1500.f, 300.f, 110.f));
				VCam = GetWorld()->SpawnActor<ACameraActor>(FVector(520, 330, 360), FRotator(-9.f, -135.f, 0.f));
				if (PC && VCam) PC->SetViewTargetWithBlend(VCam, 0.f);
				VS = 1; VT = 0.f;
			}
			else if (VS == 1)
			{
				if (VT > 2.f && !bBroke && Column) { Column->ServerForce(true); bBroke = true; }
				FString Why;
				if (VT > 6.5f && !bRebuilt && Column) { Column->ServerRebuild(Why); bRebuilt = true; }
				if (VT > 1.5f && VT < 9.5f) FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/video/f_%04d.png"), VN++), false, false);
				if (VT > 9.5f) { GEngine->Exec(GetWorld(), TEXT("quit")); VS = 2; }
			}
		}
	}
	// teste da quebra da coluna (so com -ColunaShot): camera fixa, quebra a coluna e tira capturas em sequencia
	{
		static const bool bCol = FParse::Param(FCommandLine::Get(), TEXT("ColunaShot"));
		static float CT = 0.f; static int32 CS = 0; static int32 CN = 0; static ACameraActor* CCam = nullptr;
		if (bCol && bBuilt)
		{
			CT += DeltaSeconds;
			static const float Ts[] = { 5.f, 7.f, 8.f, 8.12f, 8.4f, 8.8f, 9.5f, 11.f, 14.f, 16.f };
			if (CS < 10 && CT > Ts[CS])
			{
				APlayerController* PC = GetWorld()->GetFirstPlayerController();
				switch (CS)
				{
				case 0:
					if (PC && PC->GetPawn()) PC->GetPawn()->SetActorLocation(FVector(-1500.f, 300.f, 110.f));
					CCam = GetWorld()->SpawnActor<ACameraActor>(FVector(520, 330, 360), FRotator(-9.f, -135.f, 0.f));
					if (PC && CCam) PC->SetViewTargetWithBlend(CCam, 0.f);
					break;
				case 2: if (Column) Column->ServerForce(true); break;
				case 9: GEngine->Exec(GetWorld(), TEXT("quit")); break;
				default: FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/coluna_%d.png"), ++CN), false, false); break;
				}
				++CS;
			}
		}
	}
	// modo de captura automatica (so com -AutoShot): vistas fixas de camera (antes/depois) + vista superior sem teto
	{
		static const bool bAuto = FParse::Param(FCommandLine::Get(), TEXT("AutoShot"));
		static float AT = 0.f; static int32 AS = 0; static int32 SN = 0;
		static ACameraActor* Cam = nullptr;
		if (bAuto)
		{
			AT += DeltaSeconds;
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			APawn* Pw = PC ? PC->GetPawn() : nullptr;
			struct FV { FVector L; FRotator R; int32 Hide; };
			static const FV Views[] = {
				{ FVector(-2050, 0, 480), FRotator(-14, 0, 0), 0 },    // 1 geral: do inicio para a saida
				{ FVector(3500, 0, 480), FRotator(-14, 180, 0), 0 },   // 2 geral: da saida para o inicio
				{ FVector(750, -700, 420), FRotator(-12, 90, 0), 0 },  // 3 lateral: parede Y+ e vao
				{ FVector(750, 700, 420), FRotator(-12, -90, 0), 0 },  // 4 lateral: parede Y-
				{ FVector(3150, 0, 200), FRotator(-4, 0, 0), 0 },      // 5 porta aberta e tunel
				{ FVector(3700, 0, 220), FRotator(-6, 0, 0), 0 },      // 6 dentro do tunel
				{ FVector(-800, 0, 560), FRotator(-35, 0, 0), 0 },     // 7 alto: ponte, vigas e canos
				{ FVector(1640, 0, 4700), FRotator(-90, -90, 0), 1 },  // 8 superior sem forro (canos/vigas visiveis)
				{ FVector(1640, 0, 4700), FRotator(-90, -90, 0), 2 },  // 9 superior: so o piso
			};
			const int32 NV = UE_ARRAY_COUNT(Views);
			if (AS == 0 && AT > 5.f) { if (Pw) Pw->SetActorLocation(FVector(3300.f, 520.f, 110.f)); AS = 1; } // perto da porta: abre
			else if (AS >= 1 && AS <= NV * 2 && AT > 7.f + AS * 1.5f)
			{
				const int32 i = (AS - 1) / 2;
				if (((AS - 1) & 1) == 0)
				{
					if (!Cam) Cam = GetWorld()->SpawnActor<ACameraActor>(Views[i].L, Views[i].R);
					if (Cam) Cam->SetActorLocationAndRotation(Views[i].L, Views[i].R);
					if (Views[i].Hide)
						for (TActorIterator<AActor> It(GetWorld()); It; ++It)
						{
							if (*It == Cam) continue;
							FVector O, E; It->GetActorBounds(false, O, E);
							if (E.IsNearlyZero()) continue;
							const float MinZ = O.Z - E.Z;
							const bool bTun = O.X > 3640.f;
							if ((bTun && MinZ >= 395.f) || (!bTun && MinZ >= (Views[i].Hide == 1 ? 530.f : 380.f))) It->SetActorHiddenInGame(true);
						}
					if (PC && Cam) PC->SetViewTargetWithBlend(Cam, 0.f);
				}
				else FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir() / FString::Printf(TEXT("AutoShots/shot_%d.png"), ++SN), false, false);
				++AS;
			}
			else if (AS == NV * 2 + 1 && AT > 7.f + AS * 1.5f + 3.f) { GEngine->Exec(GetWorld(), TEXT("quit")); ++AS; }
		}
	}
	if (Phase != 2 || Rig) return;   // com rig (layout 2) o impacto e tratado em TickCollapse
	const float A = FMath::Clamp((GetWorld()->GetTimeSeconds() - PhaseT0) / RoofFallSecs, 0.f, 1.f);
	const float Bottom = RoofBottom - RoofDrop * A * A;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ACoopCharacter* C = It->Get() ? Cast<ACoopCharacter>(It->Get()->GetPawn()) : nullptr;
		if (!C || C->bCrushed) continue;
		const float Top = C->GetActorLocation().Z + C->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		if (C->GetActorLocation().Z > -120.f && Bottom <= Top) C->SetCrushed(true);
	}
}

AActor* ACoopFase1GameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!bBuilt)
	{
		bBuilt = true;
		BuildLevel();
		GetWorldTimerManager().SetTimer(FallTimer, this, &ACoopFase1GameMode::CheckFall, 0.1f, true);
	}
	if (Starts.Num() == 0) return Super::ChoosePlayerStart_Implementation(Player);
	return Starts[NextStart++ % Starts.Num()];
}

void ACoopFase1GameMode::BuildExitDoor(float FX, float FY, float FZ)
{
	UWorld* W = GetWorld();
	// F. saida: moldura (modelo, so visual) + duas folhas que DESLIZAM para os lados quando todos estao perto (ver TickExit)
	{
		ExitCenter = FVector(FX, FY, FZ + 100.f);
		if (ACoopBlock* Fr = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), FVector(FX, FY, FZ + 143.f), FRotator::ZeroRotator))
		{ Fr->Setup(FVector(56.6f, 280.4f, 286.f), FLinearColor(0.1f, 0.1f, 0.12f)); Fr->SetOpts(TEXT("Porta_Quadro"), true); Lbl(Fr, TEXT("F1_PortaQuadro")); }
		// os pilares da moldura avancam para dentro da sala: colisao invisivel para nao atravessar
		for (const FVector2D& P : { FVector2D(-104.5f, 71.f), FVector2D(98.5f, 83.f) })
			if (ACoopBlock* G = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), FVector(FX, FY + P.X, FZ + 143.f), FRotator::ZeroRotator))
			{ G->Setup(FVector(57.f, P.Y, 286.f), FLinearColor::Black, true); Lbl(G, TEXT("F1_PortaPilar_Colisao")); }
		auto Leaf = [&](const TCHAR* Slot, FVector Cen, FVector Sz, float Dir) -> ACoopExitDoor*
		{
			ACoopExitDoor* L = W->SpawnActor<ACoopExitDoor>(ACoopExitDoor::StaticClass(), Cen, FRotator::ZeroRotator);
			if (L) { L->Setup(Sz, FLinearColor(0.12f, 0.13f, 0.15f), false, FLinearColor::Black, 0.6f, 0.5f); L->SetOpts(FName(Slot), false); L->SetSlide(FVector(0, Dir, 0), 66.f); Lbl(L, Slot); }
			return L;
		};
		ExitDoor = Leaf(TEXT("Porta_Folha_Esq"), FVector(FX + 0.5f, FY - 36.4f, FZ + 95.f), FVector(54.2f, 60.7f, 190.f), -1.f);
		ExitDoor2 = Leaf(TEXT("Porta_Folha_Dir"), FVector(FX + 1.4f, FY + 24.3f, FZ + 95.f), FVector(50.3f, 60.6f, 190.f), 1.f);
	}
}

// ================= LAYOUT 2 (arquitetura importada; posicoes vindas de marcadores no mapa) =================
// Marcadores (qualquer ator; normalmente TargetPoint) com tag:
//  F1L_Coluna (base da coluna) | F1L_Ponte (centro do topo; escala X*100 = comprimento) | F1L_Porta (piso no centro da moldura)
//  F1L_Inicio (1..3 pontos de inicio) | F1L_Barreira (centro; escala*100 = tamanho) | F1L_Teto (centro da face de baixo do
//  teto que desaba; escala X/Y*100 = area) | F1L_Rachaduras (centro na parede; escala X/Z*100 = area; Y = plano da parede)
bool ACoopFase1GameMode::BuildLevelLayout2()
{
	UWorld* W = GetWorld();
	TMap<FName, TArray<AActor*>> Mk;
	for (TActorIterator<AActor> It(W); It; ++It)
		for (const FName& T : It->Tags) if (T.ToString().StartsWith(TEXT("F1L_"))) Mk.FindOrAdd(T).Add(*It);
	auto One = [&](const TCHAR* N) -> AActor* { const TArray<AActor*>* A = Mk.Find(FName(N)); return (A && A->Num()) ? (*A)[0] : nullptr; };
	AActor *MCol = One(TEXT("F1L_Coluna")), *MBr = One(TEXT("F1L_Ponte")), *MDoor = One(TEXT("F1L_Porta"));
	if (!MCol || !MBr || !MDoor) return false;
	bLayout2 = true;
	UE_LOG(LogTemp, Warning, TEXT("F1MAPA layout2 (marcadores) ativo"));
	W->SpawnActor<ACoopSun>(ACoopSun::StaticClass(), FVector(0, 0, 1000), FRotator::ZeroRotator);
	// coluna (mesmo tamanho/modelo do original) e ponte (comprimento pelo marcador), material compartilhado
	{
		const FVector C = MCol->GetActorLocation() + FVector(0, 0, 308.f);
		if (ACoopDestructible* Col = W->SpawnActorDeferred<ACoopDestructible>(ACoopDestructible::StaticClass(), FTransform(C)))
		{ Col->Setup(Sc(FVector(160, 160, 560)), FLinearColor(0.62f, 0.62f, 0.64f)); Col->Model = TEXT("Coluna_Inteira"); Col->bAssemblyFx = true; Col->FinishSpawning(FTransform(C)); Lbl(Col, TEXT("F1_Coluna")); Column = Col; }
		const float Len = FMath::Max(MBr->GetActorScale3D().X * 100.f, 300.f);
		const FVector B = MBr->GetActorLocation() - FVector(0, 0, 16.5f);
		if (ACoopDestructible* Br = W->SpawnActorDeferred<ACoopDestructible>(ACoopDestructible::StaticClass(), FTransform(B)))
		{ Br->Setup(FVector(Len, 375.f, 33.f), FLinearColor(0.55f, 0.52f, 0.46f), 360.f * KZ, 350.f * KXY, true); Br->Model = TEXT("Ponte_Inteira"); Br->bAssemblyFx = true; Br->FinishSpawning(FTransform(B)); Lbl(Br, TEXT("F1_Ponte")); Bridge = Br; }
		if (Column && Bridge) { Column->Partner = Bridge; Bridge->Partner = Column; }
	}
	for (TActorIterator<ACoopCollapseRig> It(W); It; ++It) { Rig = *It; break; }
	// teto estrutural que desaba (apoiado na coluna): paineis do Blender, mesma queda/esmagamento do original (sem rig no mapa)
	if (AActor* MT = Rig ? nullptr : One(TEXT("F1L_Teto")))
	{
		const FVector O = MT->GetActorLocation(), S = MT->GetActorScale3D() * 100.f;
		const int32 NX = FMath::Max(1, FMath::RoundToInt(S.X / 583.5f)), NY = FMath::Max(1, FMath::RoundToInt(S.Y / 540.f));
		for (int32 ix = 0; ix < NX; ++ix) for (int32 iy = 0; iy < NY; ++iy)
			if (ACoopCeiling* c = W->SpawnActor<ACoopCeiling>(ACoopCeiling::StaticClass(), O + FVector(-S.X * 0.5f + (ix + 0.5f) * S.X / NX, -S.Y * 0.5f + (iy + 0.5f) * S.Y / NY, 40.f), FRotator::ZeroRotator))
			{ c->Setup(FVector(S.X / NX, S.Y / NY, 80.f), FLinearColor(0.1f, 0.11f, 0.12f)); c->SetOpts(TEXT("Teto_Painel"), true); Lbl(c, TEXT("F1_TetoEstrutural_L2")); Roof.Add(c); }
	}
	if (AActor* MR = One(TEXT("F1L_Rachaduras"))) { const FVector O = MR->GetActorLocation(), E = MR->GetActorScale3D() * 50.f; CrackBox = FBox(O - E, O + E); CrackWallY = O.Y; }
	// porta + tunel: mesma construcao do original, deslocada para o marcador (moldura em FX=3576, piso 44 no original)
	const FVector D = MDoor->GetActorLocation();
	L2Off = FVector(D.X - 3576.f, D.Y, D.Z - 44.f);
	BuildExitDoor(D.X, D.Y, D.Z);
	auto WallPiece = [&](const TCHAR* N, FVector C, FVector Sz) { if (ACoopBlock* b = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), C + L2Off, FRotator::ZeroRotator)) { b->Setup(Sz, FLinearColor(0.20f, 0.21f, 0.24f)); Lbl(b, N); } };
	WallPiece(TEXT("F1_ParedeFundoSaida_Esq"), FVector(3630, -439.5f, 55), FVector(60, 741, 1430));
	WallPiece(TEXT("F1_ParedeFundoSaida_Dir"), FVector(3630, 433.5f, 55), FVector(60, 753, 1430));
	WallPiece(TEXT("F1_ParedeFundoSaida_Topo"), FVector(3630, 0, 503), FVector(60, 1620, 534));
	{
		TSet<AActor*> Before; for (TActorIterator<AActor> It(W); It; ++It) Before.Add(*It);
		const bool bOld = GBaked; GBaked = false;   // o tunel completo (com seus modelos) e criado pelo codigo
		BuildTransition();
		GBaked = bOld;
		for (TActorIterator<AActor> It(W); It; ++It) if (!Before.Contains(*It)) It->AddActorWorldOffset(L2Off);
	}
	if (AActor* MB = One(TEXT("F1L_Barreira"))) { BarrierLoc = MB->GetActorLocation(); BarrierSize = MB->GetActorScale3D() * 100.f; }
	SpawnBarrier();
	W->SpawnActor<ACoopFog>(ACoopFog::StaticClass(), MBr->GetActorLocation() + FVector(0, 0, 200), FRotator::ZeroRotator);
	if (const TArray<AActor*>* St = Mk.Find(FName(TEXT("F1L_Inicio"))))
		for (AActor* A : *St) Starts.Add(W->SpawnActor<APlayerStart>(A->GetActorLocation(), A->GetActorRotation()));
	return true;
}

void ACoopFase1GameMode::SpawnBarrier()
{
	// Barreira INVISIVEL provisoria no vao (ver relatorio): o vao seria pulavel (pulo corrido ~700-800cm).
	// Ignora Visibility/Camera (ghost), entao nao atrapalha mira nem linha de visao dos poderes.
	UWorld* W = GetWorld();
	ACoopBlock* B = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), BarrierLoc, FRotator::ZeroRotator);
	if (B) { B->Setup(BarrierSize, FLinearColor::Black, true); Lbl(B, TEXT("F1_BarreiraVao_Provisoria")); }
	Barrier = B;
}

void ACoopFase1GameMode::BuildLevel()
{
	UWorld* W = GetWorld();
	{
		GBaked = false;
		TArray<AActor*> Refs;
		for (TActorIterator<AActor> It(W); It; ++It)
		{
			if (It->Tags.Contains(TEXT("F1Decor"))) GBaked = true;
			if (It->Tags.Contains(TEXT("F1Ref"))) Refs.Add(*It);
		}
		for (AActor* A : Refs) A->Destroy();
		UE_LOG(LogTemp, Warning, TEXT("F1MAPA editavel=%d referencias_removidas=%d"), (int)GBaked, Refs.Num());
	}
	if (BuildLevelLayout2()) return;   // mapa com marcadores F1L_* (nova arquitetura); senao, layout original abaixo
	W->SpawnActor<ACoopSun>(ACoopSun::StaticClass(), FVector(0, 0, 1000), FRotator::ZeroRotator);

	auto B = [&](const TCHAR* Name, FVector C, FVector S, FLinearColor Col, bool bGhost = false, FLinearColor Em = FLinearColor::Black, float Rough = 0.9f, float Metal = 0.f) -> ACoopBlock*
	{
		ACoopBlock* b = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), Sc(C), FRotator::ZeroRotator);
		if (b) { b->Setup(Sc(S), Col, bGhost, Em, Rough, Metal); Lbl(b, Name); }
		return b;
	};
	auto M = [&](ACoopBlock* b, const TCHAR* Slot) { if (b) b->SetOpts(FName(Slot), false); };
	const FLinearColor Floor(0.30f, 0.31f, 0.34f), Floor2(0.42f, 0.43f, 0.46f), Wall(0.20f, 0.21f, 0.24f);

	// A. plataforma inicial: X -1400..0, topo Z=0
	M(B(TEXT("F1_PlataformaInicial"), FVector(-700, 0, -300), FVector(1400, 1000, 600), Floor), TEXT("Plataforma_Inicial"));
	// E. plataforma de saida: X 1000..2400, topo Z=+40 (elevada 40cm; degrau < MaxStepHeight 45)
	M(B(TEXT("F1_PlataformaSaida"), FVector(1700, 0, -280), FVector(1400, 1000, 640), Floor2), TEXT("Plataforma_Saida"));
	// C. vao central X 0..1000 (10m), fundo de lava com topo em Z=-350 (queda 3,5m)
	B(TEXT("F1_Lava"), FVector(500, 0, -400), FVector(1000, 1000, 100), FLinearColor(0.25f, 0.04f, 0.01f), false, FLinearColor(1.0f, 0.28f, 0.03f) * 2.5f, 0.6f);
	// superficie de lava (modelo do Blender, face brilhante para cima), fina e maior que o vao para nao mostrar bordas
	if (!GBaked) if (ACoopBlock* Lv = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), FVector(750, 0, -386 + 11), FRotator::ZeroRotator))
	{ Lv->Setup(FVector(1500, 1500, 22), FLinearColor::Black); Lv->SetOpts(TEXT("Lava"), true); Lv->SetModelScale(FVector(1.25f, 1.25f, 0.2f)); Lbl(Lv, TEXT("F1_LavaSuperficie")); Lv->Tags.Add(TEXT("F1Decor")); }
	// G. paredes/teto
	M(B(TEXT("F1_ParedeLateralEsq"), FVector(500, -520, 50), FVector(3850, 40, 1300), Wall), TEXT("Parede_Lateral_Esq"));
	M(B(TEXT("F1_ParedeLateralDir"), FVector(500, 520, 50), FVector(3850, 40, 1300), Wall), TEXT("Parede_Lateral_Dir"));
	M(B(TEXT("F1_ParedeFundoInicio"), FVector(-1420, 0, 50), FVector(40, 1080, 1300), Wall), TEXT("Parede_Fundo_Inicio"));
	// parede do fundo da saida com ABERTURA do tamanho da porta (Y -69..57, ate Z=236); coordenadas reais em cm
	auto WallPiece = [&](const TCHAR* N, FVector C, FVector Sz) { if (ACoopBlock* b = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), C, FRotator::ZeroRotator)) { b->Setup(Sz, Wall); Lbl(b, N); } };
	WallPiece(TEXT("F1_ParedeFundoSaida_Esq"), FVector(3630, -439.5f, 55), FVector(60, 741, 1430));
	WallPiece(TEXT("F1_ParedeFundoSaida_Dir"), FVector(3630, 433.5f, 55), FVector(60, 753, 1430));
	WallPiece(TEXT("F1_ParedeFundoSaida_Topo"), FVector(3630, 0, 503), FVector(60, 1620, 534));
	WallPiece(TEXT("F1_PortaSoleira"), FVector(3630, -6, 29), FVector(60, 130, 30));
	// teto cobrindo a sala toda (visual, cai no colapso) + paineis de luz + luzes sem sombra
	auto Roofed = [&](const TCHAR* Name, FVector C, FVector S, FLinearColor Col, FLinearColor Em = FLinearColor::Black) -> ACoopCeiling*
	{
		ACoopCeiling* c = W->SpawnActor<ACoopCeiling>(ACoopCeiling::StaticClass(), Sc(C), FRotator::ZeroRotator);
		if (c) { c->Setup(Sc(S), Col, false, Em, 0.9f, 0.f); Lbl(c, Name); Roof.Add(c); }
		return c;
	};
	if (ACoopCeiling* T = Roofed(TEXT("F1_Teto"), FVector(500, 0, 620), FVector(3890, 1080, 40), FLinearColor(0.22f, 0.23f, 0.26f))) T->SetOpts(TEXT("Teto_Principal"), false);
	for (float X : {-1200.f, -800.f, 300.f, 700.f, 1100.f, 1500.f, 1900.f, 2250.f})
		for (float Y : {-250.f, 250.f})
		{
			Roofed(TEXT("F1_PainelLuz"), FVector(X, Y, 520), FVector(160, 50, 8), FLinearColor(0.8f, 0.8f, 0.75f), FLinearColor(1.f, 0.95f, 0.8f) * 3.f);
			if (ACoopLamp* L = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), Sc(FVector(X, Y, 500)), FRotator::ZeroRotator)) L->Setup(X > 900.f ? FLinearColor(0.82f, 0.9f, 1.f) : FLinearColor(1.f, 0.9f, 0.78f), 7000.f * KLUM, 900.f * KXY);
		}
	// forro de paineis do Blender (lado detalhado com grade e luzes para baixo), base em Z=580; cai junto no colapso
	for (int32 ix = 0; ix < 10; ++ix)
		for (int32 iy = -1; iy <= 1; ++iy)
			if (ACoopCeiling* c = W->SpawnActor<ACoopCeiling>(ACoopCeiling::StaticClass(), FVector(-2145.f + 291.75f + 583.5f * ix, 540.f * iy, 620.f), FRotator::ZeroRotator))
			{ c->Setup(FVector(583.5f, 540.f, 80.f), FLinearColor(0.1f, 0.11f, 0.12f)); c->SetOpts(TEXT("Teto_Painel"), true); Lbl(c, TEXT("F1_TetoPainel")); Roof.Add(c); }
	// luz sob o teto estrutural (ilumina a coluna) e brilho da lava (sem sombras)
	if (ACoopLamp* L = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), Sc(FVector(-200, 0, 470)), FRotator::ZeroRotator)) L->Setup(FLinearColor(1.f, 0.95f, 0.85f), 9000.f * KLUM, 900.f * KXY);
	if (ACoopLamp* L = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), Sc(FVector(500, 0, -250)), FRotator::ZeroRotator)) L->Setup(FLinearColor(1.f, 0.35f, 0.08f), 40000.f * KLUM, 1600.f * KXY);
	// B. coluna estrutural DESTRUTIVEL: ao lado do caminho da ponte (Y=-330), 160x160x560, sustenta o teto estrutural
	if (ACoopDestructible* Col = W->SpawnActorDeferred<ACoopDestructible>(ACoopDestructible::StaticClass(), FTransform(Sc(FVector(-200, -330, 280)))))
	{
		Col->Setup(Sc(FVector(160, 160, 560)), FLinearColor(0.62f, 0.62f, 0.64f));
		Col->Model = TEXT("Coluna_Inteira");
		Col->FinishSpawning(FTransform(Sc(FVector(-200, -330, 280))));
		Lbl(Col, TEXT("F1_Coluna")); Column = Col;
	}
	// teto estrutural visivel apoiado pela coluna (X -500..100, base em Z=560 = topo da coluna)
	// D. ponte danificada: toco ligado ao lado direito (X 850..1000, largura 250) + laje caida no vao
	M(B(TEXT("F1_Ponte_Toco"), FVector(974, 0, 26), FVector(52, 240, 24), FLinearColor(0.55f, 0.52f, 0.46f)), TEXT("Ponte_Quebrada"));
	// ponte DESTRUTIVEL: nasce destruida (escombros na lava); Neon so a constroi depois que a coluna cair
	if (ACoopDestructible* Br = W->SpawnActorDeferred<ACoopDestructible>(ACoopDestructible::StaticClass(), FTransform(Sc(FVector(500, 0, 25)))))
	{
		Br->Setup(Sc(FVector(1000, 250, 30)), FLinearColor(0.55f, 0.52f, 0.46f), 360.f * KZ, 350.f * KXY, true);
		Br->Model = TEXT("Ponte_Inteira");
		Br->FinishSpawning(FTransform(Sc(FVector(500, 0, 25))));
		Lbl(Br, TEXT("F1_Ponte")); Bridge = Br;
	}
	if (Column && Bridge) { Column->Partner = Bridge; Bridge->Partner = Column; }
	if (ACoopBlock* Slab = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), Sc(FVector(780, 0, -140)), FRotator(-35.f, 0.f, 0.f)))
	{ Slab->Setup(Sc(FVector(150, 250, 30)), FLinearColor(0.45f, 0.42f, 0.38f)); Lbl(Slab, TEXT("F1_Ponte_LajeCaida")); }
	BuildExitDoor(3576.f, 0.f, 44.f);
	BuildTransition();

	SpawnBarrier();
	W->SpawnActor<ACoopFog>(ACoopFog::StaticClass(), Sc(FVector(500, 0, 200)), FRotator::ZeroRotator);
	BuildDecor();

	Starts.Add(W->SpawnActor<APlayerStart>(Sc(FVector(-1250, -150, 100)), FRotator::ZeroRotator));
	Starts.Add(W->SpawnActor<APlayerStart>(Sc(FVector(-1250, 150, 100)), FRotator::ZeroRotator));
	Starts.Add(W->SpawnActor<APlayerStart>(Sc(FVector(-1350, 0, 100)), FRotator::ZeroRotator));
}

bool ACoopFase1GameMode::ToggleTestBridge()
{
	if (!Bridge) return false;
	Bridge->ServerForce(!Bridge->IsDestroyed());
	return !Bridge->IsDestroyed();
}

void ACoopFase1GameMode::CheckFall()
{
	TickCollapse();
	TickExit();
	// barreira invisivel do vao existe so enquanto a ponte nao existe
	if (Bridge)
	{
		const bool bHasBarrier = Barrier && IsValid(Barrier);
		if (Bridge->IsDestroyed() && !bHasBarrier) SpawnBarrier();
		else if (!Bridge->IsDestroyed() && bHasBarrier) { Barrier->Destroy(); Barrier = nullptr; }
	}
	if (Starts.Num() == 0) return;
	int32 i = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It, ++i)
	{
		APlayerController* PC = It->Get();
		APawn* P = PC ? PC->GetPawn() : nullptr;
		if (!P || P->GetActorLocation().Z > -120.f) continue;
		P->SetActorLocation(Starts[i % Starts.Num()]->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		if (ACharacter* C = Cast<ACharacter>(P)) C->GetCharacterMovement()->StopMovementImmediately();
	}
}

void ACoopFase1GameMode::Announce(const FString& Text)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		if (ACoopCharacter* C = It->Get() ? Cast<ACoopCharacter>(It->Get()->GetPawn()) : nullptr) C->ClientToast(Text);
}

void ACoopFase1GameMode::SendAllToStart()
{
	if (Starts.Num() == 0) return;
	int32 i = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It, ++i)
	{
		APawn* P = It->Get() ? It->Get()->GetPawn() : nullptr;
		if (!P) continue;
		P->SetActorLocation(Starts[i % Starts.Num()]->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		if (ACharacter* C = Cast<ACharacter>(P)) C->GetCharacterMovement()->StopMovementImmediately();
	}
}

void ACoopFase1GameMode::ClearCracks()
{
	for (AActor* A : Cracks) if (A && IsValid(A)) A->Destroy();
	Cracks.Reset(); CracksSpawned = 0;
}

// rachadura em zigue-zague (3 segmentos) colada na face interna de uma das 4 paredes
void ACoopFase1GameMode::SpawnCrackCluster(int32 Idx)
{
	UWorld* W = GetWorld();
	FRandomStream R(1000 + Idx * 7919);
	if (bLayout2)
	{
		if (!CrackBox.IsValid) return;
		float U = R.FRandRange(CrackBox.Min.X, CrackBox.Max.X), Z = R.FRandRange(CrackBox.Min.Z, CrackBox.Max.Z);
		const float Base = 90.f + R.FRandRange(-40.f, 40.f);
		for (int32 s = 0; s < 3; ++s)
		{
			const float L = R.FRandRange(100.f, 220.f), Th = Base + (s % 2 ? 1.f : -1.f) * R.FRandRange(15.f, 35.f);
			const float dU = L * FMath::Cos(FMath::DegreesToRadians(Th)), dZ = L * FMath::Sin(FMath::DegreesToRadians(Th));
			if (ACoopBlock* B = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), FVector(U + dU * 0.5f, CrackWallY, Z + dZ * 0.5f), FRotator(Th, 0.f, 0.f)))
			{ B->Setup(FVector(L, 4.f, 8.f), FLinearColor(0.02f, 0.02f, 0.025f), false, FLinearColor(1.f, 0.28f, 0.04f) * 1.5f, 0.9f, 0.f); B->SetOpts(NAME_None, true); Lbl(B, TEXT("F1_Rachadura")); Cracks.Add(B); }
			U += dU; Z += dZ;
		}
		return;
	}
	const int32 Wall = Idx % 4; // 0: lateral Y- | 1: lateral Y+ | 2: fundo inicio | 3: fundo saida
	const bool bSide = Wall < 2;
	float U = bSide ? R.FRandRange(-1250.f, 2250.f) : R.FRandRange(-380.f, 380.f);
	float Z = R.FRandRange(110.f, 440.f);
	const float Base = 90.f + R.FRandRange(-40.f, 40.f); // quase vertical
	for (int32 s = 0; s < 3; ++s)
	{
		const float L = R.FRandRange(70.f, 150.f);
		const float Th = Base + (s % 2 ? 1.f : -1.f) * R.FRandRange(15.f, 35.f);
		const float dU = L * FMath::Cos(FMath::DegreesToRadians(Th)), dZ = L * FMath::Sin(FMath::DegreesToRadians(Th));
		const float CU = U + dU * 0.5f, CZ = Z + dZ * 0.5f;
		FVector Loc; FRotator Rot; FVector Sz;
		if (bSide) { Loc = Sc(FVector(CU, Wall == 0 ? -498.6f : 498.6f, CZ)); Rot = FRotator(Th, 0.f, 0.f); Sz = FVector(L * KXY, 3.f, 6.f); }
		else { Loc = Sc(FVector(Wall == 2 ? -1398.6f : 2398.6f, CU, CZ)); Rot = FRotator(0.f, 0.f, -Th); Sz = FVector(3.f, L * KXY, 6.f); }
		if (ACoopBlock* B = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), Loc, Rot))
		{
			B->Setup(Sz, FLinearColor(0.02f, 0.02f, 0.025f), false, FLinearColor(1.f, 0.28f, 0.04f) * 1.2f, 0.9f, 0.f);
			B->SetOpts(NAME_None, true);
			Lbl(B, TEXT("F1_Rachadura"));
			Cracks.Add(B);
		}
		U += dU; Z += dZ;
	}
}

// Colapso: coluna destruida -> 8s de contagem com rachaduras -> teto desaba -> todos voltam ao inicio e tudo reinicia.
// Se a coluna for reconstruida durante a contagem, o teto se estabiliza (contagem cancelada).
void ACoopFase1GameMode::TickCollapse()
{
	if (!Column) return;
	const float Now = GetWorld()->GetTimeSeconds();
	const bool bColDown = Column->IsDestroyed();
	if (Phase == 0)
	{
		if (bColDown) { Phase = 1; PhaseT0 = Now; CracksSpawned = 0; LastSecShown = -1; if (Rig) Rig->ServerBeginInstability(CollapseSecs); }
	}
	else if (Phase == 1)
	{
		if (!bColDown) { Phase = 0; ClearCracks(); if (Rig) Rig->ServerStabilize(); Announce(TEXT("Coluna restaurada: teto estabilizado")); return; }
		const float El = Now - PhaseT0;
		const int32 Left = FMath::CeilToInt(CollapseSecs - El);
		if (Left != LastSecShown && Left > 0) { LastSecShown = Left; Announce(FString::Printf(TEXT("TETO INSTAVEL! Desaba em %d s - reconstrua a coluna!"), Left)); }
		const int32 Want = FMath::Min(24, FMath::FloorToInt(El / 0.5f) + 1);
		while (CracksSpawned < Want) SpawnCrackCluster(CracksSpawned++);
		if (El >= CollapseSecs)
		{
			Phase = 2; PhaseT0 = Now;
			for (ACoopCeiling* C : Roof) if (C) C->StartFall(RoofFallSecs, RoofDrop);
			if (Rig)
			{
				Rig->ServerBeginFall(); bRigCrushed = false;
				// derrota controlada: todos param e se protegem, camera de derrota (do rig) mostra o desabamento
				for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
					if (APlayerController* PC = It->Get())
					{
						if (ACoopCharacter* Ch = Cast<ACoopCharacter>(PC->GetPawn())) Ch->SetBrace(true);
						PC->ClientIgnoreMoveInput(true);
						PC->SetViewTargetWithBlend(Rig, 0.45f, VTBlend_EaseInOut, 2.f);
					}
			}
			Announce(Rig ? TEXT("A ESTRUTURA ESTA DESABANDO!") : TEXT("O TETO DESABOU!"));
		}
	}
	else if (Phase == 2 && Rig && !bRigCrushed && Now - PhaseT0 >= Rig->ImpactDelay())
	{
		// layout 2: o esmagamento acontece no impacto visivel da secao do teto (servidor), nao antes
		bRigCrushed = true;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			if (ACoopCharacter* C = It->Get() ? Cast<ACoopCharacter>(It->Get()->GetPawn()) : nullptr) C->SetCrushed(true, true);
		Announce(TEXT("FALHA: a estrutura desabou sobre voces"));
	}
	else if (Phase == 2 && Now - PhaseT0 >= (Rig ? Rig->ImpactDelay() + 2.8f : 1.4f))   // layout 2: reinicia so depois da cena de derrota
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			if (ACoopCharacter* C = It->Get() ? Cast<ACoopCharacter>(It->Get()->GetPawn()) : nullptr) { C->SetCrushed(false); C->SetBrace(false); }
		SendAllToStart();                    // todos voltam ao inicio
		if (Rig)
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
				if (APlayerController* PC = It->Get()) { PC->ClientIgnoreMoveInput(false); if (PC->GetPawn()) PC->SetViewTargetWithBlend(PC->GetPawn(), 0.35f); }
		ClearCracks();
		for (ACoopCeiling* C : Roof) if (C) C->ResetPos();
		if (Rig) Rig->ServerReset();
		Column->ServerForce(false, !Rig);    // estruturas voltam ao estado inicial (layout 2: sem animacao de montagem)
		if (Bridge) Bridge->ServerForce(true, !Rig);
		Phase = 0;
		if (ExitDoor) ExitDoor->SetOpen(false);
		if (ExitDoor2) ExitDoor2->SetOpen(false);
		bDoorLatched = false; bPhaseDone = false;
		Announce(TEXT("Fase reiniciada"));
	}
}

// Area de transicao depois da porta (coordenadas ja em cm reais, sem Sc): corredor ate o "portal" da proxima fase.
static constexpr float TrX0 = 3660.f, TrLen = 1710.f, TrHalfW = 420.f, TrFloorTop = 44.f, TrH = 440.f;
void ACoopFase1GameMode::BuildTransition()
{
	UWorld* W = GetWorld();
	const float X1 = TrX0 + TrLen, CX = (TrX0 + X1) * 0.5f;
	const FLinearColor Dk(0.10f, 0.11f, 0.13f), Wl(0.17f, 0.18f, 0.21f);
	auto Blk = [&](const TCHAR* N, FVector C, FVector S, FLinearColor Col, FLinearColor Em = FLinearColor::Black) -> ACoopBlock*
	{
		ACoopBlock* b = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), C, FRotator::ZeroRotator);
		if (b) { b->Setup(S, Col, false, Em, 0.85f, 0.2f); Lbl(b, N); }
		return b;
	};
	Blk(TEXT("F1_Trans_Piso"), FVector(CX, 0, TrFloorTop - 30.f), FVector(TrLen, TrHalfW * 2 + 80.f, 60.f), Dk);
	for (float Sg : {-1.f, 1.f})
		Blk(TEXT("F1_Trans_Parede"), FVector(CX, Sg * (TrHalfW + 20.f), TrFloorTop + TrH * 0.5f - 30.f), FVector(TrLen, 40.f, TrH + 60.f), Wl);
	Blk(TEXT("F1_Trans_Teto"), FVector(CX, 0, TrFloorTop + TrH + 20.f), FVector(TrLen, TrHalfW * 2 + 80.f, 40.f), Dk);
	Blk(TEXT("F1_Trans_Fundo"), FVector(X1 + 20.f, 0, TrFloorTop + TrH * 0.5f - 30.f), FVector(40.f, TrHalfW * 2 + 80.f, TrH + 60.f), Wl);
	// "portal" luminoso da proxima fase no fundo
	Blk(TEXT("F1_Trans_PortalMoldura"), FVector(X1 - 4.f, 0, TrFloorTop + 160.f), FVector(10.f, 400.f, 340.f), Dk);
	Blk(TEXT("F1_Trans_Portal"), FVector(X1 - 14.f, 0, TrFloorTop + 160.f), FVector(6.f, 320.f, 300.f), FLinearColor(0.02f, 0.1f, 0.2f), FLinearColor(0.2f, 1.2f, 3.f) * 3.f);
	if (ACoopLamp* PL = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), FVector(X1 - 120.f, 0, TrFloorTop + 160.f), FRotator::ZeroRotator)) PL->Setup(FLinearColor(0.2f, 0.7f, 1.f), 22000.f, 900.f);
	// tiras de luz no teto + lampadas
	for (float X = TrX0 + 200.f; X < X1 && !Rig; X += 400.f)
	{
		Blk(TEXT("F1_Trans_Luz"), FVector(X, 0, TrFloorTop + TrH - 84.f), FVector(180.f, 40.f, 6.f), FLinearColor(0.8f, 0.85f, 0.9f), FLinearColor(0.85f, 0.95f, 1.f) * 3.f);
		if (ACoopLamp* L = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), FVector(X, 0, TrFloorTop + TrH - 120.f), FRotator::ZeroRotator)) L->Setup(FLinearColor(0.75f, 0.9f, 1.f), 9000.f, 800.f);
	}
	// faixas de chao guiando ate o portal
	for (float Sg : {-1.f, 1.f})
		Blk(TEXT("F1_Trans_Guia"), FVector(CX, Sg * 150.f, TrFloorTop + 0.5f), FVector(TrLen - 100.f, 8.f, 1.f), FLinearColor(0.02f, 0.05f, 0.08f), FLinearColor(0.1f, 0.8f, 1.f) * 2.f);
	// modelos do Blender: paineis nas paredes e placas no piso
	auto Prop = [&](const TCHAR* Slot, FVector Base, FVector Size, FRotator Rot, FVector Scl = FVector(1.f), bool bCollide = false) -> ACoopBlock*
	{
		if (GBaked || Rig) return nullptr;   // Fase1_Arquitetura: tunel revestido por ACoopTunnelDressing (linguagem da nova fase)
		ACoopBlock* b = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), Base + FVector(0, 0, Size.Z * 0.5f), Rot);
		if (b) { b->Setup(Size, FLinearColor(0.22f, 0.18f, 0.16f), false, FLinearColor::Black, 0.85f, 0.25f); b->SetOpts(FName(Slot), !bCollide); if (!Scl.Equals(FVector(1.f))) b->SetModelScale(Scl); Lbl(b, Slot); b->Tags.Add(TEXT("F1Decor")); if (bCollide) b->Tags.Add(TEXT("F1Col")); }
		return b;
	};
	// paredes revestidas com placas industriais (Blender) lado a lado, em pe; piso todo de placas
	auto Tile = [&](float X0, float Y0, bool bAlongX, float Len, float Z0, float Height, float Yaw)
	{
		const int32 Rows = FMath::Max(1, FMath::RoundToInt(Height / 300.f)), N = FMath::Max(1, FMath::RoundToInt(Len / 140.f));
		const float RowH = Height / Rows, Sx = RowH / 300.f, Sy = Len / (N * 140.f);
		for (int32 r = 0; r < Rows; ++r)
			for (int32 i = 0; i < N; ++i)
			{
				const float A = (i + 0.5f) * Len / N;
				const FVector P(bAlongX ? X0 + A : X0, bAlongX ? Y0 : Y0 + A, Z0 + (r + 0.5f) * RowH - 2.f);
				const FRotator R = (FQuat(FRotator(0, Yaw, 0)) * FQuat(FRotator(0, 0, 90)) * FQuat(FRotator(0, 90.f + (((i + r) & 1) ? 180.f : 0.f), 0))).Rotator();
				Prop(TEXT("Placa_Industrial"), P, FVector(300, 140, 4), R, FVector(Sx, Sy, 1.f));
			}
	};
	Tile(TrX0, -(TrHalfW - 4.f), true, TrLen, TrFloorTop, TrH, 0.f);
	Tile(TrX0, TrHalfW - 4.f, true, TrLen, TrFloorTop, TrH, 180.f);
	for (int32 i = 0; i < 5; ++i) Prop(TEXT("Placa_Industrial"), FVector(TrX0 + 150.f + 300.f * i, 0, TrFloorTop), FVector(300, 140, 4), FRotator(0, 0, 0));
	// segunda fileira de paineis de parede (mais alta), forro de paineis de teto do Blender
	for (int32 i = 0; i < 3; ++i)
	{
		const float X = TrX0 + 285.f + 570.f * i;
		for (float Sg : {-1.f, 1.f})
			Prop(TEXT("Teto_Painel"), FVector(TrX0 + 285.f + 570.f * i, Sg * 210.f, TrFloorTop + TrH - 80.f), FVector(570, 421, 80), FRotator(0, 0, 0), FVector(0.977f, 0.78f, 1.f));
	}
	// parede do fundo do tunel (dos dois lados do portal)
	for (float Sg : {-1.f, 1.f}) Tile(X1 - 4.f, Sg > 0 ? 204.f : -TrHalfW, false, TrHalfW - 204.f, TrFloorTop, TrH, 90.f);
	// parede de entrada (lado do tunel, em frente ao portal): paineis, moldura da porta e luz
	{
		const float EX = TrX0 + 7.f;
		Tile(EX - 3.f, -TrHalfW, false, TrHalfW - 100.f, TrFloorTop, TrH, -90.f);
		Tile(EX - 3.f, 76.f, false, TrHalfW - 76.f, TrFloorTop, TrH, -90.f);
		Tile(EX - 3.f, -100.f, false, 176.f, TrFloorTop + 286.f, TrH - 286.f, -90.f);
		Prop(TEXT("Porta_Quadro"), FVector(TrX0 + 29.f, -11.f, TrFloorTop), FVector(56.6f, 280.4f, 286.f), FRotator(0, 180, 0));
		for (const FVector2D& P : { FVector2D(-115.5f, 71.f), FVector2D(87.5f, 83.f) })
			if (ACoopBlock* G = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), FVector(TrX0 + 29.f, P.X, TrFloorTop + 143.f), FRotator::ZeroRotator))
			{ G->Setup(FVector(57.f, P.Y, 286.f), FLinearColor::Black, true); Lbl(G, TEXT("F1_Trans_PortaPilar_Colisao")); }
		Blk(TEXT("F1_Trans_LuzPorta"), FVector(TrX0 + 60.f, -6.f, TrFloorTop + 300.f), FVector(6.f, 140.f, 5.f), FLinearColor(0.1f, 0.5f, 0.8f), FLinearColor(0.2f, 1.f, 2.5f) * 3.f);
		Prop(TEXT("Estrutura_Aco"), FVector(TrX0 + 110.f, -330.f, TrFloorTop), FVector(169, 139, 380), FRotator(0, 0, 0), FVector(1.f, 1.f, 0.95f), true);
		Prop(TEXT("Estrutura_Aco"), FVector(TrX0 + 110.f, 330.f, TrFloorTop), FVector(169, 139, 380), FRotator(0, 0, 0), FVector(1.f, 1.f, 0.95f), true);
	}
	// estruturas de aco flanqueando o portal, pilares de ruido, cabos e canos
	Prop(TEXT("Estrutura_Aco"), FVector(X1 - 110.f, -330.f, TrFloorTop), FVector(169, 139, 380), FRotator(0, 180, 0), FVector(1.f, 1.f, 0.95f), true);
	Prop(TEXT("Estrutura_Aco"), FVector(X1 - 110.f, 330.f, TrFloorTop), FVector(169, 139, 380), FRotator(0, 180, 0), FVector(1.f, 1.f, 0.95f), true);
	Prop(TEXT("Pilar_Ruido"), FVector(TrX0 + 430.f, 330.f, TrFloorTop), FVector(143, 124, 260), FRotator(0, 20, 0), FVector(1.f), true);
	Prop(TEXT("Pilar_Ruido"), FVector(TrX0 + 1000.f, -330.f, TrFloorTop), FVector(143, 124, 260), FRotator(0, -35, 0), FVector(1.f), true);
	Prop(TEXT("Cabos"), FVector(TrX0 + 700.f, 280.f, TrFloorTop), FVector(200, 124, 20), FRotator(0, 15, 0));
	Prop(TEXT("Cabos"), FVector(TrX0 + 1300.f, -250.f, TrFloorTop), FVector(200, 124, 20), FRotator(0, 100, 0));
	Prop(TEXT("Cano_Curvo"), FVector(TrX0 + 250.f, -300.f, TrFloorTop), FVector(160, 105, 19), FRotator(0, 25, 0));
	Prop(TEXT("Cano_Teto"), FVector(CX, -250.f, TrFloorTop + 330.f), FVector(1700, 60, 61), FRotator(0, 0, 0), FVector(0.3f, 1.f, 1.f));
	Prop(TEXT("Cano_Teto"), FVector(CX, 250.f, TrFloorTop + 330.f), FVector(1700, 60, 61), FRotator(0, 0, 0), FVector(0.3f, 1.f, 1.f));
	for (float X : {TrX0 + 350.f, TrX0 + 950.f, TrX0 + 1450.f})
		for (float Sg : {-1.f, 1.f}) Prop(TEXT("Suporte_Aco"), FVector(X, Sg * 250.f, TrFloorTop + 320.f), FVector(18, 200, 93), FRotator(0, 90, 0));
	Prop(TEXT("Arcade"), FVector(TrX0 + 1150.f, 340.f, TrFloorTop), FVector(101, 142, 190), FRotator(0, 180, 0), FVector(1.f), true);
	Prop(TEXT("Caixa_Madeira"), FVector(TrX0 + 120.f, 340.f, TrFloorTop), FVector(120, 120, 79), FRotator(0, 12, 0), FVector(1.f), true);
	Prop(TEXT("Piso_Grade"), FVector(X1 - 380.f, 0, TrFloorTop + 4.f), FVector(300, 300, 6), FRotator(0, 0, 0));
	Prop(TEXT("Grade_Ralo"), FVector(TrX0 + 700.f, -300.f, TrFloorTop), FVector(200, 200, 5), FRotator(0, 0, 0));
}

// Porta: abre quando TODOS os jogadores (min. 1) estao perto dela; fica aberta. Fim da fase: todos na zona do portal.
void ACoopFase1GameMode::TickExit()
{
	if (!ExitDoor) return;
	const FVector DoorLoc = ExitCenter;
	int32 N = 0, Near = 0, InZone = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APawn* P = It->Get() ? It->Get()->GetPawn() : nullptr;
		if (!P) continue;
		++N;
		const FVector L = P->GetActorLocation();
		if (FVector::Dist2D(L, DoorLoc) < 650.f && FMath::Abs(L.Z - DoorLoc.Z) < 400.f) ++Near;
		if (L.X > TrX0 + L2Off.X + TrLen - 380.f && FMath::Abs(L.Y - L2Off.Y) < TrHalfW) ++InZone;
	}
	if (N == 0) return;
	if (!bDoorLatched && Near == N)
	{
		bDoorLatched = true; ExitDoor->SetOpen(true); if (ExitDoor2) ExitDoor2->SetOpen(true);
		Announce(N > 1 ? TEXT("Todos juntos: PORTA ABERTA!") : TEXT("PORTA ABERTA!"));
	}
	if (bDoorLatched && !bPhaseDone && InZone == N)
	{
		bPhaseDone = true;
		if (FPackageName::DoesPackageExist(TEXT("/Game/Maps/Fase2")))
		{
			Announce(TEXT("FASE 1 CONCLUIDA! Indo para a proxima fase..."));
			GetWorld()->ServerTravel(TEXT("/Game/Maps/Fase2?listen"));
		}
		else Announce(TEXT("FASE 1 CONCLUIDA! A proxima fase vem em breve."));
	}
}

// Decoracao procedural (so visual, SEM colisao) para dar profundidade/atmosfera ate os modelos do Blender entrarem.
// Cada peca tem um "slot": se existir /Game/Fase1/<Slot>/SM_<Slot>, o modelo substitui o placeholder (ver MapaFase1/MODELOS.md).
void ACoopFase1GameMode::BuildDecor()
{
	UWorld* W = GetWorld();
	auto D = [&](const TCHAR* Slot, FVector C, FVector S, FLinearColor Col, FLinearColor Em = FLinearColor::Black, FRotator Rot = FRotator::ZeroRotator, bool bCollide = false) -> ACoopBlock*
	{
		const bool bHasModel = HasModel(Slot);
		if (GBaked && bHasModel) return nullptr;
		ACoopBlock* b = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), Sc(C), Rot);
		if (b) { b->Setup(Sc(S), Col, false, Em, 0.85f, 0.25f); b->SetOpts(FName(Slot), !bCollide); Lbl(b, Slot); if (bHasModel) { b->Tags.Add(TEXT("F1Decor")); if (bCollide) b->Tags.Add(TEXT("F1Col")); } }
		return b;
	};
	auto Lamp = [&](FVector P, FLinearColor C, float Lm, float Rad, float Fl) { if (ACoopLamp* L = W->SpawnActor<ACoopLamp>(ACoopLamp::StaticClass(), Sc(P), FRotator::ZeroRotator)) L->Setup(C, Lm * KLUM, Rad * KXY, Fl); };
	const FLinearColor Rust(0.30f, 0.13f, 0.06f), Dark(0.05f, 0.05f, 0.06f), Steel(0.36f, 0.38f, 0.41f), Grime(0.16f, 0.15f, 0.15f);
	const FLinearColor Cyan(0.05f, 0.9f, 1.f), Orange(1.f, 0.4f, 0.05f), Fire(1.f, 0.3f, 0.04f);
	FRandomStream R(4242);

	// canos no teto + vigas quebradas penduradas sobre o vao + gotejo de lava
	D(TEXT("Cano_Teto"), FVector(500, 363.3f, 480), FVector(3700, 36, 36), Rust);      // Y +545 real
	D(TEXT("Cano_Grosso"), FVector(500, -353.3f, 475), FVector(3700, 56, 56), Steel);  // Y -530 real
	D(TEXT("Viga_Quebrada"), FVector(250, -120, 470), FVector(340, 26, 26), Rust, FLinearColor::Black, FRotator(-28.f, 15.f, 0.f));
	D(TEXT("Viga_Quebrada"), FVector(700, 150, 500), FVector(300, 26, 26), Steel, FLinearColor::Black, FRotator(24.f, -25.f, 0.f));
	D(TEXT("Viga_Quebrada"), FVector(480, 20, 461), FVector(280, 22, 22), Rust, FLinearColor::Black, FRotator(-30.f, 40.f, 8.f));
	D(TEXT("Viga_Quebrada"), FVector(850, -150, 520), FVector(240, 22, 22), Steel, FLinearColor::Black, FRotator(35.f, 10.f, 0.f));
	// fluxos de lava + brilho vindo do fundo do vao
	const FVector4 Flows[] = { FVector4(150, -150, 260, 380), FVector4(430, 200, 300, 330), FVector4(720, -90, 280, 420), FVector4(900, 230, 200, 300) };
	for (const FVector4& F : Flows) D(TEXT("Lava_Fluxo"), FVector(F.X, F.Y, -349.f), FVector(F.Z, F.W, 2), FLinearColor::Black, FLinearColor(1.f, 0.5f, 0.05f) * 5.f);
	for (float X : {200.f, 500.f, 800.f}) Lamp(FVector(X, 0, -200), FLinearColor(1.f, 0.35f, 0.08f), 30000.f, 1400.f, 0.35f);
	// focos de incendio
	const FVector2D Fires[] = { FVector2D(-1250, -420), FVector2D(-950, 440), FVector2D(-600, -440), FVector2D(1400, 440), FVector2D(2000, -440) };
	for (const FVector2D& P : Fires)
	{
		const float Z0 = P.X > 900.f ? 40.f : 0.f;
		for (int32 i = 0; i < 4; ++i)
		{
			const float Hh = R.FRandRange(60.f, 150.f), Ww = R.FRandRange(28.f, 55.f);
			D(TEXT("Fogo"), FVector(P.X + R.FRandRange(-50.f, 50.f), P.Y + R.FRandRange(-50.f, 50.f), Z0 + Hh * 0.5f), FVector(Ww, Ww, Hh), FLinearColor::Black, Fire * 3.f, FRotator(0.f, R.FRandRange(0.f, 90.f), 0.f));
		}
		Lamp(FVector(P.X, P.Y, Z0 + 110), FLinearColor(1.f, 0.4f, 0.08f), 14000.f, 750.f, 0.5f);
	}
	// juntas do piso (ladrilhos) e faixas de perigo nas bordas
	for (int32 i = 0; i < 7; ++i) D(TEXT("Piso_Junta"), FVector(-1300.f + i * 200.f, 0, 0.6f), FVector(3, 1000, 1), Dark);
	for (int32 i = 0; i < 4; ++i) D(TEXT("Piso_Junta"), FVector(-700, -300.f + i * 200.f, 0.6f), FVector(1400, 3, 1), Dark);
	for (int32 i = 0; i < 7; ++i) D(TEXT("Piso_Junta"), FVector(1100.f + i * 200.f, 0, 40.6f), FVector(3, 1000, 1), Dark);
	for (int32 i = 0; i < 4; ++i) D(TEXT("Piso_Junta"), FVector(1700, -300.f + i * 200.f, 40.6f), FVector(1400, 3, 1), Dark);
	D(TEXT("Faixa_Perigo"), FVector(-15, 0, 0.7f), FVector(24, 1000, 1), FLinearColor(0.9f, 0.7f, 0.05f));
	D(TEXT("Faixa_Perigo"), FVector(1015, 0, 40.7f), FVector(24, 1000, 1), FLinearColor(0.9f, 0.7f, 0.05f));
	// corrimao laranja nas bordas do vao (abertura de 280 para a ponte) - so visual
	for (float EdgeX : {-6.f, 1006.f})
	{
		const float Z0 = EdgeX > 500.f ? 40.f : 0.f;
		D(TEXT("Corrimao"), FVector(EdgeX, -320, Z0 + 100), FVector(5, 360, 5), Orange * 0.6f, FLinearColor::Black);
		D(TEXT("Corrimao"), FVector(EdgeX, 320, Z0 + 100), FVector(5, 360, 5), Orange * 0.6f, FLinearColor::Black);
		for (float Y : {-320.f, 320.f})
			if (ACoopBlock* G = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), Sc(FVector(EdgeX, Y, Z0 + 60)), FRotator::ZeroRotator))
			{ G->Setup(Sc(FVector(12, 360, 120)), FLinearColor::Black, true); Lbl(G, TEXT("F1_GuardaCorpo_Colisao")); }
		for (float Y : {-480.f, -360.f, -240.f, 240.f, 360.f, 480.f}) D(TEXT("Corrimao_Poste"), FVector(EdgeX, Y, Z0 + 50), FVector(6, 6, 100), Rust);
	}
	// sinalizacao de saida (EXIT) + moldura da porta
	// (a placa EXIT agora faz parte do modelo da porta)

	// ---- Props do Blender (tamanhos em cm reais; Base = centro da base do modelo; Size = tamanho exportado) ----
	// Modelo "de frente" = +Y local (Yaw 0). Yaw 180 = de frente para -Y.
	auto Prop = [&](const TCHAR* Slot, FVector Base, FVector Size, FRotator Rot, bool bCollide) -> ACoopBlock*
	{
		if (GBaked) return nullptr;
		ACoopBlock* b = W->SpawnActor<ACoopBlock>(ACoopBlock::StaticClass(), Base + FVector(0, 0, Size.Z * 0.5f), Rot);
		if (b) { b->Setup(Size, FLinearColor(0.22f, 0.18f, 0.16f), false, FLinearColor::Black, 0.85f, 0.25f); b->SetOpts(FName(Slot), !bCollide); Lbl(b, Slot); b->Tags.Add(TEXT("F1Decor")); if (bCollide) b->Tags.Add(TEXT("F1Col")); }
		return b;
	};
	const float ZS = 0.f, ZE = 44.f;   // topo do piso: plataforma inicial / de saida
	// fachadas de laboratorio queimado nas paredes laterais (parede Y- olha para +Y, parede Y+ olha para -Y)
	for (float X : {-1100.f, 2200.f, 3050.f})
	{
		const float Z0 = X > 1500.f ? ZE : ZS;
		Prop(TEXT("Fachada_Queimada"), FVector(X, -700, Z0), FVector(380, 70, 300), FRotator(0, 0, 0), true);
		Prop(TEXT("Fachada_Queimada"), FVector(X, 700, Z0), FVector(380, 70, 300), FRotator(0, 180, 0), true);
	}
	Prop(TEXT("Cabine_Fortaleza"), FVector(-1850, 550, ZS), FVector(300, 264, 273), FRotator(0, 180, 0), true);
	Prop(TEXT("Pilar_Ruido"), FVector(-350, 610, ZS), FVector(143, 124, 260), FRotator(0, 0, 0), true);     // eixo de colunas X -350 / Y +610 (coluna danificada perto do vao)
	Prop(TEXT("Pilar_Ruido"), FVector(1850, 610, ZE), FVector(143, 124, 260), FRotator(0, 0, 0), true);     // eixo X 1850 / Y +610
	for (const FVector2D& P : { FVector2D(1850, -610), FVector2D(3290, -610), FVector2D(3290, 610) })      // pilares (ex-mezanino) no eixo da plataforma final, sob a linha de canos
		Prop(TEXT("Mezanino_Pilar"), FVector(P.X, P.Y, ZE), FVector(70, 76, 440), FRotator(0, 0, 0), true);
	Prop(TEXT("Estrutura_Aco"), FVector(-1980, -480, ZS), FVector(169, 139, 380), FRotator(0, 0, 0), true);
	Prop(TEXT("Estrutura_Aco"), FVector(3450, 450, ZE), FVector(169, 139, 380), FRotator(0, 180, 0), true);
	for (float X : {-1815.f, -960.f, -105.f, 750.f, 1605.f, 2460.f, 3315.f})
		for (float Sgn : {-1.f, 1.f})
			Prop(TEXT("Suporte_Aco"), FVector(X, Sgn * 647.f, 480), FVector(18, 200, 93), FRotator(0, Sgn > 0 ? 180.f : 0.f, 0), false);
	for (float X : {-1950.f, -650.f, 1900.f, 3420.f})
		for (float Sgn : {-1.f, 1.f})
			Prop(TEXT("Cano_Parede"), FVector(X, Sgn * 717.f, 320), FVector(55, 55, 10), FRotator(90.f, 0, 0), false);
	Prop(TEXT("Cano_Curvo"), FVector(-1200, -300, ZS), FVector(160, 105, 19), FRotator(0, 20, 0), false);
	Prop(TEXT("Cano_Curvo"), FVector(2700, 300, ZE), FVector(160, 105, 19), FRotator(0, -35, 0), false);
	Prop(TEXT("Cabos"), FVector(-700, 400, ZS), FVector(200, 124, 20), FRotator(0, 10, 0), false);
	Prop(TEXT("Cabos"), FVector(2400, -350, ZE), FVector(200, 124, 20), FRotator(0, 80, 0), false);
	const FVector Piles[] = { FVector(-150, -640, ZS), FVector(-1500, -380, ZS), FVector(-900, 230, ZS), FVector(2200, 200, ZE), FVector(2900, -150, ZE), FVector(3300, 500, ZE) };
	int32 pi = 0;
	for (const FVector& P : Piles) Prop(TEXT("Pilha_Escombro"), P, FVector(130, 135, 85), FRotator(0, 47.f * (pi++), 0), false);
	Prop(TEXT("Piso_Placa"), FVector(-1100, 450, ZS), FVector(220, 220, 5), FRotator(0, 0, 0), false);
	Prop(TEXT("Piso_Placa"), FVector(2800, 450, ZE), FVector(220, 220, 5), FRotator(0, 0, 0), false);
	Prop(TEXT("Piso_Grade"), FVector(-600, -420, ZS), FVector(300, 300, 6), FRotator(0, 0, 0), false);
	Prop(TEXT("Piso_Grade"), FVector(3150, -420, ZE), FVector(300, 300, 6), FRotator(0, 0, 0), false);
	Prop(TEXT("Grade_Ralo"), FVector(-1500, -450, ZS), FVector(200, 200, 5), FRotator(0, 0, 0), false);
	Prop(TEXT("Grade_Ralo"), FVector(2400, -450, ZE), FVector(200, 200, 5), FRotator(0, 0, 0), false);
	Prop(TEXT("Caixa_Madeira"), FVector(-1580, 640, ZS), FVector(120, 120, 79), FRotator(0, 8, 0), true);
	Prop(TEXT("Caixa_Madeira"), FVector(-1450, 640, ZS), FVector(120, 120, 79), FRotator(0, -5, 0), true);
	Prop(TEXT("Caixa_Madeira"), FVector(2000, 640, ZE), FVector(120, 120, 79), FRotator(0, 12, 0), true);
	Prop(TEXT("Cubo_Steampunk"), FVector(-1350, -600, ZS), FVector(86, 87, 120), FRotator(0, 15, 0), true);
	Prop(TEXT("Arcade"), FVector(1650, -640, ZE), FVector(101, 142, 190), FRotator(0, 0, 0), true);
	Prop(TEXT("Torre_PC"), FVector(1730, -650, ZE), FVector(24, 26, 62), FRotator(0, 0, 0), true);
	Prop(TEXT("Botao_Vermelho"), FVector(3450, -300, ZE), FVector(50, 52, 19), FRotator(0, 0, 0), false);

	// paredes revestidas de placas industriais (Blender) lado a lado, 4 fileiras, em todas as paredes
	auto Tile = [&](float X0, float Y0, bool bAlongX, float Len, float Z0, float Height, float Yaw)
	{
		const int32 Rows = FMath::Max(1, FMath::RoundToInt(Height / 300.f)), N = FMath::Max(1, FMath::RoundToInt(Len / 140.f));
		const float RowH = Height / Rows, Sx = RowH / 300.f, Sy = Len / (N * 140.f);
		for (int32 r = 0; r < Rows; ++r)
			for (int32 i = 0; i < N; ++i)
			{
				const float A = (i + 0.5f) * Len / N;
				const FVector P(bAlongX ? X0 + A : X0, bAlongX ? Y0 : Y0 + A, Z0 + (r + 0.5f) * RowH - 2.f);
				const FRotator R = (FQuat(FRotator(0, Yaw, 0)) * FQuat(FRotator(0, 0, 90)) * FQuat(FRotator(0, 90.f + (((i + r) & 1) ? 180.f : 0.f), 0))).Rotator();
				if (ACoopBlock* T = Prop(TEXT("Placa_Industrial"), P, FVector(300, 140, 4), R, false)) T->SetModelScale(FVector(Sx, Sy, 1.f));
			}
	};
	Tile(-2100.f, -739.f, true, 3600.f, ZS, 560.f, 0.f);
	Tile(-2100.f, 739.f, true, 3600.f, ZS, 560.f, 180.f);
	Tile(1500.f, -739.f, true, 2100.f, ZE, 560.f, 0.f);
	Tile(1500.f, 739.f, true, 2100.f, ZE, 560.f, 180.f);
	Tile(-2089.f, -739.f, false, 1478.f, ZS, 560.f, -90.f);
	// laterais do vao (abaixo do piso, ate a lava): mesmas placas, sem trecho liso
	Tile(0.f, -739.f, true, 1500.f, -560.f, 560.f, 0.f);
	Tile(0.f, 739.f, true, 1500.f, -560.f, 560.f, 180.f);
	Tile(3589.f, -739.f, false, 639.f, ZE, 560.f, 90.f);
	Tile(3589.f, 76.f, false, 663.f, ZE, 560.f, 90.f);
	Tile(3589.f, -100.f, false, 176.f, ZE + 290.f, 270.f, 90.f);
	// janelas (poucas, a meia altura e meia distancia)
	for (float WX : {-300.f, 2550.f})
		for (float Sg : {-1.f, 1.f})
			if (ACoopBlock* Wn = Prop(TEXT("Painel_Parede"), FVector(WX, Sg * 731.f, (WX > 1500.f ? ZE : ZS) + 190.f), FVector(570, 14, 240), Sg < 0 ? FRotator(0, 0, 0) : FRotator(0, 180, 0), false)) Wn->SetModelScale(FVector(0.5f, 1.f, 0.6f));
	// passarela de placas industriais (coringa) levando a ponte, nas duas plataformas
	for (int32 i = 0; i < 7; ++i)
	{
		Prop(TEXT("Placa_Industrial"), FVector(-2100.f + 150.f + 300.f * i, 0, ZS), FVector(300, 140, 4), FRotator(0, 0, 0), false);
		Prop(TEXT("Placa_Industrial"), FVector(1500.f + 150.f + 300.f * i, 0, ZE), FVector(300, 140, 4), FRotator(0, 0, 0), false);
	}

	// easter eggs (Joint) - um em cada plataforma
	if (ACoopEasterEgg* E1 = W->SpawnActor<ACoopEasterEgg>(ACoopEasterEgg::StaticClass(), FVector(-1700, -560, ZS), FRotator(0, 35, 0)))
		E1->Message = TEXT("Easter egg 1/2 (Fase 1): um joint esquecido no meio dos escombros. Calma, ainda da tempo de salvar o teto.");
	if (ACoopEasterEgg* E2 = W->SpawnActor<ACoopEasterEgg>(ACoopEasterEgg::StaticClass(), FVector(3200, -560, ZE), FRotator(0, -120, 0)))
		E2->Message = TEXT("Easter egg 2/2 (Fase 1): passou pela ponte, ganhou o joint. Joint Journey: a jornada e conjunta.");
}
