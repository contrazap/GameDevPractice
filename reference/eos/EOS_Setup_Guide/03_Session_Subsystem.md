# 03 — The Session Subsystem (login / host / join)

**Goal:** Add the C++ that logs into EOS and hosts/finds/joins a lobby — and get it to **compile**.

---

## Concept — why a GameInstance subsystem

Login and sessions must outlive any single map or player. A **`UGameInstanceSubsystem`** exists for exactly the lifetime of the game instance, so it's the natural home for this state. It exposes four Blueprint-callable actions — `Login`, `HostSession`, `FindAndJoinSession`, `PrintStatus` — which file 04's buttons will call.

How each action maps to the online interfaces:
- **`Login()`** → `IOnlineIdentity`. Detects `-AUTH_TYPE=` on the command line (the Dev Auth Tool path, file 06) and uses `AutoLogin`; otherwise opens the Epic **account portal** in a browser.
- **`HostSession()`** → `IOnlineSession::CreateSession` with lobby + voice flags, then `ServerTravel` to the map with `?listen`.
- **`FindAndJoinSession()`** → `FindSessions` filtered by a **bucket string** (must be identical on host and client), then `JoinSession`, then `ClientTravel` to the resolved EOS address.

The **bucket** (`GTFOClone_v1`) is how a client's search matches your host and ignores every other EOS game's lobbies. Keep it identical across all builds.

---

## Steps

### 3.1 — Add dependencies to `GTFOClone.Build.cs`

Inside the module's constructor:

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "OnlineBase",           // Online/OnlineSessionNames.h (SEARCH_LOBBIES)
    "OnlineSubsystem",
    "OnlineSubsystemUtils",
});
PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
if (Target.Platform == UnrealTargetPlatform.Android)
{
    PrivateDependencyModuleNames.Add("AndroidPermission"); // runtime mic permission
}
```

### 3.2 — Create `EOSSessionSubsystem.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EOSSessionSubsystem.generated.h"

/**
 * Minimal EOS login + lobby host/join flow.
 * Flow: Login() -> HostSession() (server) / FindAndJoinSession() (client).
 * Host travels to the game map with ?listen; clients ClientTravel to the
 * resolved EOS P2P address ("EOS:<PUID>:GameNetDriver:...").
 */
UCLASS()
class GTFOCLONE_API UEOSSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "EOS")
	void Login();

	UFUNCTION(BlueprintCallable, Category = "EOS")
	void HostSession();

	UFUNCTION(BlueprintCallable, Category = "EOS")
	void FindAndJoinSession();

	UFUNCTION(BlueprintCallable, Category = "EOS")
	void PrintStatus();

	bool IsLoggedIn() const;

private:
	void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	IOnlineIdentityPtr GetIdentity() const;
	IOnlineSessionPtr GetSessions() const;
	void ShowMsg(const FString& Msg, const FColor& Color = FColor::Cyan) const;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	FDelegateHandle LoginDelegateHandle;
	FDelegateHandle CreateSessionDelegateHandle;
	FDelegateHandle FindSessionsDelegateHandle;
	FDelegateHandle JoinSessionDelegateHandle;
	FDelegateHandle DestroySessionDelegateHandle;
};
```

### 3.3 — Create `EOSSessionSubsystem.cpp`

```cpp
#include "EOSSessionSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemUtils.h"

namespace
{
	// Must be identical in hosting and searching builds (and across platforms).
	const TCHAR* SessionBucket = TEXT("GTFOClone_v1");
	// Literal of OSSEOS_BUCKET_ID_ATTRIBUTE_KEY (OnlineSubsystemEOSTypesPublic.h);
	// used as FName to avoid a hard module dependency on OnlineSubsystemEOS.
	const TCHAR* BucketKeyName = TEXT("OSSEOS_BUCKET_ID_ATTRIBUTE_KEY");
	// Adjust to the actual game map. Third Person template default:
	const TCHAR* ListenTravelUrl = TEXT("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
}

void UEOSSessionSubsystem::Login()
{
	IOnlineIdentityPtr Identity = GetIdentity();
	if (!Identity.IsValid())
	{
		ShowMsg(TEXT("No identity interface. Is OnlineSubsystemEOS enabled/configured?"), FColor::Red);
		return;
	}
	if (Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
		ShowMsg(FString::Printf(TEXT("Already logged in as %s"), *Identity->GetPlayerNickname(0)), FColor::Green);
		return;
	}
	if (LoginDelegateHandle.IsValid())
	{
		ShowMsg(TEXT("Login already in progress..."));
		return;
	}

	LoginDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(
		0, FOnLoginCompleteDelegate::CreateUObject(this, &UEOSSessionSubsystem::HandleLoginComplete));

	FString AuthType;
	FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), AuthType);
	if (!AuthType.IsEmpty())
	{
		// Dev iteration path: -AUTH_TYPE=developer -AUTH_LOGIN=localhost:8081 -AUTH_PASSWORD=<DevAuthCredName>
		ShowMsg(FString::Printf(TEXT("AutoLogin via command line (-AUTH_TYPE=%s)..."), *AuthType));
		Identity->AutoLogin(0);
	}
	else
	{
		// Ships path: persistent auth if cached, otherwise Epic account portal (browser).
		ShowMsg(TEXT("Logging in (Epic account portal / cached auth)..."));
		FOnlineAccountCredentials Credentials;
		Credentials.Type = TEXT("accountportal");
		Identity->Login(0, Credentials);
	}
}

