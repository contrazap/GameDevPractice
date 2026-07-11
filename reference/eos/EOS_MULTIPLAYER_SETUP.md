# EOS Multiplayer + Voice Chat — Implementation Spec (UE 5.8)

**Status:** READY TO EXECUTE — waiting for user to create the base C++ Third Person template project in this folder.
**Audience:** Claude Code agent (or human) executing the setup. Written 2026-07-07 after verifying every claim below against the locally installed UE 5.8 source at `C:\Program Files\Epic Games\UE_5.8`.
**Goal for this milestone:** Internet multiplayer (PC ↔ Android crossplay) via Epic Online Services with Epic account login, one replicated actor visible to all players, and lobby voice chat. GTFO gameplay comes later (see `gtfo_clone_project_brief.md`).

---

## 0. Context

- User: solo hobby dev (`contrazap@gmail.com`), Windows 11, UE 5.6/5.7/5.8 installed. Has packaged Win64 + Android before. LAN multiplayer via hardcoded IP already worked in a previous project.
- Base project: user will create a **C++ Third Person template** project themselves (expected name `GTFOClone`, in `C:\MyFiles\Projects\UE5Projects\GTFOClone`). **Do not scaffold the project for them.** If the project/module name differs from `GTFOClone`, substitute the module name and `GTFOCLONE_API` macro accordingly in all code below.
- After project creation, the agent may also set up the official **Unreal MCP plugin** (§8) to help with editor-side work (map edits, placing actors).

## 1. Decision & Rationale

**Chosen stack: built-in `OnlineSubsystemEOS` (OSSv1) + Epic account login (Account Portal) + EOS Lobbies + EOS P2P listen server + EOS RTC lobby voice.**

Why:
- Ships with the engine — zero third-party dependencies, free.
- Android is officially supported: plugin `PlatformAllowList` includes Android, and `libEOSSDK.so` (arm64-v8a) ships at `Engine\Source\ThirdParty\EOSSDK\SDK\Bin\Android`.
- EOS P2P gives NAT punch-through + relay — internet play with **no port forwarding**.
- Same stack Lyra uses; aligns with the project brief's plan to study Lyra.
- Voice: EOS RTC rooms attach automatically to lobbies (no separate voice service).

Rejected alternatives:
- **Device-ID (anonymous) login:** NOT supported by the built-in plugin. Verified in `UserManagerEOS.cpp` — only `exchangecode`, `developer`, `password`, `accountportal`, `persistentauth`, `externalauth`. Anonymous login would require Redpoint's EOS Online Framework (3rd-party; free tier exists for <$30k/yr hobbyists at docs.redpoint.games) — fallback option if Epic login proves too painful on Android.
- **Steam:** no Android.
- **Hardcoded IP over VPN (Tailscale/ZeroTier):** zero-code emergency fallback for playtests (works with the existing `ClientTravel(ip)` approach, Android apps exist), but no session discovery/voice, and not the learning goal.

### Facts verified against UE 5.8 engine source (do not re-litigate; re-verify only if engine version changes)

