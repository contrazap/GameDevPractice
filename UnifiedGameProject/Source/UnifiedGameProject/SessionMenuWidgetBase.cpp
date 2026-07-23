#include "SessionMenuWidgetBase.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "EOSSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SessionPlayerController.h"

void USessionMenuWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	// BindWidget pointers are guaranteed live here - the WBP cannot compile
	// with any of them missing, which is the whole point of the contract.
	ResumeButton->OnClicked.AddDynamic(this, &USessionMenuWidgetBase::OnResumeClicked);
	LoginButton->OnClicked.AddDynamic(this, &USessionMenuWidgetBase::OnLoginClicked);
	HostButton->OnClicked.AddDynamic(this, &USessionMenuWidgetBase::OnHostClicked);
	JoinButton->OnClicked.AddDynamic(this, &USessionMenuWidgetBase::OnJoinClicked);
	LeaveButton->OnClicked.AddDynamic(this, &USessionMenuWidgetBase::OnLeaveClicked);
	MicButton->OnClicked.AddDynamic(this, &USessionMenuWidgetBase::OnMicClicked);
	SpeakerButton->OnClicked.AddDynamic(this, &USessionMenuWidgetBase::OnSpeakerClicked);
	QuitButton->OnClicked.AddDynamic(this, &USessionMenuWidgetBase::OnQuitClicked);

	if (UEOSSessionSubsystem* EOS = GetEOS())
	{
		EOS->OnMessage.AddDynamic(this, &USessionMenuWidgetBase::HandleSessionMessage);
	}
	StatusText->SetText(FText::FromString(TEXT("Ready.")));
}

void USessionMenuWidgetBase::NativeDestruct()
{
	// The subsystem outlives this widget across travels - unbind explicitly.
	if (UEOSSessionSubsystem* EOS = GetEOS())
	{
		EOS->OnMessage.RemoveDynamic(this, &USessionMenuWidgetBase::HandleSessionMessage);
	}
	Super::NativeDestruct();
}

void USessionMenuWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Poll-based guards - the same behaviour the test project ran as per-frame
	// Slate attribute lambdas (guide ch.10 §3).
	UEOSSessionSubsystem* EOS = GetEOS();
	const bool bLoggedIn = EOS && EOS->IsLoggedIn();
	const bool bInSession = EOS && EOS->IsInSession();
	const bool bVoiceActive = EOS && EOS->IsVoiceActive();

	// Logged out: always clickable (if the plugin is there at all).
	// Logged in: it reads "Epic Logout" and locks while in a session - leave first.
	LoginButton->SetIsEnabled(bLoggedIn ? !bInSession : EOS != nullptr);
	LoginButtonText->SetText(FText::FromString(bLoggedIn ? TEXT("Epic Logout") : TEXT("Epic Login")));

	HostButton->SetIsEnabled(bLoggedIn && !bInSession);
	JoinButton->SetIsEnabled(bLoggedIn && !bInSession);
	LeaveButton->SetIsEnabled(bInSession);

	MicButton->SetIsEnabled(bVoiceActive);
	SpeakerButton->SetIsEnabled(bVoiceActive);
	MicButtonText->SetText(FText::FromString((EOS && EOS->IsMicMuted()) ? TEXT("Mic: MUTED") : TEXT("Mic: ON")));
	SpeakerButtonText->SetText(FText::FromString((EOS && EOS->IsSpeakerMuted()) ? TEXT("Speaker: MUTED") : TEXT("Speaker: ON")));
}

void USessionMenuWidgetBase::OnResumeClicked()
{
	if (ASessionPlayerController* PC = GetSessionPC())
	{
		PC->SetMainMenuOpen(false);
	}
}

void USessionMenuWidgetBase::OnLoginClicked()
{
	// One slot, two actions - the label above flips with IsLoggedIn().
	ASessionPlayerController* PC = GetSessionPC();
	UEOSSessionSubsystem* EOS = GetEOS();
	if (PC && EOS)
	{
		if (EOS->IsLoggedIn())
		{
			PC->EOSLogout();
		}
		else
		{
			PC->EOSLogin();
		}
	}
}

void USessionMenuWidgetBase::OnHostClicked()
{
	if (ASessionPlayerController* PC = GetSessionPC())
	{
		PC->EOSHost();
	}
}

void USessionMenuWidgetBase::OnJoinClicked()
{
	if (ASessionPlayerController* PC = GetSessionPC())
	{
		PC->EOSJoin();
	}
}

void USessionMenuWidgetBase::OnLeaveClicked()
{
	if (ASessionPlayerController* PC = GetSessionPC())
	{
		PC->EOSLeave();
	}
}

void USessionMenuWidgetBase::OnMicClicked()
{
	if (ASessionPlayerController* PC = GetSessionPC())
	{
		PC->EOSMic();
	}
}

void USessionMenuWidgetBase::OnSpeakerClicked()
{
	if (ASessionPlayerController* PC = GetSessionPC())
	{
		PC->EOSSpeaker();
	}
}

void USessionMenuWidgetBase::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}

void USessionMenuWidgetBase::HandleSessionMessage(const FString& Message, FColor Color)
{
	StatusText->SetText(FText::FromString(Message));
	StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(Color)));
}

UEOSSessionSubsystem* USessionMenuWidgetBase::GetEOS() const
{
	const UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UEOSSessionSubsystem>() : nullptr;
}

ASessionPlayerController* USessionMenuWidgetBase::GetSessionPC() const
{
	return Cast<ASessionPlayerController>(GetOwningPlayer());
}
