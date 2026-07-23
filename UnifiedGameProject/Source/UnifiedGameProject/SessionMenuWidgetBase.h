#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionMenuWidgetBase.generated.h"

class UButton;
class UTextBlock;
class UEOSSessionSubsystem;
class ASessionPlayerController;

/**
 * C++ half of WBP_SessionMenu - logic here, layout in UMG (CONVENTIONS rule 6).
 * BindWidget contract: the WBP must contain widgets of these exact names and
 * types or it refuses to compile; that error is the contract working.
 * State is polled in NativeTick (the plugin has no state-change delegates by
 * design - P0.2 cut line 1). Actions route through the PlayerController so the
 * console commands and the menu share one code path.
 */
UCLASS(Abstract)
class UNIFIEDGAMEPROJECT_API USessionMenuWidgetBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// --- BindWidget contract: the names + types the WBP must supply. ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LoginButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LeaveButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MicButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SpeakerButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

	// Flip labels (runtime-driven). Static labels (Resume/Host/...) stay UMG-only.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LoginButtonText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MicButtonText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeakerButtonText;

	/** Shows the last OnMessage line, in the message's color. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

private:
	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnLoginClicked();

	UFUNCTION()
	void OnHostClicked();

	UFUNCTION()
	void OnJoinClicked();

	UFUNCTION()
	void OnLeaveClicked();

	UFUNCTION()
	void OnMicClicked();

	UFUNCTION()
	void OnSpeakerClicked();

	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void HandleSessionMessage(const FString& Message, FColor Color);

	UEOSSessionSubsystem* GetEOS() const;
	ASessionPlayerController* GetSessionPC() const;
};
