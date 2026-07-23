#include "SandboxGameMode.h"

#include "SessionPlayerController.h"

ASandboxGameMode::ASandboxGameMode()
{
	// AGameModeBase already supplies ADefaultPawn and the spectator defaults;
	// the controller is the only piece this harness replaces. The BP subclass
	// overrides this with BP_SessionPlayerController (which holds the widget ref).
	PlayerControllerClass = ASessionPlayerController::StaticClass();
}
