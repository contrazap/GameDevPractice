// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MultiplayerCoursePlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class SWeakWidget;
class SWidget;
class UEOSSessionSubsystem;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AMultiplayerCoursePlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/** EOS on-screen actions. Also exposed as Exec so they work from the PC console. */
	UFUNCTION(Exec, BlueprintCallable, Category = "EOS") void EOSLogin();
	UFUNCTION(Exec, BlueprintCallable, Category = "EOS") void EOSLogout();
	UFUNCTION(Exec, BlueprintCallable, Category = "EOS") void EOSHost();
	UFUNCTION(Exec, BlueprintCallable, Category = "EOS") void EOSJoin();
	UFUNCTION(Exec, BlueprintCallable, Category = "EOS") void EOSStatus();
	UFUNCTION(Exec, BlueprintCallable, Category = "EOS") void EOSLeave();

	/** Opens/closes the (non-pausing) main menu overlay. Bound to Escape / Android Back. */
	void ToggleMainMenu();
	void SetMainMenuOpen(bool bOpen);

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Tears down the EOS menu widget */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

private:

	/** Resolves the EOS session subsystem from the game instance (may be null) */
	UEOSSessionSubsystem* GetEOS() const;

	/** Slate overlay: centered main menu + top-right menu toggle + voice HUD */
	TSharedPtr<SWidget> MenuWidget;
	TSharedPtr<SWeakWidget> MenuWidgetContainer;

	/** Whether the centered main menu is currently shown. */
	bool bMainMenuOpen = false;

};