void UEOSSessionSubsystem::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (IOnlineIdentityPtr Identity = GetIdentity())
	{
		Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginDelegateHandle);
	}
	LoginDelegateHandle.Reset();

	if (bWasSuccessful)
	{
		ShowMsg(FString::Printf(TEXT("LOGIN OK: %s"), *GetIdentity()->GetPlayerNickname(LocalUserNum)), FColor::Green);
	}
	else
	{
		ShowMsg(FString::Printf(TEXT("LOGIN FAILED: %s"), *Error), FColor::Red);
	}
}

bool UEOSSessionSubsystem::IsLoggedIn() const
{
	IOnlineIdentityPtr Identity = GetIdentity();
	return Identity.IsValid() && Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn;
}

void UEOSSessionSubsystem::HostSession()
{
	if (!IsLoggedIn())
	{
		ShowMsg(TEXT("Not logged in - run EOSLogin first"), FColor::Red);
		return;
	}
	IOnlineSessionPtr Sessions = GetSessions();
	if (!Sessions.IsValid())
	{
		ShowMsg(TEXT("No session interface"), FColor::Red);
		return;
	}
	if (Sessions->GetNamedSession(NAME_GameSession) != nullptr)
	{
		ShowMsg(TEXT("Already in a session - destroying it. Run EOSHost again afterwards."), FColor::Yellow);
		DestroySessionDelegateHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &UEOSSessionSubsystem::HandleDestroySessionComplete));
		Sessions->DestroySession(NAME_GameSession);
		return;
	}

	FOnlineSessionSettings Settings;
	Settings.NumPublicConnections = 4;
	Settings.bIsDedicated = false;
	Settings.bIsLANMatch = false;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bAllowInvites = true;
	Settings.bUsesPresence = false;            // avoids needing the Presence scope/portal permission
	Settings.bAllowJoinViaPresence = false;
	Settings.bUseLobbiesIfAvailable = true;    // EOS lobbies: right fit for player-hosted co-op
	Settings.bUseLobbiesVoiceChatIfAvailable = true; // EOS RTC voice room, auto-joined (needs Voice in client policy)
	Settings.bUsesStats = false;
	Settings.Set(FName(BucketKeyName), FString(SessionBucket), EOnlineDataAdvertisementType::ViaOnlineService);

	CreateSessionDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UEOSSessionSubsystem::HandleCreateSessionComplete));

	ShowMsg(TEXT("Creating session..."));
	if (!Sessions->CreateSession(0, NAME_GameSession, Settings))
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
		CreateSessionDelegateHandle.Reset();
		ShowMsg(TEXT("CreateSession call failed immediately"), FColor::Red);
	}
}

void UEOSSessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (IOnlineSessionPtr Sessions = GetSessions())
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
	}
	CreateSessionDelegateHandle.Reset();

	if (!bWasSuccessful)
	{
		ShowMsg(TEXT("Session creation FAILED (check LogEOSSDK / client policy)"), FColor::Red);
		return;
	}
	ShowMsg(TEXT("Session created - starting listen server..."), FColor::Green);
	GetGameInstance()->GetWorld()->ServerTravel(ListenTravelUrl);
}

void UEOSSessionSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (IOnlineSessionPtr Sessions = GetSessions())
	{
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
	}
	DestroySessionDelegateHandle.Reset();
	ShowMsg(bWasSuccessful ? TEXT("Old session destroyed.") : TEXT("Failed to destroy old session."),
		bWasSuccessful ? FColor::Green : FColor::Red);
}

void UEOSSessionSubsystem::FindAndJoinSession()
{
	if (!IsLoggedIn())
	{
		ShowMsg(TEXT("Not logged in - run EOSLogin first"), FColor::Red);
		return;
	}
	IOnlineSessionPtr Sessions = GetSessions();
	if (!Sessions.IsValid())
	{
		return;
	}

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	SessionSearch->QuerySettings.Set(FName(BucketKeyName), FString(SessionBucket), EOnlineComparisonOp::Equals);

	FindSessionsDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UEOSSessionSubsystem::HandleFindSessionsComplete));

	ShowMsg(TEXT("Searching for sessions..."));
	if (!Sessions->FindSessions(0, SessionSearch.ToSharedRef()))
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
		FindSessionsDelegateHandle.Reset();
		ShowMsg(TEXT("FindSessions call failed immediately"), FColor::Red);
	}
}

void UEOSSessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessions();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
	}
	FindSessionsDelegateHandle.Reset();

	if (!bWasSuccessful || !SessionSearch.IsValid() || SessionSearch->SearchResults.Num() == 0)
	{
		ShowMsg(TEXT("No sessions found. Is the host logged in + hosting? Same build/bucket?"), FColor::Yellow);
		return;
	}

	ShowMsg(FString::Printf(TEXT("Found %d session(s) - joining the first..."), SessionSearch->SearchResults.Num()), FColor::Green);
	JoinSessionDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UEOSSessionSubsystem::HandleJoinSessionComplete));
	Sessions->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[0]);
}

void UEOSSessionSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr Sessions = GetSessions();
	if (Sessions.IsValid())
	{
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
	}
	JoinSessionDelegateHandle.Reset();

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		ShowMsg(FString::Printf(TEXT("Join FAILED (result=%d)"), static_cast<int32>(Result)), FColor::Red);
		return;
	}

	FString ConnectString;
	if (!Sessions.IsValid() || !Sessions->GetResolvedConnectString(SessionName, ConnectString))
	{
		ShowMsg(TEXT("Joined but could not resolve connect string"), FColor::Red);
		return;
	}

	ShowMsg(FString::Printf(TEXT("Joined - traveling to %s"), *ConnectString), FColor::Green);
	if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
	{
		PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
	}
}

void UEOSSessionSubsystem::PrintStatus()
{
	IOnlineIdentityPtr Identity = GetIdentity();
	IOnlineSessionPtr Sessions = GetSessions();
	const UWorld* World = GetGameInstance()->GetWorld();

	FString Status = TEXT("OSS: none");
	if (const IOnlineSubsystem* OSS = Online::GetSubsystem(World))
	{
		Status = FString::Printf(TEXT("OSS: %s"), *OSS->GetSubsystemName().ToString());
	}
	if (Identity.IsValid())
	{
		Status += FString::Printf(TEXT(" | Login: %s (%s)"),
			ELoginStatus::ToString(Identity->GetLoginStatus(0)), *Identity->GetPlayerNickname(0));
	}
	if (Sessions.IsValid() && Sessions->GetNamedSession(NAME_GameSession) != nullptr)
	{
		Status += TEXT(" | In session");
	}
	Status += FString::Printf(TEXT(" | NetMode: %d"), static_cast<int32>(World->GetNetMode()));
	ShowMsg(Status, FColor::White);
}

IOnlineIdentityPtr UEOSSessionSubsystem::GetIdentity() const
{
	if (IOnlineSubsystem* OSS = Online::GetSubsystem(GetGameInstance()->GetWorld()))
	{
		return OSS->GetIdentityInterface();
	}
	return nullptr;
}

IOnlineSessionPtr UEOSSessionSubsystem::GetSessions() const
{
	if (IOnlineSubsystem* OSS = Online::GetSubsystem(GetGameInstance()->GetWorld()))
	{
		return OSS->GetSessionInterface();
	}
	return nullptr;
}

void UEOSSessionSubsystem::ShowMsg(const FString& Msg, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[EOS] %s"), *Msg);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, Color, FString::Printf(TEXT("[EOS] %s"), *Msg));
	}
}
```

### 3.4 — Build

From a terminal:

```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" GTFOCloneEditor Win64 Development -Project="C:\MyFiles\Projects\UE5Projects\GTFOClone\GTFOClone.uproject" -WaitMutex
```

(Or build from Visual Studio / the editor's Live Coding — but a clean Build.bat run is the honest gate.)

---

## ✅ Verify before continuing

- [ ] Build finishes with **`BUILD SUCCESSFUL`** — no errors.
- [ ] The editor loads the module without a "module out of date / failed to compile" prompt.
- [ ] In the editor, **Tools → New C++ Class** dialog (or the Content/C++ view) lists **`EOSSessionSubsystem`** — confirms the class is registered.

> Nothing is wired to a button yet, so there's nothing to click. This gate is purely: **does it compile and register.** The functions get exercised in file 04 (login) and file 06 (host/join).

Common build error: missing `OnlineBase` in `Build.cs` → `SEARCH_LOBBIES` unresolved. See **`99_Troubleshooting.md`**.

Green build → open **`04_UI_And_Controller.md`**.
