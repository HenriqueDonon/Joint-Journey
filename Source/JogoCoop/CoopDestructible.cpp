#include "CoopDestructible.h"
#include "CoopBurst.h"
#include "JJVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/UObjectGlobals.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"

ACoopDestructible::ACoopDestructible()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
	Mesh->SetMobility(EComponentMobility::Movable);
}

void ACoopDestructible::Setup(FVector InSize, FLinearColor InColor, float InDrop, float InSpreadX, bool bStartDestroyed)
{
	Size = InSize; Color = InColor; DebrisDrop = InDrop; DebrisSpreadX = InSpreadX;
	if (bStartDestroyed) bDestroyed = true;
}

void ACoopDestructible::BeginPlay()
{
	Super::BeginPlay();
	if (UStaticMesh* Cube = JJ::Mesh(TEXT("Cube"))) Mesh->SetStaticMesh(Cube);
	Mesh->SetWorldScale3D(Size / 100.f);
	BodyMat = JJ::Mat(this, Color, FLinearColor(0.6f, 0.25f, 1.f) * 0.15f, 0.85f);
	Mesh->SetMaterial(0, BodyMat);
	if (Model != NAME_None)
	{
		const FString P = FString::Printf(TEXT("/Game/Fase1/%s/SM_%s.SM_%s"), *Model.ToString(), *Model.ToString(), *Model.ToString());
		if (UStaticMesh* SM = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *P, nullptr, LOAD_Quiet | LOAD_NoWarn)))
		{
			ModelComp = NewObject<UStaticMeshComponent>(this);
			ModelComp->SetStaticMesh(SM);
			ModelComp->SetupAttachment(Mesh);
			ModelComp->SetAbsolute(false, false, true);
			ModelComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ModelComp->RegisterComponent();
			ModelComp->SetWorldScale3D(FVector(1.f));
			ModelComp->SetWorldLocationAndRotation(GetActorLocation() - FVector(0, 0, Size.Z * 0.5f), GetActorRotation());
		}
	}
	// coluna: fragmentos do proprio modelo (SM_ColFrag_01..); sem eles, mantem os pedacos-cubo antigos
	if (Model == FName(TEXT("Coluna_Inteira")))
		for (int32 i = 1; i <= 24; ++i)
		{
			const FString FP = FString::Printf(TEXT("/Game/Fase1/ColunaFrag/SM_ColFrag_%02d.SM_ColFrag_%02d"), i, i);
			UStaticMesh* FM = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *FP, nullptr, LOAD_Quiet | LOAD_NoWarn));
			if (!FM) break;
			FragMeshes.Add(FM);
		}
	if (FragMeshes.Num() == 0) BuildChunks();
	ApplyState(false);
}

void ACoopDestructible::EndPlay(const EEndPlayReason::Type Reason)
{
	ClearFrags();
	ClearAssembly();
	Super::EndPlay(Reason);
}

void ACoopDestructible::ClearFrags()
{
	if (UWorld* W = GetWorld()) W->GetTimerManager().ClearTimer(FragTimer);
	for (UStaticMeshComponent* F : Frags) if (F) F->DestroyComponent();
	for (UBoxComponent* B : FragBodies) if (B) B->DestroyComponent();
	Frags.Reset(); FragBodies.Reset();
}

