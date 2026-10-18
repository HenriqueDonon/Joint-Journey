#include "CoopDestructible.h"
#include "CoopBurst.h"
#include "JJVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

ACoopDestructible::ACoopDestructible()
{
	bReplicates = true;
	bAlwaysRelevant = true;
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
	BuildChunks();
	ApplyState(false);
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
	Mesh->SetVisibility(!bDestroyed, false);
	Mesh->SetCollisionEnabled(bDestroyed ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
	for (UStaticMeshComponent* C : Chunks) if (C) C->SetVisibility(bDestroyed);
	if (bWithFx && GetNetMode() != NM_DedicatedServer)
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
	bDestroyed = true;
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
	bDestroyed = false;
	ApplyState(true);
	ForceNetUpdate();
	return true;
}

void ACoopDestructible::ServerForce(bool bBreak)
{
	if (!HasAuthority() || bDestroyed == bBreak) return;
	bDestroyed = bBreak;
	ApplyState(true);
	ForceNetUpdate();
}

void ACoopDestructible::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopDestructible, bDestroyed);
	DOREPLIFETIME_CONDITION(ACoopDestructible, Size, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ACoopDestructible, Color, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ACoopDestructible, DebrisDrop, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ACoopDestructible, DebrisSpreadX, COND_InitialOnly);
}
