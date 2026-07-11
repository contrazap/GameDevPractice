// Copyright Epic Games, Inc. All Rights Reserved.


#include "MultiplayerCoursePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerCourse.h"
#include "Widgets/Input/SVirtualJoystick.h"

#include "EOSSessionSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "InputCoreTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#if PLATFORM_ANDROID
#include "AndroidPermissionFunctionLibrary.h"
#endif

void AMultiplayerCoursePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogMultiplayerCourse, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}

	// Build the EOS menu overlay + voice HUD on the local player.
	if (IsLocalController() && GEngine && GEngine->GameViewport)
	{
#if PLATFORM_ANDROID
		// Voice chat needs the mic. Ask once at startup.
		if (!UAndroidPermissionFunctionLibrary::CheckPermission(TEXT("android.permission.RECORD_AUDIO")))
		{
			UAndroidPermissionFunctionLibrary::AcquirePermissions({ TEXT("android.permission.RECORD_AUDIO") });
		}
#endif

		TWeakObjectPtr<AMultiplayerCoursePlayerController> WeakThis(this);

		// A sized button with dynamic text + enabled state (evaluated live by Slate).
		auto MakeButton = [](float Width, float Height, int32 FontSize,
			TAttribute<FText> Text, TFunction<void()> Action, TAttribute<bool> Enabled)
		{
			return SNew(SBox)
				.WidthOverride(Width)
				.HeightOverride(Height)
				.Padding(FMargin(4.f, 6.f))
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(10.f, 6.f))
					.IsEnabled(Enabled)
					.OnClicked_Lambda([Action = MoveTemp(Action)]()
					{
						Action();
						return FReply::Handled();
					})
					[
						SNew(STextBlock)
						.Text(Text)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", FontSize))
						.ColorAndOpacity(FLinearColor::Black)
					]
				];
		};

		// --- live state attributes (safe against controller destruction) ---
		auto LoggedIn = [WeakThis]() { UEOSSessionSubsystem* E = WeakThis.IsValid() ? WeakThis->GetEOS() : nullptr; return E && E->IsLoggedIn(); };
		auto InSession = [WeakThis]() { UEOSSessionSubsystem* E = WeakThis.IsValid() ? WeakThis->GetEOS() : nullptr; return E && E->IsInSession(); };
		auto VoiceOn = [WeakThis]() { UEOSSessionSubsystem* E = WeakThis.IsValid() ? WeakThis->GetEOS() : nullptr; return E && E->IsVoiceActive(); };

		auto MicText = [WeakThis]() { UEOSSessionSubsystem* E = WeakThis.IsValid() ? WeakThis->GetEOS() : nullptr; return FText::FromString((E && E->IsMicMuted()) ? TEXT("Mic: MUTED") : TEXT("Mic: ON")); };
		auto SpeakerText = [WeakThis]() { UEOSSessionSubsystem* E = WeakThis.IsValid() ? WeakThis->GetEOS() : nullptr; return FText::FromString((E && E->IsSpeakerMuted()) ? TEXT("Speaker: MUTED") : TEXT("Speaker: ON")); };
		auto ToggleMic = [WeakThis]() { UEOSSessionSubsystem* E = WeakThis.IsValid() ? WeakThis->GetEOS() : nullptr; if (E) { E->SetMicMuted(!E->IsMicMuted()); } };
		auto ToggleSpeaker = [WeakThis]() { UEOSSessionSubsystem* E = WeakThis.IsValid() ? WeakThis->GetEOS() : nullptr; if (E) { E->SetSpeakerMuted(!E->IsSpeakerMuted()); } };

		// Centered main-menu buttons (large for mobile).
		const float BtnW = 420.f;
		const float BtnH = 84.f;
		const int32 BtnFont = 22;

		TSharedRef<SVerticalBox> MenuButtons = SNew(SVerticalBox);
		MenuButtons->AddSlot().AutoHeight()
		[
			MakeButton(BtnW, BtnH, BtnFont, FText::FromString(TEXT("Resume")),
				[WeakThis]() { if (WeakThis.IsValid()) { WeakThis->SetMainMenuOpen(false); } },
				TAttribute<bool>(true))
		];
		// Login / Logout share one slot: label + action flip on login state. Logout is
		// blocked while in a session (leave first) - same guard style as Host/Join.
		MenuButtons->AddSlot().AutoHeight()
		[
			MakeButton(BtnW, BtnH, BtnFont,
				TAttribute<FText>::CreateLambda([LoggedIn]() { return FText::FromString(LoggedIn() ? TEXT("Epic Logout") : TEXT("Epic Login")); }),
				[WeakThis, LoggedIn]() { if (WeakThis.IsValid()) { if (LoggedIn()) { WeakThis->EOSLogout(); } else { WeakThis->EOSLogin(); } } },
				TAttribute<bool>::CreateLambda([LoggedIn, InSession]() { return LoggedIn() ? !InSession() : true; }))
		];
		MenuButtons->AddSlot().AutoHeight()
		[
			MakeButton(BtnW, BtnH, BtnFont, FText::FromString(TEXT("Host")),
				[WeakThis]() { if (WeakThis.IsValid()) { WeakThis->EOSHost(); } },
				TAttribute<bool>::CreateLambda([LoggedIn, InSession]() { return LoggedIn() && !InSession(); }))
		];
		MenuButtons->AddSlot().AutoHeight()
		[
			MakeButton(BtnW, BtnH, BtnFont, FText::FromString(TEXT("Join")),
				[WeakThis]() { if (WeakThis.IsValid()) { WeakThis->EOSJoin(); } },
				TAttribute<bool>::CreateLambda([LoggedIn, InSession]() { return LoggedIn() && !InSession(); }))
		];
		MenuButtons->AddSlot().AutoHeight()
		[
			MakeButton(BtnW, BtnH, BtnFont, FText::FromString(TEXT("Leave Session")),
				[WeakThis]() { if (WeakThis.IsValid()) { WeakThis->EOSLeave(); } },
				TAttribute<bool>::CreateLambda([InSession]() { return InSession(); }))
		];
		MenuButtons->AddSlot().AutoHeight()
		[
			MakeButton(BtnW, BtnH, BtnFont, TAttribute<FText>::CreateLambda(MicText),
				ToggleMic,
				TAttribute<bool>::CreateLambda([VoiceOn]() { return VoiceOn(); }))
		];
		MenuButtons->AddSlot().AutoHeight()
		[
			MakeButton(BtnW, BtnH, BtnFont, TAttribute<FText>::CreateLambda(SpeakerText),
				ToggleSpeaker,
				TAttribute<bool>::CreateLambda([VoiceOn]() { return VoiceOn(); }))
		];
		MenuButtons->AddSlot().AutoHeight()
		[
			MakeButton(BtnW, BtnH, BtnFont, FText::FromString(TEXT("Quit")),
				[WeakThis]() { if (WeakThis.IsValid()) { UKismetSystemLibrary::QuitGame(WeakThis->GetWorld(), WeakThis.Get(), EQuitPreference::Quit, false); } },
				TAttribute<bool>(true))
		];

		// Compact voice HUD (top-right) - only visible while voice is active.
		TSharedRef<SVerticalBox> VoiceHud = SNew(SVerticalBox);
		VoiceHud->AddSlot().AutoHeight().HAlign(HAlign_Right)
		[
			MakeButton(200.f, 48.f, 16, TAttribute<FText>::CreateLambda(MicText),
				ToggleMic, TAttribute<bool>(true))
		];
		VoiceHud->AddSlot().AutoHeight().HAlign(HAlign_Right)
		[
			MakeButton(200.f, 48.f, 16, TAttribute<FText>::CreateLambda(SpeakerText),
				ToggleSpeaker, TAttribute<bool>(true))
		];

		// Platform-specific content for the menu toggle button. Desktop shows an "Esc" hint
		// (it's mostly a hint while playing, since the cursor is hidden then); Android shows a
		// drawn hamburger - three bars - so it never depends on a font having the glyph.
		TSharedRef<SWidget> ToggleContent =
