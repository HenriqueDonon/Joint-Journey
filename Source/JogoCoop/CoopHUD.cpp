#include "CoopHUD.h"
#include "CoopCharacter.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

void ACoopHUD::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
}

void ACoopHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas) return;
	const float cx = Canvas->SizeX * 0.5f, cy = Canvas->SizeY * 0.5f;
	const ACoopCharacter* C = Cast<ACoopCharacter>(GetOwningPawn());
	const bool bPortal = C && C->bPortalMode;
	const bool bOn = C && !bPortal && C->IsAimOnProp();
	const FLinearColor Col = bOn ? FLinearColor(0.1f, 1.f, 0.3f) : FLinearColor(1.f, 1.f, 1.f, 0.9f);
	const float Gap = 6.f, Len = 11.f, Th = 2.f;
	DrawLine(cx - Gap - Len, cy, cx - Gap, cy, Col, Th);
	DrawLine(cx + Gap, cy, cx + Gap + Len, cy, Col, Th);
	DrawLine(cx, cy - Gap - Len, cx, cy - Gap, Col, Th);
	DrawLine(cx, cy + Gap, cx, cy + Gap + Len, Col, Th);
	DrawRect(Col, cx - 1.5f, cy - 1.5f, 3.f, 3.f);
	if (bPortal)
	{
		const FLinearColor Blue(0.1f, 0.5f, 1.f), Orange(1.f, 0.45f, 0.05f);
		const float R = 22.f;
		for (int32 i = 0; i < 24; i++)
		{
			const float A0 = i * 2.f * PI / 24.f, A1 = (i + 1) * 2.f * PI / 24.f;
			const bool bLeft = FMath::Cos((A0 + A1) * 0.5f) < 0.f;
			DrawLine(cx + R * FMath::Cos(A0), cy + R * FMath::Sin(A0), cx + R * FMath::Cos(A1), cy + R * FMath::Sin(A1), bLeft ? Blue : Orange, 3.f);
		}
	}
	if (GEngine)
	{
		const FString T = bPortal ? TEXT("[R] ARMA DE PORTAIS  -  botao esq: portal azul | botao dir: portal laranja")
		                          : TEXT("[R] ARMA DE PODERES  -  botao esquerdo: atirar poder (Ruina destroi / Neon constroi)");
		DrawText(T, bPortal ? FLinearColor(0.4f, 0.8f, 1.f) : FLinearColor(0.8f, 1.f, 0.6f), cx - 280.f, Canvas->SizeY - 50.f, GEngine->GetMediumFont(), 1.f);
	}
}
