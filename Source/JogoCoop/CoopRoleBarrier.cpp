#include "CoopRoleBarrier.h"
#include "CoopCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

ACoopRoleBarrier::ACoopRoleBarrier()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.2f;   // reavalia os personagens 5x/s (cobre troca de papel e jogadores que entram depois)
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Malha"));
	SetRootComponent(Mesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Mesh->SetStaticMesh(Cube.Object);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_WorldStatic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);        // nao atrapalha mira, camera, fisica de fragmentos
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);   // so bloqueia personagens
	Mesh->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/Fase2/Materiais/M_F2_Barreira.M_F2_Barreira"));
	if (Mat.Succeeded()) Mesh->SetMaterial(0, Mat.Object);
}

void ACoopRoleBarrier::ApplyLook()
{
	Mesh->SetRelativeScale3D(Size / 100.f);
	UMaterialInterface* Cur = Mesh->GetMaterial(0);
	if (!Cur || !Cur->GetName().Contains(TEXT("Barreira")))   // garante o material translucido mesmo se o ator foi salvo antes dele existir
		if (UMaterialInterface* B = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Fase2/Materiais/M_F2_Barreira.M_F2_Barreira"))) Mesh->SetMaterial(0, B);
	FLinearColor C = Tint;
	if (bAutoTint) C = (AllowedRole == ECoopBarrierRole::Ruina) ? FLinearColor(0.65f, 0.15f, 1.f) : (AllowedRole == ECoopBarrierRole::Neon) ? FLinearColor(0.05f, 0.85f, 1.f) : FLinearColor(1.f, 0.5f, 0.05f);
	if (UMaterialInterface* M = Mesh->GetMaterial(0))
	{
		Mid = Mesh->CreateDynamicMaterialInstance(0, M);
		if (Mid) { Mid->SetVectorParameterValue(TEXT("Cor"), C); Mid->SetScalarParameterValue(TEXT("Opac"), Opacity); }
	}
}

void ACoopRoleBarrier::OnConstruction(const FTransform& T) { Super::OnConstruction(T); ApplyLook(); }
void ACoopRoleBarrier::BeginPlay() { Super::BeginPlay(); ApplyLook(); }

void ACoopRoleBarrier::Tick(float Dt)
{
	Super::Tick(Dt);
	if (!GetWorld() || !GetWorld()->IsGameWorld()) return;
	for (TActorIterator<ACoopCharacter> It(GetWorld()); It; ++It)
	{
		const bool bAllowed = (uint8)It->CoopRole == (uint8)AllowedRole;
		if (UCapsuleComponent* Cap = It->GetCapsuleComponent()) Cap->IgnoreActorWhenMoving(this, bAllowed);
	}
}
