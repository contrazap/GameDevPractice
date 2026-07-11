# 04 — On-Screen Buttons + Player Controller

> **⚠️ Superseded by file 10 for the final UI.** This file builds a minimal always-on
> corner menu — enough to smoke-test EOS in PIE. The shipping project replaces it with a
> proper Escape/Back menu (state guards, Leave Session, voice mute HUD, direct mouse-look).
> Build this first to prove the plumbing, then see **`10_InGame_Menu_Voice_HUD.md`** for the
> real menu. If you only want the finished behaviour, skim this for the controller/GameMode
> wiring (4.3) and jump to 10.

**Goal:** Add touch-friendly Login / Host / Join / Status buttons and wire them to the subsystem — so you can drive EOS without typing console commands (critical on Android).

---

## Concept — why a custom PlayerController with Slate

Android has no keyboard-friendly console (a 4-finger tap opens it in non-shipping builds, but typing on a phone is miserable). So the controls need to be **on-screen buttons**. A custom `APlayerController` is the right place: it's created per local player, it can build a small **Slate** UI in `BeginPlay`, and it exposes `Exec` functions so the same actions also work from the PC console.

It also does one Android-only job: **request the microphone permission** at startup, so voice (file 09) can work.

Finally you'll point the GameMode at this controller so it's actually used.

---

## Steps

### 4.1 — Create `GTFOClonePlayerController.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GTFOClonePlayerController.generated.h"

class SWeakWidget;
class SWidget;
class UEOSSessionSubsystem;

UCLASS()
class GTFOCLONE_API AGTFOClonePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec, BlueprintCallable, Category = "EOS") void EOSLogin();
	UFUNCTION(Exec, BlueprintCallable, Category = "EOS") void EOSHost();
	UFUNCTION(Exec, BlueprintCallable, Category = "EOS") void EOSJoin();
	UFUNCTION(Exec, BlueprintCallable, Category = "EOS") void EOSStatus();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UEOSSessionSubsystem* GetEOS() const;

	TSharedPtr<SWidget> MenuWidget;
	TSharedPtr<SWeakWidget> MenuWidgetContainer;
};
```

### 4.2 — Create `GTFOClonePlayerController.cpp`

```cpp
#include "GTFOClonePlayerController.h"

#include "EOSSessionSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#if PLATFORM_ANDROID
#include "AndroidPermissionFunctionLibrary.h"
#endif

void AGTFOClonePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController() || !GEngine || !GEngine->GameViewport)
	{
		return;
	}

#if PLATFORM_ANDROID
	// Voice chat needs the mic. Ask once at startup.
	if (!UAndroidPermissionFunctionLibrary::CheckPermission(TEXT("android.permission.RECORD_AUDIO")))
	{
		UAndroidPermissionFunctionLibrary::AcquirePermissions({ TEXT("android.permission.RECORD_AUDIO") });
	}
#endif

	TWeakObjectPtr<AGTFOClonePlayerController> WeakThis(this);
	auto MakeButton = [](const TCHAR* Label, TFunction<void()> Action)
	{
		return SNew(SBox)
			.WidthOverride(280.f)
			.HeightOverride(64.f)
			.Padding(FMargin(0.f, 6.f))
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked_Lambda([Action = MoveTemp(Action)]()
				{
					Action();
					return FReply::Handled();
				})
				[
					SNew(STextBlock)
					.Text(FText::FromString(Label))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FLinearColor::Black)
				]
			];
	};

	MenuWidget =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(40.f, 80.f, 0.f, 0.f)
		[
			MakeButton(TEXT("1. Epic Login"), [WeakThis] { if (WeakThis.IsValid()) { WeakThis->EOSLogin(); } })
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(40.f, 0.f, 0.f, 0.f)
		[
			MakeButton(TEXT("2. Host"), [WeakThis] { if (WeakThis.IsValid()) { WeakThis->EOSHost(); } })
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(40.f, 0.f, 0.f, 0.f)
		[
			MakeButton(TEXT("3. Join"), [WeakThis] { if (WeakThis.IsValid()) { WeakThis->EOSJoin(); } })
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(40.f, 0.f, 0.f, 0.f)
		[
			MakeButton(TEXT("Status"), [WeakThis] { if (WeakThis.IsValid()) { WeakThis->EOSStatus(); } })
		];

	MenuWidgetContainer = SNew(SWeakWidget).PossiblyNullContent(MenuWidget.ToSharedRef());
	GEngine->GameViewport->AddViewportWidgetContent(MenuWidgetContainer.ToSharedRef(), 10);

	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AGTFOClonePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MenuWidgetContainer.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidgetContainer.ToSharedRef());
	}
	MenuWidget.Reset();
	MenuWidgetContainer.Reset();
	Super::EndPlay(EndPlayReason);
}

UEOSSessionSubsystem* AGTFOClonePlayerController::GetEOS() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UEOSSessionSubsystem>() : nullptr;
}

void AGTFOClonePlayerController::EOSLogin()  { if (UEOSSessionSubsystem* EOS = GetEOS()) { EOS->Login(); } }
void AGTFOClonePlayerController::EOSHost()   { if (UEOSSessionSubsystem* EOS = GetEOS()) { EOS->HostSession(); } }
void AGTFOClonePlayerController::EOSJoin()   { if (UEOSSessionSubsystem* EOS = GetEOS()) { EOS->FindAndJoinSession(); } }
void AGTFOClonePlayerController::EOSStatus() { if (UEOSSessionSubsystem* EOS = GetEOS()) { EOS->PrintStatus(); } }
```

### 4.3 — Wire the controller into the GameMode

In the template's `AGTFOCloneGameMode` **constructor**, add:

```cpp
PlayerControllerClass = AGTFOClonePlayerController::StaticClass();
```

(Include the header: `#include "GTFOClonePlayerController.h"`.) The template's Blueprint GameMode (`BP_ThirdPersonGameMode`) inherits this unless it explicitly overrides Player Controller Class — you'll verify that below.

### 4.4 — Rebuild

Same Build.bat command as file 03, or Live Coding if the editor's open.

---

## ✅ Verify before continuing

1. **Rebuild succeeds.**
2. Open **`BP_ThirdPersonGameMode`** → Class Defaults → **Player Controller Class** shows **`GTFOClonePlayerController`** (not the default). If it's overridden to something else, set it here.
3. Press **Play (PIE)**:
   - [ ] The four buttons — **1. Epic Login / 2. Host / 3. Join / Status** — appear top-left.
   - [ ] Clicking **Status** prints an on-screen `[EOS] OSS: EOS | Login: NotLoggedIn ...` line. **`OSS: EOS` is the key** — it proves file 02's config is live.
4. **(Optional login smoke test)** Click **1. Epic Login** — a browser/account-portal window should open. You don't have to complete it here (full login is tested in file 06 with the Dev Auth Tool), but the portal appearing confirms the identity path works.

> Note: plain PIE won't let you actually *host and join* between windows — one process shares a single EOS login. That's expected; real host/join is file 06.

Buttons visible + `OSS: EOS` → open **`05_Replication_Cube.md`**.

Buttons missing → GameMode isn't using your controller (step 2), or the widget wasn't added. See **`99_Troubleshooting.md`**.