| Fact | Where verified |
|---|---|
| Login types: `exchangecode`, `developer`, `password`, `accountportal`, `persistentauth`, `externalauth` — **no deviceid** | `OnlineSubsystemEOS\Private\UserManagerEOS.cpp` ~line 198–223 |
| Plugin auto-falls back persistentauth → accountportal; `bPreferPersistentAuth` setting exists | `UserManagerEOS.cpp` ~735–993, `EOSSettings.h` |
| `AutoLogin` reads `-AUTH_TYPE=`, (`-AUTH_LOGIN=`, `-AUTH_PASSWORD=`) from command line | `UserManagerEOS.cpp` ~1605 |
| Config class `[/Script/OnlineSubsystemEOS.EOSSettings]`; artifact struct keys: `ArtifactName, ClientId, ClientSecret, ProductId, SandboxId, DeploymentId, ClientEncryptionKey` (**renamed from** `EncryptionKey` — old tutorials are wrong) | `EOSSettings.h` (`FArtifactSettings`) |
| Settings: `bUseEAS`, `bUseEOSConnect`, `bUseEOSRTC` (default true), `bPreferPersistentAuth`, `AuthScopeFlags` (array; token `BasicProfile` parses via `LexFromString`) | `EOSSettings.h`, `EOSShared.cpp` ~322 |
| Net driver class is `/Script/SocketSubsystemEOS.NetDriverEOS`; `bIsUsingP2PSockets` is **deprecated since 5.6** (always-on) — do not set it | `SocketSubsystemEOS\Public\NetDriverEOS.h` |
| `NetDriverEOS` falls through to IP behavior for non-EOS URLs (`bIsPassthrough`) → LAN/IP travel still works with the fallback IpNetDriver definition | `NetDriverEOS.h/.cpp` |
| Lobby bucket: custom session setting / search param `FName("OSSEOS_BUCKET_ID_ATTRIBUTE_KEY")` (macro in public header `OnlineSubsystemEOSTypesPublic.h`); defaults to `BuildUniqueId` if absent | `OnlineSessionEOS.cpp` ~4064–4103 |
| Host advertises custom settings with `>= ViaOnlineService` as lobby attributes (`SetLobbyAttributes`), so a client-side bucket query param matches both the EOS bucket filter AND the generic attribute filter — set the bucket **symmetrically on create and search** | `OnlineSessionEOS.cpp` ~4442–4483, ~4693–4733 |
| Lobby search requires query param `SEARCH_LOBBIES` (= `FName("LOBBYSEARCH")`, from `Online/OnlineSessionNames.h` in `OnlineBase` module) set to `true` | `OnlineSessionEOS.cpp` ~2289 |
| Voice: `FOnlineSessionSettings::bUseLobbiesVoiceChatIfAvailable` → `CreateLobbyOptions.bEnableRTCRoom`; RTC room joined automatically (`EOS_LRRJAT_AutomaticJoin`) | `OnlineSessionEOS.cpp` ~4137–4153 |
| `bUsesPresence=false` forces `bAllowJoinViaPresence=false` (logged warning) — set both false to avoid needing the Presence scope/permission | `OnlineSessionEOS.cpp` ~4118–4124 |
| Android login redirect: engine UPL auto-injects an intent filter with scheme `eos.<clientid-lowercase>` read from the `Artifacts=` line in `DefaultEngine.ini` — **no manual AndroidManifest work**, but the Artifacts line must be correct at package time | `Engine\Source\ThirdParty\EOSSDK\EOSSDK_UPL.xml` ~78–130 |
| `NAME_GameSession` is a Core registered name (`UnrealNames.inl` #287) — available everywhere, matches `APlayerState::SessionName` for auto player registration | `Core\Public\UObject\UnrealNames.inl`, `Engine\Private\PlayerState.cpp` |
| Unreal MCP plugin: `Engine\Plugins\Experimental\ModelContextProtocol` ("Unreal MCP", experimental). HTTP server, default `http://127.0.0.1:8000/mcp`, settings class `UModelContextProtocolSettings` (config=`EditorPerProjectUserSettings`), `bAutoStartServer=false` by default, can write Claude Code `.mcp.json` into project root | plugin source, `ModelContextProtocolSettings.h`, `ModelContextProtocolClientConfig.h` |

---

## 2. Execution Order (agent TODO list)

1. §3 Epic Dev Portal walkthrough with user (blocking; needs their browser) — collect + verify evidence.
2. §4 Project config: enable plugins, write `DefaultEngine.ini` blocks, insert real credentials.
3. §5 Add C++ files, build `<Project>Editor Win64 Development`.
4. §6 Place `AReplicatedTestCube` in the third-person map (manually, via MCP plugin, or via the Python snippet).
5. §7 Test matrix: PIE/standalone with DevAuthTool → packaged Win64 ↔ Win64 → Win64 ↔ Android. Collect log evidence.
6. §8 (optional, on request) Unreal MCP plugin setup.
7. Voice verification (§7.4) — mic permission on Android, audible two-device test.

---

## 3. Epic Developer Portal Setup (user does this in browser; agent guides & verifies)

Portal: https://dev.epicgames.com/portal — sign in with the user's Epic account.

> **Evidence protocol:** after each step, have the user paste the value or a screenshot into chat. Verify format before proceeding (wrong/foreign IDs are the #1 cause of silent login failure). ProductId / SandboxId / DeploymentId / ClientId are 32-char lowercase hex strings (ClientId conventionally starts with `xyza`). ClientSecret is a generated string — treat as a secret; it will live in `DefaultEngine.ini` (fine for dev; note it ships in packaged builds).

1. **Organization** — create one if prompted (any name). Note: until the EAS application passes "Brand Review" (do NOT submit for this project), **only members of this organization can log in**. So: Organization → Members → invite the 2nd Epic account (the one the phone will use) and any friends' accounts. They must accept the email invite. *Evidence: member list screenshot showing Accepted status.*
2. **Create Product** — name `GTFOClone`. *Evidence: product appears in sidebar.*
3. **Product Settings → General** — copy **ProductId**. *Evidence: value.*
4. **Product Settings → Sandboxes/Deployments** — use the default `Live` sandbox and its default deployment. Copy **SandboxId** and **DeploymentId**. *Evidence: both values.*
5. **Product Settings → Clients → Add New Client**:
   - Name: `GTFOCloneClient`.
   - Client Policy: create new, name `GTFOClonePolicy`, policy type **`Peer2Peer`** (this grants player-hosted Sessions/Lobbies + P2P relay actions).
   - **Voice:** on the policy's feature list, additionally enable the **Voice** feature (Voice → `voice:createRoomToken` / RTC actions) if it is not already included by Peer2Peer — required for lobby RTC rooms. If the portal UI doesn't allow adding Voice to a Peer2Peer preset, switch the policy to **Custom** and enable: Sessions (player-owned), Lobbies, P2P relays, Voice.
   - Copy **ClientId** and **ClientSecret**. *Evidence: screenshot of the policy's enabled features + both values.*
6. **Epic Account Services (left sidebar) → Create Application** (auto-created for the product in some portal versions — then just configure it):
   - **Permissions tab:** enable **Basic Profile** only (matches `AuthScopeFlags=BasicProfile` in our config — requested scopes must match portal permissions or login fails with a scope error). Leave Online Presence and Friends OFF for now.
   - **Linked Clients tab:** link `GTFOCloneClient`.
   - Brand Settings: leave unsubmitted (dev mode). *Evidence: screenshot of Permissions tab + Linked Clients tab.*
7. *(For voice, nothing else portal-side — RTC is enabled by client policy + `bUseEOSRTC`.)*

Collected values → fill into §4's `Artifacts=` line:
```
ProductId=________  SandboxId=________  DeploymentId=________
ClientId=________   ClientSecret=________
```

---

## 4. Project Configuration

### 4.1 `<Project>.uproject` — add plugins

```json
"Plugins": [
    { "Name": "OnlineSubsystem", "Enabled": true },
    { "Name": "OnlineSubsystemUtils", "Enabled": true },
    { "Name": "OnlineSubsystemEOS", "Enabled": true }
]
```
(Append to the template's existing `Plugins` array — keep ModelingToolsEditorMode etc. `EOSShared`, `SocketSubsystemEOS`, `EOSVoiceChat` are auto-enabled as dependencies of `OnlineSubsystemEOS`.)

### 4.2 `Config/DefaultEngine.ini` — append these blocks

```ini
; ---------------- EOS multiplayer ----------------
[OnlineSubsystem]
DefaultPlatformService=EOS

[OnlineSubsystemEOS]
bEnabled=true

[/Script/Engine.GameEngine]
!NetDriverDefinitions=ClearArray
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/SocketSubsystemEOS.NetDriverEOS",DriverClassNameFallback="/Script/OnlineSubsystemUtils.IpNetDriver")

[/Script/OnlineSubsystemEOS.EOSSettings]
DefaultArtifactName=GTFOClone
bUseEAS=True
bUseEOSConnect=True
bPreferPersistentAuth=True
bUseEOSRTC=True
+AuthScopeFlags=BasicProfile
+Artifacts=(ArtifactName="GTFOClone",ClientId="REPLACE_ME",ClientSecret="REPLACE_ME",ProductId="REPLACE_ME",SandboxId="REPLACE_ME",DeploymentId="REPLACE_ME",ClientEncryptionKey="REPLACE_ME")

; ---------------- Android ----------------
[/Script/AndroidRuntimeSettings.AndroidRuntimeSettings]
PackageName=com.contrazap.gtfoclone
bPackageDataInsideApk=True
+ExtraPermissions=android.permission.RECORD_AUDIO
```

Notes:
- `ClientEncryptionKey` = any 64-hex-char string (used for Player/Title Data Storage; not portal-issued). The one above is fine.
- The key is named `ClientEncryptionKey`, NOT `EncryptionKey` (5.8 rename — `EncryptionKey` gets stripped from packaged builds by `IniKeyDenylist`).
- Do NOT add `bIsUsingP2PSockets` (deprecated 5.6, always-on).
- `bUseEOSRTC=True` + `RECORD_AUDIO` are for voice. If voice is deferred, set RTC to False and drop the permission.
- Keep the template's existing `GameMapsSettings`/input sections untouched.
- Optional while debugging (very noisy): `[Core.Log]` → `LogEOSSDK=Verbose`, `LogOnlineSession=Verbose`, `LogNet=Log`.

---

## 5. C++ Implementation

Files to add under `Source/GTFOClone/` (adjust module/API macro to actual project name). The template's Character/GameMode stay as-is — the third-person character already replicates movement, so connected players see each other walk around out of the box.

### 5.1 `GTFOClone.Build.cs` — add dependencies

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

### 5.2 `EOSSessionSubsystem.h`

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

### 5.3 `EOSSessionSubsystem.cpp`

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

### 5.4 `GTFOClonePlayerController.h` / `.cpp` — exec commands + touch-friendly Slate buttons

On-screen buttons matter because Android has no easy console (4-finger tap opens it in non-shipping builds, but typing on a phone is painful).

> **Note — this is the minimal version.** The block below is the original always-on corner
> menu. The shipping project replaces it with an Escape/Back pause-less menu (state guards,
> Leave Session, voice mute HUD, direct mouse-look, platform-specific toggle label). See
> **`EOS_Setup_Guide/10_InGame_Menu_Voice_HUD.md`** for the current design and code.

```cpp
// GTFOClonePlayerController.h
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

```cpp
// GTFOClonePlayerController.cpp
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

**Wire the controller in:** in the template's `AGTFOCloneGameMode` constructor add
`PlayerControllerClass = AGTFOClonePlayerController::StaticClass();`
(The template's BP GameMode subclass inherits this unless it explicitly overrides PlayerControllerClass — verify in `BP_ThirdPersonGameMode` that Player Controller Class shows the new C++ class.)

### 5.5 `ReplicatedTestCube.h` / `.cpp` — the replication proof actor

Server-only tick rotates/bobs it (movement replication) and cycles a color every 2 s (`ReplicatedUsing` property replication). Clients render what the server dictates.

```cpp
// ReplicatedTestCube.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReplicatedTestCube.generated.h"

class UMaterialInstanceDynamic;
class UStaticMeshComponent;

UCLASS()
class GTFOCLONE_API AReplicatedTestCube : public AActor
{
	GENERATED_BODY()

public:
	AReplicatedTestCube();
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnRep_ColorIndex();

	UPROPERTY(VisibleAnywhere, Category = "Test")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(ReplicatedUsing = OnRep_ColorIndex)
	uint8 ColorIndex = 0;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> Mid;

	float SpawnZ = 0.f;
	float ColorTimer = 0.f;
};
```

```cpp
// ReplicatedTestCube.cpp
#include "ReplicatedTestCube.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FLinearColor CubeColors[] =
	{
		FLinearColor::Red,
		FLinearColor::Green,
		FLinearColor::Blue,
		FLinearColor::Yellow,
	};
}

AReplicatedTestCube::AReplicatedTestCube()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
}

void AReplicatedTestCube::BeginPlay()
{
	Super::BeginPlay();
	SpawnZ = GetActorLocation().Z;
	Mid = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	OnRep_ColorIndex();
}

void AReplicatedTestCube::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Server drives all state; clients only receive replication.
	if (!HasAuthority())
	{
		return;
	}

	AddActorWorldRotation(FRotator(0.f, 60.f * DeltaSeconds, 0.f));

	FVector Location = GetActorLocation();
	Location.Z = SpawnZ + 60.f * FMath::Sin(GetWorld()->GetTimeSeconds() * 1.5f);
	SetActorLocation(Location);

	ColorTimer += DeltaSeconds;
	if (ColorTimer >= 2.f)
	{
		ColorTimer = 0.f;
		ColorIndex = (ColorIndex + 1) % UE_ARRAY_COUNT(CubeColors);
		OnRep_ColorIndex(); // OnRep doesn't fire on the authority; apply locally for the listen server
	}
}

void AReplicatedTestCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AReplicatedTestCube, ColorIndex);
}

void AReplicatedTestCube::OnRep_ColorIndex()
{
	if (Mid)
	{
		Mid->SetVectorParameterValue(TEXT("Color"), CubeColors[ColorIndex]);
	}
}
```

### 5.6 Build

```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" GTFOCloneEditor Win64 Development -Project="C:\MyFiles\Projects\UE5Projects\GTFOClone\GTFOClone.uproject" -WaitMutex
```

---

## 6. Level Setup

Place one `AReplicatedTestCube` in the third-person map (`/Game/ThirdPerson/Maps/ThirdPersonMap`), e.g. at (300, 0, 300). Options:
1. **Editor drag-drop** (user, 30 seconds — simplest).
2. **Unreal MCP plugin** (§8) — agent places it via MCP tools.
3. **Headless Python** (PythonScriptPlugin must be enabled in the .uproject):
   ```powershell
   & "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "<uproject>" -ExecutePythonScript="<script.py>"
   ```
   ```python
   import unreal
   les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
   eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
   les.load_level("/Game/ThirdPerson/Maps/ThirdPersonMap")
   cube_cls = unreal.load_class(None, "/Script/GTFOClone.ReplicatedTestCube")
   cube = eas.spawn_actor_from_class(cube_cls, unreal.Vector(300, 0, 300))
   cube.set_actor_label("ReplicatedTestCube")
   les.save_current_level()
   ```

---

## 7. Testing & Evidence

### 7.1 Fast local iteration — Dev Auth Tool (recommended before touching Android)

1. Download the **EOS SDK** zip from the Dev Portal (Product → SDK → C SDK) — the **Dev Auth Tool** is in `SDK\Tools\EOS_DevAuthTool-win32-x64-*.zip`. Unzip, run, set port **8081**, log in with two different Epic accounts, naming credentials `Cred1` and `Cred2`.
2. Editor → Play → Advanced Settings → Play Standalone, or launch two packaged/-game instances with:
   - Instance A: `-AUTH_TYPE=developer -AUTH_LOGIN=localhost:8081 -AUTH_PASSWORD=Cred1`
   - Instance B: `-AUTH_TYPE=developer -AUTH_LOGIN=localhost:8081 -AUTH_PASSWORD=Cred2`
   (Editor field: "Additional Launch Parameters". The subsystem's `Login()` detects `-AUTH_TYPE` and uses `AutoLogin` automatically.)
3. Instance A: Login → Host. Instance B: Login → Join. B's character should appear in A's world; the cube must rotate/bob/color-cycle identically in both.

Note: plain multi-client PIE (one process) shares one EOS instance and will not give two logins — use standalone instances or packaged builds for multiplayer tests.

### 7.2 Packaged Windows ↔ Windows over internet
Package Development build, run on two PCs (or one PC + one laptop on a different network/hotspot to prove non-LAN). Buttons: Login (Account Portal browser pops; persistent auth caches it) → Host / Join.

### 7.3 Android
- Package **Development** (console available via 4-finger tap; on-screen buttons work regardless).
- **Both test accounts must be members of the EOS organization** (brand review not passed ⇒ non-members cannot log in).
- Login opens the browser via the auto-injected `eos.<clientid>` intent filter. *Evidence:* after packaging, check `Intermediate\Android\arm64\gradle\app\src\main\AndroidManifest.xml` contains `<data android:scheme="eos.<clientid>"/>`. If missing, the `Artifacts=` line was wrong/absent at package time.
- Logs: `adb logcat -s UE` (look for `LogEOSSDK`, `[EOS]` messages).

### 7.4 Voice chat verification
1. Portal evidence first (§3.5): client policy shows Voice enabled.
2. Host + join from two devices **in different rooms** (or headphones) — echo makes it hard to judge.
3. Expected: voice connects automatically after joining the lobby (RTC room auto-join — no game code needed beyond `bUseLobbiesVoiceChatIfAvailable=true`).
4. Log evidence: `LogEOSVoiceChat` lines showing user login + channel join; on failure grep for `EOS_Permission` / `EOS_MissingPermissions` (→ client policy lacks Voice) or RTC init errors (→ `bUseEOSRTC=False` somewhere).
5. Android: confirm the OS mic-permission dialog appeared on first run (from `AcquirePermissions`), and Settings → App shows Microphone allowed.
6. Later (not this milestone): mute/volume UI via `IVoiceChatUser` (`EOSVoiceChat` plugin implements `IVoiceChat`).

### 7.5 Troubleshooting table

| Symptom | Likely cause |
|---|---|
| Login fails instantly, `LogEOSSDK: ... EOS_InvalidCredentials` / `EOS_Auth_...` | Wrong ClientId/Secret, or client not linked to the EAS application (§3.6) |
| Login fails with scope/consent error | Portal Permissions ≠ `AuthScopeFlags` — both must be Basic Profile only |
| Login works for you, fails for friend | Friend's account not an accepted org member (brand review gate) |
| Browser opens but app never gets the token (Android) | Missing/wrong intent filter — Artifacts line broken at package time (§7.3) |
| `CreateSession` fails, `EOS_Permission` denied | Client policy missing Lobbies/Sessions actions — use Peer2Peer/Custom policy |
| Search finds 0 sessions | Host not logged in/hosting; bucket string mismatch; different `SessionBucket` between builds |
| Join OK but travel fails / timeout | NetDriverDefinitions block missing on one side; check both builds' `DefaultEngine.ini` |
| No voice | §7.4 — policy Voice missing, RTC disabled, or mic permission denied |

### Definition of done
- [ ] PC (host) + Android (client) connect over the internet (different networks ideally) via EOS, no IP/port config.
- [ ] Both see each other's third-person characters moving.
- [ ] `ReplicatedTestCube` rotates/bobs/color-cycles in sync on both.
- [ ] Two-way voice audible.
- [ ] Evidence collected: portal screenshots, login/host/join log excerpts, short screen recording.

---

## 8. Optional: Official Unreal MCP Plugin (experimental, ships with 5.8)

Purpose: lets a Claude Code agent drive the running editor (place actors, edit levels, call BP tool libraries) — useful from Day 5–6 of the brief (level assembly).

Verified facts (plugin source, `Engine\Plugins\Experimental\ModelContextProtocol`):
- Friendly name **"Unreal MCP"**; experimental; disabled by default; NoRedist.
- HTTP MCP server, default **`http://127.0.0.1:8000/mcp`** (`ServerPortNumber=8000`, `ServerUrlPath=/mcp`).
- Settings: `UModelContextProtocolSettings`, stored in **EditorPerProjectUserSettings** (Editor Preferences → "Model Context Protocol"). `bAutoStartServer=false` by default. `bEnableToolSearch=true` (tools exposed via `list_toolsets`/`describe_toolset`/`call_tool`).
- Has built-in Claude Code config generation: writes `.mcp.json` into the project root (`EModelContextProtocolClient::ClaudeCode`).

Setup steps:
1. Enable plugin "Unreal MCP" (Edit → Plugins, category Other / search "MCP"). Restart editor. (Or add `{ "Name": "ModelContextProtocol", "Enabled": true }` to the .uproject.)
2. Editor Preferences → Model Context Protocol → set **Auto Start Server = true** (or equivalently in `Saved\Config\WindowsEditor\EditorPerProjectUserSettings.ini`: `[/Script/ModelContextProtocolEngine.ModelContextProtocolSettings]` → `bAutoStartServer=True`). Restart editor.
3. Create/verify `.mcp.json` in the project root:
   ```json
   {
     "mcpServers": {
       "unreal": { "type": "http", "url": "http://127.0.0.1:8000/mcp" }
     }
   }
   ```
4. Verify: with the editor running, `curl http://127.0.0.1:8000/mcp` should respond (405/JSON-RPC error is fine — it means the server is listening); then in Claude Code, `/mcp` should list the `unreal` server.
5. Editor must be running for the MCP tools to work; the agent uses `list_toolsets` → `describe_toolset` → `call_tool`.

---

## 9. Deferred / later milestones
- Mute/volume voice UI (`IVoiceChat`), positional voice.
- Friends list + invites (needs FriendsList/Presence scopes + portal permissions + presence-enabled lobbies).
- Session browser UI instead of join-first-result.
- Brand review submission if ever distributing beyond org members.
- GAS + game systems per `gtfo_clone_project_brief.md`.