#if PLATFORM_ANDROID
			SNew(SBox).WidthOverride(30.f).HeightOverride(24.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
				[
					SNew(SBox).HeightOverride(4.f)
					[ SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush")).ColorAndOpacity(FLinearColor::Black) ]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
				[
					SNew(SBox).HeightOverride(4.f)
					[ SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush")).ColorAndOpacity(FLinearColor::Black) ]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f)
				[
					SNew(SBox).HeightOverride(4.f)
					[ SNew(SImage).Image(FCoreStyle::Get().GetBrush("WhiteBrush")).ColorAndOpacity(FLinearColor::Black) ]
				]
			];
#else
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Menu (Esc)")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			.ColorAndOpacity(FLinearColor::Black);
#endif

		MenuWidget =
			SNew(SOverlay)

			// Voice HUD - top-right, below the Menu button.
			+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(FMargin(0.f, 76.f, 16.f, 0.f))
			[
				SNew(SBox)
				.Visibility(TAttribute<EVisibility>::CreateLambda([VoiceOn]()
				{
					return VoiceOn() ? EVisibility::Visible : EVisibility::Collapsed;
				}))
				[
					VoiceHud
				]
			]

			// Menu open/close toggle - top-right corner. Also reachable via Escape / Android Back.
			+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(FMargin(0.f, 16.f, 16.f, 0.f))
			[
				SNew(SBox).WidthOverride(190.f).HeightOverride(48.f).Padding(FMargin(4.f, 6.f))
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(10.f, 6.f))
					.OnClicked_Lambda([WeakThis]() { if (WeakThis.IsValid()) { WeakThis->ToggleMainMenu(); } return FReply::Handled(); })
					[
						ToggleContent
					]
				]
			]

			// Centered main menu - dim backdrop + button column, collapsed when closed.
			+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.6f))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Visibility(TAttribute<EVisibility>::CreateLambda([WeakThis]()
				{
					return (WeakThis.IsValid() && WeakThis->bMainMenuOpen) ? EVisibility::Visible : EVisibility::Collapsed;
				}))
				[
					MenuButtons
				]
			];

		MenuWidgetContainer = SNew(SWeakWidget).PossiblyNullContent(MenuWidget.ToSharedRef());
		GEngine->GameViewport->AddViewportWidgetContent(MenuWidgetContainer.ToSharedRef(), 10);

		// Start closed: SetMainMenuOpen() hides the cursor and captures the mouse so camera
		// look works directly (no click-drag). It owns the cursor/input-mode swap from here.
		SetMainMenuOpen(false);
	}
}

void AMultiplayerCoursePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MenuWidgetContainer.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidgetContainer.ToSharedRef());
	}
	MenuWidget.Reset();
	MenuWidgetContainer.Reset();
	Super::EndPlay(EndPlayReason);
}

void AMultiplayerCoursePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}

		// Toggle the main menu with Escape (desktop) or the hardware Back button (Android).
		// These keys aren't mapped in any IMC, so a direct key bind is the simplest route.
		if (InputComponent)
		{
			InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AMultiplayerCoursePlayerController::ToggleMainMenu);
			InputComponent->BindKey(EKeys::Android_Back, IE_Pressed, this, &AMultiplayerCoursePlayerController::ToggleMainMenu);
		}
	}
}

bool AMultiplayerCoursePlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

UEOSSessionSubsystem* AMultiplayerCoursePlayerController::GetEOS() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UEOSSessionSubsystem>() : nullptr;
}

void AMultiplayerCoursePlayerController::EOSLogin()  { if (UEOSSessionSubsystem* EOS = GetEOS()) { EOS->Login(); } }
void AMultiplayerCoursePlayerController::EOSLogout() { if (UEOSSessionSubsystem* EOS = GetEOS()) { EOS->Logout(); } }
void AMultiplayerCoursePlayerController::EOSHost()   { if (UEOSSessionSubsystem* EOS = GetEOS()) { EOS->HostSession(); } }
void AMultiplayerCoursePlayerController::EOSJoin()   { if (UEOSSessionSubsystem* EOS = GetEOS()) { EOS->FindAndJoinSession(); } }
void AMultiplayerCoursePlayerController::EOSStatus() { if (UEOSSessionSubsystem* EOS = GetEOS()) { EOS->PrintStatus(); } }
void AMultiplayerCoursePlayerController::EOSLeave()  { if (UEOSSessionSubsystem* EOS = GetEOS()) { EOS->LeaveSession(); } }

void AMultiplayerCoursePlayerController::ToggleMainMenu()
{
	SetMainMenuOpen(!bMainMenuOpen);
}

void AMultiplayerCoursePlayerController::SetMainMenuOpen(bool bOpen)
{
	// Menu visibility is driven by bMainMenuOpen via Slate attribute lambdas. We deliberately
	// do NOT pause - other players stay live.
	bMainMenuOpen = bOpen;

	if (bOpen)
	{
		// Show the cursor so the menu buttons are clickable. Use GameAndUI (not UIOnly) so the
		// Escape / Back key bind still reaches us and can toggle the menu shut again.
		bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		// Back to gameplay: hide the cursor and capture the mouse so camera look is direct.
		// (On mobile, touch still reaches the on-screen buttons regardless of this mode.)
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
	}
}
