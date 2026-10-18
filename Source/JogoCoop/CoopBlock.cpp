#include "CoopBlock.h"
#include "JJVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/UObjectGlobals.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ACoopBlock::ACoopBlock()
{
	bReplicates = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded()) Mesh->SetStaticMesh(Cube.Object);
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void ACoopBlock::Setup(FVector S, FLinearColor C, bool bInGhost, FLinearColor E, float R, float M)
{
	Size = S; Color = C; bGhost = bInGhost; Emissive = E; Rough = R; Metal = M;
	OnRep_Setup();
}

void ACoopBlock::OnRep_Setup()
{
	Mesh->SetWorldScale3D(Size / 100.f);
	Mesh->SetVisibility(!bGhost);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, bGhost ? ECR_Ignore : ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, bGhost ? ECR_Ignore : ECR_Block);
	if (!bGhost) Mesh->SetMaterial(0, JJ::Mat(this, Color, Emissive, Rough, Metal));
	if (bNoCollide) Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Model != NAME_None)
	{
		const FString P = FString::Printf(TEXT("/Game/Fase1/%s/SM_%s.SM_%s"), *Model.ToString(), *Model.ToString(), *Model.ToString());
		if (UStaticMesh* SM = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *P, nullptr, LOAD_Quiet | LOAD_NoWarn)))
		{
			if (!ModelComp)
			{
				ModelComp = NewObject<UStaticMeshComponent>(this);
				ModelComp->SetupAttachment(Mesh);
				ModelComp->SetAbsolute(false, false, true); // escala absoluta: o modelo ja vem em cm reais
				ModelComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				ModelComp->RegisterComponent();
			}
			ModelComp->SetStaticMesh(SM);
			ModelComp->SetWorldScale3D(FVector(1.f));
			ModelComp->SetWorldLocationAndRotation(GetActorLocation() - FVector(0, 0, Size.Z * 0.5f), GetActorRotation());
			Mesh->SetVisibility(false);
		}
	}
}

void ACoopBlock::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACoopBlock, Size);
	DOREPLIFETIME(ACoopBlock, Color);
	DOREPLIFETIME(ACoopBlock, Emissive);
	DOREPLIFETIME(ACoopBlock, Rough);
	DOREPLIFETIME(ACoopBlock, Metal);
	DOREPLIFETIME(ACoopBlock, bGhost);
	DOREPLIFETIME(ACoopBlock, Model);
	DOREPLIFETIME(ACoopBlock, bNoCollide);
}
