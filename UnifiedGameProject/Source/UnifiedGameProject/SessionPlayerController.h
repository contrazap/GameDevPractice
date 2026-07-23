#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SessionPlayerController.generated.h"

class UEOSSessionSubsystem;
class USessionMenuWidgetBase;

/**
 * Game-side facade over the OnlineCore plugin: an exec command per session
 * action (the debug path that works when UI breaks), the session menu widget,
 * and the cursor/input-mode swap. All plugin access goes through GetEOS() -
 * the game's only seam to OnlineCore.
 */
UCLASS()
class UNIFIEDGAMEPROJECT_API ASessionPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// --- Session actions. Console and menu both land here. ---
	UFUNCTION(Exec)
	void EOSLogin();

	UFUNCTION(Exec)
	void EOSLogout();

	/** Hosts on ListenMapUrl. The plugin travels the URL exactly as passed - ?listen included here. */
	UFUNCTION(Exec)
	void EOSHost();

	UFUNCTION(Exec)
	void EOSJoin();

	UFUNCTION(Exec)
	void EOSLeave();

	UFUNCTION(Exec)
	void EOSStatus();

	/** Toggles our microphone (device-level mute on our EOS user). */
	UFUNCTION(Exec)
	void EOSMic();

	/** Toggles incoming voice. */
	UFUNCTION(Exec)
	void EOSSpeaker();

	void ToggleMainMenu();
	void SetMainMenuOpen(bool bOpen);

	UEOSSessionSubsystem* GetEOS() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;

	/**
	 * Travel URL EOSHost() passes to the plugin. Caller's choice by design
	 * (P0.2 step 2 decision): the P1.11 map selector will pass its own URL per
	 * call; this default is the harness fallback. ?listen is mandatory -
	 * without it the host travels standalone and nobody can connect.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Session")
	FString ListenMapUrl = TEXT("/Game/UnifiedGameProject/Maps/L_Sandbox?listen");

	/** Set to WBP_SessionMenu in BP_SessionPlayerController. Unset = no menu; exec still works. */
	UPROPERTY(EditDefaultsOnly, Category = "Session")
	TSubclassOf<USessionMenuWidgetBase> SessionMenuWidgetClass;

private:
	/** OnMessage sink - the plugin never paints the screen; the game decides to, here. */
	UFUNCTION()
	void HandleSessionMessage(const FString& Message, FColor Color);

	UPROPERTY()
	TObjectPtr<USessionMenuWidgetBase> SessionMenuWidget;

	bool bMainMenuOpen = false;
};
