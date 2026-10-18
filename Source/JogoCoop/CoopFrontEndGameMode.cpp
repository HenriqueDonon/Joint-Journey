#include "CoopFrontEndGameMode.h"
#include "UI/CoopFrontEndPlayerController.h"

ACoopFrontEndGameMode::ACoopFrontEndGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ACoopFrontEndPlayerController::StaticClass();
}