void ACoopDestructible::SpawnFrags()
{
	ClearFrags();
	const FVector Base = GetActorLocation() - FVector(0, 0, Size.Z * 0.5f);
	const FRotator Rot = GetActorRotation();
	UMaterialInterface* Mat = ModelComp ? ModelComp->GetMaterial(0) : nullptr;
	FRandomStream R(FMath::Rand());
	for (UStaticMesh* FM : FragMeshes)
	{
		const FBox LB = FM->GetBoundingBox();          // em cm, relativo a base da coluna
		const FVector Cen = LB.GetCenter(), Ext = LB.GetExtent();
		UBoxComponent* B = NewObject<UBoxComponent>(this);
		B->SetMobility(EComponentMobility::Movable);
		B->SetBoxExtent(Ext * 0.8f);                    // um pouco menor que a peca (encaixe visual no chao)
		B->SetWorldLocationAndRotation(Base + Rot.RotateVector(Cen), Rot);
		B->SetHiddenInGame(true);
		B->SetCollisionObjectType(ECC_PhysicsBody);
		B->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		B->SetCollisionResponseToAllChannels(ECR_Block);
		B->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);       // nunca prende/empurra jogador
		B->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		B->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);  // nao atrapalha a mira dos poderes
		B->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore); // pedacos nao se empurram (nascem encostados)
		B->RegisterComponent();
		UStaticMeshComponent* F = NewObject<UStaticMeshComponent>(this);
		F->SetMobility(EComponentMobility::Movable);
		F->SetStaticMesh(FM);
		if (Mat) F->SetMaterial(0, Mat);
		F->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		F->SetupAttachment(B);
		F->SetRelativeLocation(-Cen);
		F->RegisterComponent();
		FragBodies.Add(B); Frags.Add(F);
		const float H = Cen.Z / FMath::Max(Size.Z, 1.f); // 0 = base, 1 = topo
		if (H < 0.1f) { B->SetCollisionEnabled(ECollisionEnabled::NoCollision); B->ComponentTags.Add(TEXT("Toco")); continue; } // toco da base fica no lugar
		B->SetSimulatePhysics(true);
		B->SetLinearDamping(0.15f); B->SetAngularDamping(0.4f);
		FVector Out = Rot.RotateVector(FVector(Cen.X, Cen.Y, 0.f)).GetSafeNormal();
		if (Out.IsNearlyZero()) Out = FVector(R.FRandRange(-1.f, 1.f), R.FRandRange(-1.f, 1.f), 0.f).GetSafeNormal();
		Out = Out.RotateAngleAxis(R.FRandRange(-25.f, 25.f), FVector::UpVector);
		const FVector V = Out * R.FRandRange(140.f, 300.f) + FVector(0, 0, R.FRandRange(60.f, 220.f) * (0.4f + H));
		B->SetPhysicsLinearVelocity(V);
		B->SetPhysicsAngularVelocityInDegrees(FVector(R.FRandRange(-160.f, 160.f), R.FRandRange(-160.f, 160.f), R.FRandRange(-90.f, 90.f)));
	}
	GetWorldTimerManager().SetTimer(FragTimer, this, &ACoopDestructible::SettleFrags, 5.f, false);
}

void ACoopDestructible::SettleFrags()
{
	// apos 5 s: para a fisica e vira entulho estatico sem colisao (some ao reconstruir); o que caiu na lava e removido
	for (int32 i = FragBodies.Num() - 1; i >= 0; --i)
	{
		UBoxComponent* B = FragBodies[i];
		if (!B) continue;
		B->SetSimulatePhysics(false);
		B->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (B->GetComponentLocation().Z < -200.f)
		{
			if (Frags.IsValidIndex(i) && Frags[i]) Frags[i]->DestroyComponent();
			B->DestroyComponent(); FragBodies.RemoveAt(i); if (Frags.IsValidIndex(i)) Frags.RemoveAt(i);
		}
	}
}

