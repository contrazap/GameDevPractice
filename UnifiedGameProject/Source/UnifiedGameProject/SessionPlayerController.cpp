#include "SessionPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "EOSSessionSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"
#include "SessionMenuWidgetBase.h"

void ASessionPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Menu + message sink are local-player concerns; on a listen server the
	// proxies for remote clients must not bind (double-painted messages) or
	// spawn UI.
	if (!IsLocalController())
	{
		return;
	}

	if (UEOSSessionSubsystem* EOS = GetEOS())
	{
		EOS->OnMessage.AddDynamic(this, &ASessionPlayerController::HandleSessionMessage);
	}

	if (SessionMenuWidgetClass)
	{
		SessionMenuWidget = CreateWidget<USessionMenuWidgetBase>(this, SessionMenuWidgetClass);
		if (SessionMenuWidget)
		{
			SessionMenuWidget->AddToViewport(10);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: SessionMenuWidgetClass not set - no menu; exec commands still work."), *GetName());
	}

	// The menu IS the harness UI right now - start open (login/host live there).
	SetMainMenuOpen(SessionMenuWidget != nullptr);
}

void ASessionPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// The subsystem outlives this controller across travels - unbind explicitly.
	if (UEOSSessionSubsystem* EOS = GetEOS())
	{
		EOS->OnMessage.RemoveDynamic(this, &ASessionPlayerController::HandleSessionMessage);
	}
	if (SessionMenuWidget)
	{
		SessionMenuWidget->RemoveFromParent();
		SessionMenuWidget = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void ASessionPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Direct key binds - no Input Mapping Context yet (that's P0.3's job).
	// Escape works in -game/packaged runs; the editor's Stop-PIE bind eats it
	// in PIE, so M is the PIE-safe toggle. Android uses the hardware Back.
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ASessionPlayerController::ToggleMainMenu);
		InputComponent->BindKey(EKeys::M, IE_Pressed, this, &ASessionPlayerController::ToggleMainMenu);
		InputComponent->BindKey(EKeys::Android_Back, IE_Pressed, this, &ASessionPlayerController::ToggleMainMenu);
	}
}

UEOSSessionSubsystem* ASessionPlayerController::GetEOS() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UEOSSessionSubsystem>() : nullptr;
}

void ASessionPlayerController::EOSLogin()
{
	if (UEOSSessionSubsystem* EOS = GetEOS())
	{
		EOS->Login();
	}
}

void ASessionPlayerController::EOSLogout()
{
	if (UEOSSessionSubsystem* EOS = GetEOS())
	{
		EOS->Logout();
	}
}

void ASessionPlayerController::EOSHost()
{
	if (UEOSSessionSubsystem* EOS = GetEOS())
	{
		EOS->HostSession(ListenMapUrl);
	}
}

void ASessionPlayerController::EOSJoin()
{
	if (UEOSSessionSubsystem* EOS = GetEOS())
	{
		EOS->FindAndJoinSession();
	}
}

void ASessionPlayerController::EOSLeave()
{
	if (UEOSSessionSubsystem* EOS = GetEOS())
	{
		EOS->LeaveSession();
	}
}

void ASessionPlayerController::EOSStatus()
{
	if (UEOSSessionSubsystem* EOS = GetEOS())
	{
		EOS->PrintStatus();
	}
}

void ASessionPlayerController::EOSMic()
{
	if (UEOSSessionSubsystem* EOS = GetEOS())
	{
		EOS->SetMicMuted(!EOS->IsMicMuted());
	}
}

void ASessionPlayerController::EOSSpeaker()
{
	if (UEOSSessionSubsystem* EOS = GetEOS())
	{
		EOS->SetSpeakerMuted(!EOS->IsSpeakerMuted());
	}
}

void ASessionPlayerController::ToggleMainMenu()
{
	SetMainMenuOpen(!bMainMenuOpen);
}

void ASessionPlayerController::SetMainMenuOpen(bool bOpen)
{
	// Never SetPause here - multiplayer; one client pausing must not touch anyone else.
	bMainMenuOpen = bOpen;

	if (SessionMenuWidget)
	{
		SessionMenuWidget->SetVisibility(bOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (bOpen)
	{
		// GameAndUI (not UIOnly) keeps the key binds alive so Escape/M/Back can
		// toggle the menu shut again.
		bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		// Back to gameplay: capture the mouse so camera look is direct.
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
	}
}

void ASessionPlayerController::HandleSessionMessage(const FString& Message, FColor Color)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 8.f, Color, Message);
	}
}
