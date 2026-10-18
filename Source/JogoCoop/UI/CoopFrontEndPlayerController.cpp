#include "CoopFrontEndPlayerController.h"
#include "CoopMainMenuWidget.h"
#include "Blueprint/UserWidget.h"

void ACoopFrontEndPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalController()) return;
	MenuWidget = CreateWidget<UCoopMainMenuWidget>(this, UCoopMainMenuWidget::StaticClass());
	if (MenuWidget) MenuWidget->AddToViewport(0);
}