void ACoopDestructible::BuildChunks()
{
	// pedacos so visuais (sem colisao) espalhados no chao quando destruido
	UStaticMesh* Cube = JJ::Mesh(TEXT("Cube"));
	UMaterialInstanceDynamic* CM = JJ::Mat(this, Color * 0.7f, FLinearColor(0.7f, 0.3f, 1.f) * 0.6f, 0.9f);
	const FVector L0 = GetActorLocation();
	FRandomStream R(FMath::RoundToInt(L0.X * 7.f + L0.Y * 13.f + L0.Z * 3.f)); // mesmo layout em todas as maquinas
	const int32 N = 10;
	for (int32 i = 0; i < N; ++i)
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetStaticMesh(Cube);
		C->SetMaterial(0, CM);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetupAttachment(RootComponent);
		C->SetAbsolute(false, false, true); // escala absoluta (raiz esta escalada)
		C->RegisterComponent();
		const float S = R.FRandRange(0.18f, 0.42f);
		C->SetWorldScale3D(FVector(S, S * R.FRandRange(0.6f, 1.4f), S * R.FRandRange(0.5f, 1.f)));
		const float OffX = DebrisSpreadX > 0.f ? R.FRandRange(-1.f, 1.f) * DebrisSpreadX : R.FRandRange(-0.6f, 0.6f) * Size.X;
		const float OffY = R.FRandRange(-1.2f, 1.2f) * Size.Y;
		const FVector Off(OffX, OffY, -Size.Z * 0.5f + S * 40.f - DebrisDrop);
		C->SetWorldLocation(GetActorLocation() + Off);
		C->SetWorldRotation(FRotator(R.FRandRange(-25.f, 25.f), R.FRandRange(0.f, 360.f), R.FRandRange(-25.f, 25.f)));
		Chunks.Add(C);
	}
}

void ACoopDestructible::ApplyState(bool bWithFx)
{
	// mesma logica no servidor e nos clientes: visual + colisao seguem o estado replicado
	if (ModelComp) { ModelComp->SetVisibility(!bDestroyed); Mesh->SetVisibility(false, false); }
	else Mesh->SetVisibility(!bDestroyed, false);
	Mesh->SetCollisionEnabled(bDestroyed ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
	for (UStaticMeshComponent* C : Chunks) if (C) C->SetVisibility(bDestroyed);
	if (FragMeshes.Num() > 0 && GetNetMode() != NM_DedicatedServer)
	{
		if (!bDestroyed) ClearFrags();
		else if (Frags.Num() == 0) SpawnFrags();
	}
	ClearAssembly();
	if (bWithFx && bAssemblyFx && !bDestroyed && !bQuietChange && GetNetMode() != NM_DedicatedServer) StartAssembly();
	if (bWithFx && !bQuietChange && GetNetMode() != NM_DedicatedServer)
	{
		const FLinearColor Fx = bDestroyed ? FLinearColor(0.7f, 0.3f, 1.f) : FLinearColor(0.1f, 1.f, 0.4f);
		if (ACoopBurst* B = GetWorld()->SpawnActor<ACoopBurst>(ACoopBurst::StaticClass(), GetActorLocation(), FRotator::ZeroRotator))
			B->Init(Fx, GetActorLocation() + FVector(0, 0, Size.Z * 0.5f));
	}
	bApplied = true;
}

void ACoopDestructible::OnRep_Destroyed() { ApplyState(bApplied); }

bool ACoopDestructible::ServerBreak(FString& OutWhy)
{
	if (!HasAuthority()) { OutWhy = TEXT("sem autoridade"); return false; }
	if (bDestroyed) { OutWhy = TEXT("Ja esta destruido"); return false; }
	bDestroyed = true; bQuietChange = false;
	ApplyState(true); // servidor/host nao recebe OnRep
	ForceNetUpdate();
	return true;
}

bool ACoopDestructible::ServerRebuild(FString& OutWhy)
{
	if (!HasAuthority()) { OutWhy = TEXT("sem autoridade"); return false; }
	if (!bDestroyed) { OutWhy = TEXT("Objeto ja esta intacto"); return false; }
	if (Partner && !Partner->IsDestroyed()) { OutWhy = TEXT("Sem material: destrua a outra estrutura primeiro"); return false; }
	// nao reconstruir em cima de alguem (evita prender jogador dentro da colisao)
	TArray<FOverlapResult> Hits;
	FCollisionObjectQueryParams Obj; Obj.AddObjectTypesToQuery(ECC_Pawn);
	if (GetWorld()->OverlapMultiByObjectType(Hits, GetActorLocation(), GetActorQuat(), Obj, FCollisionShape::MakeBox(Size * 0.5f)))
		for (const FOverlapResult& H : Hits)
			if (Cast<APawn>(H.GetActor())) { OutWhy = TEXT("Tem alguem no lugar do objeto"); return false; }
	bDestroyed = false; bQuietChange = false;
	ApplyState(true);
	ForceNetUpdate();
	return true;
}

void ACoopDestructible::ServerForce(bool bBreak, bool bFx)
{
	if (!HasAuthority() || bDestroyed == bBreak) return;
	bDestroyed = bBreak; bQuietChange = !bFx;
	ApplyState(true);
	ForceNetUpdate();
}

void ACoopDestructible::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopDestructible, bDestroyed);
	DOREPLIFETIME(ACoopDestructible, bQuietChange);
	DOREPLIFETIME(ACoopDestructible, Partner);
	DOREPLIFETIME_CONDITION(ACoopDestructible, bAssemblyFx, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ACoopDestructible, Size, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ACoopDestructible, Color, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ACoopDestructible, Model, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ACoopDestructible, DebrisDrop, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ACoopDestructible, DebrisSpreadX, COND_InitialOnly);
}

