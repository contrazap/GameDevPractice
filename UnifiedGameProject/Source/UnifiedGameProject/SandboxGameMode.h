#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SandboxGameMode.generated.h"

/**
 * Harness GameMode for the session flow. Everything stays engine-default (the
 * flying DefaultPawn is the cross-instance connectivity proof) except the
 * PlayerController, which carries the EOS session UI. Wire BP_SandboxGameMode
 * in Project Settings -> Maps & Modes.
 */
UCLASS()
class UNIFIEDGAMEPROJECT_API ASandboxGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASandboxGameMode();
};
