#include "CoopTunnelDressing.h"
#include "JJVisual.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

static UMaterialInterface* TM(const TCHAR* P) { return LoadObject<UMaterialInterface>(nullptr, P); }

ACoopTunnelDressing::ACoopTunnelDressing()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Raiz"));
}

void ACoopTunnelDressing::Box(UStaticMesh* M, UMaterialInterface* Mat, FVector C, FVector S, FRotator R, bool bShadow)
{
	UStaticMeshComponent* Q = NewObject<UStaticMeshComponent>(this);
	Q->SetupAttachment(RootComponent); Q->SetMobility(EComponentMobility::Movable);
	Q->SetStaticMesh(M); if (Mat) Q->SetMaterial(0, Mat);
	Q->SetCollisionEnabled(ECollisionEnabled::NoCollision); Q->SetCanEverAffectNavigation(false); Q->SetCastShadow(bShadow);
	Q->RegisterComponent();
	Q->SetWorldLocationAndRotation(GetActorLocation() + C, R); Q->SetWorldScale3D(S / 100.f);
}

void ACoopTunnelDressing::BeginPlay()
{
	Super::BeginPlay();
	UStaticMesh* Cube = JJ::Mesh(TEXT("Cube")); UStaticMesh* Cyl = JJ::Mesh(TEXT("Cylinder"));
	UMaterialInterface *Steel = TM(TEXT("/Game/Fase1Arq/HF/M_steel_dark.M_steel_dark")), *Paint = TM(TEXT("/Game/Fase1Arq/HF/M_steel_paint.M_steel_paint")),
		*Conc = TM(TEXT("/Game/Fase1Arq/HF/M_concrete_dark.M_concrete_dark")), *Haz = TM(TEXT("/Game/Fase1Arq/HF/M_hazard_yellow.M_hazard_yellow")),
		*Ember = TM(TEXT("/Game/Fase1Arq/HF/M_ember.M_ember")), *Cable = TM(TEXT("/Game/Fase1Arq/HF/M_cable.M_cable"));
	UMaterialInterface* Teal = JJ::Mat(this, FLinearColor(0.02f, 0.08f, 0.1f), FLinearColor(0.1f, 0.75f, 1.f) * 5.f, 0.4f);
	UMaterialInterface* Cool = JJ::Mat(this, FLinearColor(0.8f, 0.85f, 0.9f), FLinearColor(0.72f, 0.86f, 1.f) * 5.f, 0.5f);
	const float X1 = X0 + Len, CX = (X0 + X1) * 0.5f, W = HalfW;
	// piso: concreto escuro + faixas de perigo nas bordas
	Box(Cube, Conc, FVector(CX, 0, 1.5f), FVector(Len, 2 * W, 3));
	for (float s : { -1.f, 1.f }) Box(Cube, Haz, FVector(CX, s * (W - 70.f), 3.5f), FVector(Len - 40.f, 24, 2), FRotator::ZeroRotator, false);
	for (float s : { -1.f, 1.f }) Box(Cube, Teal, FVector(CX, s * 150.f, 3.5f), FVector(Len - 100.f, 8, 2), FRotator::ZeroRotator, false);   // guias ate o portal
	// paredes: chapas de aco, rodape pintado, nervuras verticais, faixa ciano (mesma altura/linguagem do galpao)
	for (float s : { -1.f, 1.f })
	{
		Box(Cube, Steel, FVector(CX, s * (W - 4.f), H * 0.5f), FVector(Len, 8, H));
		Box(Cube, Paint, FVector(CX, s * (W - 12.f), 45.f), FVector(Len, 10, 90));
		Box(Cube, Teal, FVector(CX, s * (W - 12.f), 270.f), FVector(Len, 4, 6), FRotator::ZeroRotator, false);
		for (float X = X0 + 140.f; X < X1; X += 285.f)
		{
			Box(Cube, Paint, FVector(X, s * (W - 16.f), H * 0.5f), FVector(26, 18, H));
			Box(Cube, Steel, FVector(X, s * (W - 40.f), H - 30.f), FVector(22, 60, 22), FRotator(0, 0, s * 35.f));   // mao-francesa ate o teto
		}
		// consoles na parede (tela ciano)
		for (float X : { X0 + 520.f, X0 + 1180.f })
		{
			Box(Cube, Paint, FVector(X, s * (W - 40.f), 120.f), FVector(120, 50, 90));
			Box(Cube, Teal, FVector(X, s * (W - 66.f), 135.f), FVector(80, 3, 40), FRotator::ZeroRotator, false);
		}
	}
	// teto: vigas transversais, canos longitudinais, bandeja de cabos
	for (float X = X0 + 140.f; X < X1; X += 285.f) Box(Cube, Steel, FVector(X, 0, H - 23.f), FVector(36, 2 * W, 46));
	for (float s : { -1.f, 1.f }) Box(Cyl, s > 0 ? Paint : Steel, FVector(CX, s * 290.f, H - 70.f), FVector(32, 32, Len), FRotator(90, 0, 0));
	Box(Cube, Cable, FVector(CX, -200.f, H - 60.f), FVector(Len, 60, 8));
	// moldura do portal (perigo + aco) e balizas ambar; moldura de entrada
	for (float s : { -1.f, 1.f })
	{
		Box(Cube, Steel, FVector(X1 - 30.f, s * 230.f, 190.f), FVector(60, 50, 380));
		Box(Cube, Haz, FVector(X1 - 62.f, s * 230.f, 190.f), FVector(4, 50, 380), FRotator::ZeroRotator, false);
		Box(Cube, Ember, FVector(X1 - 66.f, s * 230.f, 395.f), FVector(10, 20, 14), FRotator::ZeroRotator, false);
		Box(Cube, Haz, FVector(X0 + 30.f, s * 160.f, 150.f), FVector(10, 24, 300), FRotator::ZeroRotator, false);
	}
	// moldura da porta de entrada (abertura Y -69..57, 286 de altura)
	for (float y : { -96.f, 84.f }) Box(Cube, Steel, FVector(X0 + 20.f, y, 150.f), FVector(40, 50, 300));
	Box(Cube, Steel, FVector(X0 + 20.f, -6.f, 305.f), FVector(40, 230, 30));
	Box(Cube, Teal, FVector(X0 + 42.f, -6.f, 296.f), FVector(3, 150, 5), FRotator::ZeroRotator, false);
	Box(Cube, Steel, FVector(X1 - 30.f, 0, 400.f), FVector(60, 510, 40));
	Box(Cube, Haz, FVector(X1 - 62.f, 0, 400.f), FVector(4, 510, 14), FRotator::ZeroRotator, false);
	// luzes: mesma familia do galpao (fria azulada) + acentos ambar no portal
	for (float X : { X0 + 300.f, CX, X1 - 380.f })
	{
		Box(Cube, Cool, FVector(X, 0, H - 52.f), FVector(160, 40, 6), FRotator::ZeroRotator, false);
		UPointLightComponent* L = NewObject<UPointLightComponent>(this); L->SetupAttachment(RootComponent); L->RegisterComponent();
		L->SetWorldLocation(GetActorLocation() + FVector(X, 0, H - 80.f)); L->SetLightColor(FLinearColor(0.72f, 0.86f, 1.f));
		L->SetIntensityUnits(ELightUnits::Lumens); L->SetIntensity(4500.f); L->SetAttenuationRadius(1000.f); L->SetCastShadows(false);
	}
	for (float s : { -1.f, 1.f })
	{
		UPointLightComponent* L = NewObject<UPointLightComponent>(this); L->SetupAttachment(RootComponent); L->RegisterComponent();
		L->SetWorldLocation(GetActorLocation() + FVector(X1 - 120.f, s * 230.f, 380.f)); L->SetLightColor(FLinearColor(1.f, 0.45f, 0.08f));
		L->SetIntensityUnits(ELightUnits::Lumens); L->SetIntensity(1500.f); L->SetAttenuationRadius(500.f); L->SetCastShadows(false);
	}
}