// ---------- montagem visual na reconstrucao (so visual, local em cada maquina; colisao/estado ja valem na hora) ----------
void ACoopDestructible::TakeDebris(TArray<FVector>& Out)
{
	for (int32 i = 0; i < Frags.Num(); ++i)
		if (Frags[i] && Frags[i]->IsVisible() && !(FragBodies.IsValidIndex(i) && FragBodies[i] && FragBodies[i]->ComponentHasTag(TEXT("Toco"))))
		{
			Out.Add(Frags[i]->Bounds.Origin); Frags[i]->SetVisibility(false);
			if (FragBodies.IsValidIndex(i) && FragBodies[i]) { FragBodies[i]->SetSimulatePhysics(false); FragBodies[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision); }
		}
	for (UStaticMeshComponent* C : Chunks) if (C && C->IsVisible()) { Out.Add(C->GetComponentLocation()); C->SetVisibility(false); }
}

void ACoopDestructible::ClearAssembly()
{
	for (FAsm& A : Asm) if (A.C) A.C->DestroyComponent();
	Asm.Reset();
	if (AsmBeam) { AsmBeam->DestroyComponent(); AsmBeam = nullptr; }
	if (AsmLight) { AsmLight->DestroyComponent(); AsmLight = nullptr; }
	if (AsmT0 >= 0.f) { AsmT0 = -1.f; SetActorTickEnabled(false); }
}

void ACoopDestructible::StartAssembly()
{
	TArray<FVector> Src;
	if (Partner) Partner->TakeDebris(Src);
	const FVector Ctr = GetActorLocation(); const FQuat Q = GetActorQuat();
	bAsmHoriz = Size.X >= Size.Z * 2.f;
	if (Src.Num() == 0) for (int32 i = 0; i < 8; ++i) Src.Add(Ctr + FVector(FMath::Cos(i * 0.8f) * 500.f, FMath::Sin(i * 0.8f) * 500.f, -Size.Z * 0.5f - 200.f));
	UStaticMesh* Cube = JJ::Mesh(TEXT("Cube"));
	// ponte: azul-ciano (conexao horizontal) | coluna: verde (sustentacao vertical)
	const FLinearColor Glow = bAsmHoriz ? FLinearColor(0.15f, 0.7f, 1.6f) : FLinearColor(0.3f, 1.6f, 0.45f);
	UMaterialInstanceDynamic* PM = JJ::Mat(this, Color * 0.6f, Glow * 2.5f, 0.55f, 0.3f);
	UMaterialInstanceDynamic* BM = JJ::Mat(this, FLinearColor::Black, Glow * 10.f, 1.f);
	const int32 N = bAsmHoriz ? FMath::Clamp(FMath::RoundToInt(Size.X / 140.f), 6, 12) : 7;
	AsmFlight = bAsmHoriz ? 0.42f : 0.5f;
	FRandomStream R(FMath::RoundToInt(Ctr.X + Ctr.Y));
	for (int32 i = 0; i < N; ++i)
	{
		FAsm A;
		if (bAsmHoriz)
		{
			A.To = Ctr + Q.RotateVector(FVector(-Size.X * 0.5f + (i + 0.5f) * Size.X / N, 0, 0));
			A.Scale = FVector(Size.X / N * 0.97f, Size.Y, Size.Z) / 100.f;
			A.Delay = 0.18f + 0.07f * i;     // monta da margem inicial para a final
		}
		else
		{
			A.To = Ctr + Q.RotateVector(FVector(0, 0, -Size.Z * 0.5f + (i + 0.5f) * Size.Z / N));
			A.Scale = FVector(Size.X * 0.9f, Size.Y * 0.9f, Size.Z / N * 0.94f) / 100.f;
			A.Delay = 0.12f + 0.1f * i;      // aneis empilhando de baixo para cima
		}
		A.From = Src[i % Src.Num()] + FVector(R.FRandRange(-40.f, 40.f), R.FRandRange(-40.f, 40.f), 0.f);
		A.C = NewObject<UStaticMeshComponent>(this);
		A.C->SetStaticMesh(Cube); A.C->SetMaterial(0, PM);
		A.C->SetCollisionEnabled(ECollisionEnabled::NoCollision); A.C->SetCanEverAffectNavigation(false);
		A.C->SetMobility(EComponentMobility::Movable); A.C->SetupAttachment(RootComponent); A.C->SetAbsolute(true, true, true);
		A.C->RegisterComponent(); A.C->SetWorldLocation(A.From); A.C->SetWorldScale3D(A.Scale * 0.4f); A.C->SetVisibility(false);
		Asm.Add(A);
	}
	AsmEnd = Asm.Last().Delay + AsmFlight + 0.22f;
	// feixe-guia: ponte = linha horizontal que atravessa o vao | coluna = pilar de energia subindo da base
	AsmBeam = NewObject<UStaticMeshComponent>(this);
	AsmBeam->SetStaticMesh(Cube); AsmBeam->SetMaterial(0, BM); AsmBeam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AsmBeam->SetCastShadow(false); AsmBeam->SetMobility(EComponentMobility::Movable); AsmBeam->SetupAttachment(RootComponent);
	AsmBeam->SetAbsolute(true, true, true); AsmBeam->RegisterComponent(); AsmBeam->SetWorldRotation(GetActorRotation()); AsmBeam->SetVisibility(false);
	AsmLight = NewObject<UPointLightComponent>(this);
	AsmLight->SetupAttachment(RootComponent); AsmLight->SetAbsolute(true, true, true); AsmLight->RegisterComponent();
	AsmLight->SetLightColor(Glow); AsmLight->SetAttenuationRadius(900.f); AsmLight->SetCastShadows(false); AsmLight->SetIntensity(0.f);
	if (ModelComp) ModelComp->SetVisibility(false);
	Mesh->SetVisibility(false, false);
	AsmT0 = GetWorld()->GetTimeSeconds();
	SetActorTickEnabled(true);
	TickAssembly();
}

void ACoopDestructible::Tick(float Dt)
{
	Super::Tick(Dt);
	if (AsmT0 >= 0.f) TickAssembly();
}

void ACoopDestructible::TickAssembly()
{
	const float T = GetWorld()->GetTimeSeconds() - AsmT0;
	const FVector Ctr = GetActorLocation(); const FQuat Q = GetActorQuat();
	if (T >= AsmEnd)
	{
		// encaixe final: o modelo real aparece no lugar das pecas de energia
		const bool bKeep = !bDestroyed;
		ClearAssembly();
		if (bKeep) { if (ModelComp) ModelComp->SetVisibility(true); else Mesh->SetVisibility(true, false); }
		return;
	}
	float LightI = 0.f; FVector LightP = Ctr;
	for (const FAsm& A : Asm)
	{
		const float t = T - A.Delay;
		if (t <= 0.f) { A.C->SetVisibility(false); continue; }
		A.C->SetVisibility(true);
		const float a = FMath::Clamp(t / AsmFlight, 0.f, 1.f), e = 1.f - FMath::Pow(1.f - a, 3.f);
		FVector L; FRotator Rr = GetActorRotation(); FVector S = A.Scale;
		if (bAsmHoriz)
		{
			// arco do destroco ate o encaixe, chegando por cima e "batendo" no lugar
			L = FMath::Lerp(A.From, A.To + FVector(0, 0, 60.f), e) + FVector(0, 0, FMath::Sin(a * PI) * 320.f);
			if (a >= 1.f) { const float k = FMath::Clamp((t - AsmFlight) / 0.12f, 0.f, 1.f); L = A.To + FVector(0, 0, 60.f * (1.f - k)); }
			Rr += FRotator((1.f - e) * 160.f, (1.f - e) * 90.f, (1.f - e) * 60.f);
			S = A.Scale * FMath::Lerp(0.4f, 1.f, e);
		}
		else
		{
			// sobe girando (parafusando) ate a altura do anel; pulso horizontal ao encaixar
			const FVector Under(A.To.X, A.To.Y, Ctr.Z - Size.Z * 0.5f - 40.f);
			L = a < 0.55f ? FMath::Lerp(A.From, Under, FMath::SmoothStep(0.f, 1.f, a / 0.55f)) + FVector(0, 0, FMath::Sin(a / 0.55f * PI) * 200.f)
			              : FMath::Lerp(Under, A.To, FMath::SmoothStep(0.f, 1.f, (a - 0.55f) / 0.45f));
			Rr += FRotator(0, (1.f - e) * 270.f, 0);
			const float p = a >= 1.f ? FMath::Clamp((t - AsmFlight) / 0.15f, 0.f, 1.f) : 0.f;
			S = A.Scale * FMath::Lerp(0.5f, 1.f, e);
			if (a >= 1.f) S = A.Scale * FVector(1.f + 0.35f * (1.f - p), 1.f + 0.35f * (1.f - p), 1.f);
		}
		A.C->SetWorldLocationAndRotation(L, Rr); A.C->SetWorldScale3D(S);
		if (a >= 1.f && t - AsmFlight < 0.12f) { LightI = 9000.f; LightP = A.To; }
	}
	// feixe-guia
	const float b = FMath::Clamp(T / 0.3f, 0.f, 1.f), fade = FMath::Clamp((AsmEnd - T) / 0.25f, 0.f, 1.f);
	if (bAsmHoriz)
	{
		const float Len = Size.X * b;
		AsmBeam->SetWorldLocation(Ctr + Q.RotateVector(FVector(-Size.X * 0.5f + Len * 0.5f, 0, Size.Z * 0.5f + 4.f)));
		AsmBeam->SetWorldScale3D(FVector(FMath::Max(Len, 1.f), 10.f * fade + 1.f, 4.f) / 100.f);
	}
	else
	{
		const float Hh = (Size.Z + 60.f) * b;
		AsmBeam->SetWorldLocation(Ctr + FVector(0, 0, -Size.Z * 0.5f + Hh * 0.5f));
		AsmBeam->SetWorldScale3D(FVector(14.f * fade + 2.f, 14.f * fade + 2.f, FMath::Max(Hh, 1.f)) / 100.f);
	}
	AsmBeam->SetVisibility(true);
	AsmLight->SetWorldLocation(LightP); AsmLight->SetIntensity(LightI + 2500.f * fade);
}
