#include "EOSSessionSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemUtils.h"
#include "IOnlineSubsystemEOS.h"
#include "VoiceChat.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	const TCHAR* SessionBucket = TEXT("GTFOClone_v1");
	const TCHAR* BucketKeyName = TEXT("OSSEOS_BUCKET_ID_ATTRIBUTE_KEY");
	const TCHAR* ListenTravelUrl = TEXT("/Game/ThirdPerson/Lvl_ThirdPerson?listen");
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
		0, FOnLoginCompleteDelegate::CreateUObject(this, &UEOSSessionSubsystem::HandleLoginComplete)
	);

	FString AuthType;
	FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), AuthType);
	if (!AuthType.IsEmpty())
	{
		ShowMsg(FString::Printf(TEXT("AutoLogin via command line (-AUTH_TYPE=%s)..."), *AuthType));
		Identity->AutoLogin(0);
	}
	else
	{
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
		SetupVoiceChatWorkaround();
	}
	else
	{
		ShowMsg(FString::Printf(TEXT("LOGIN FAILED: %s"), *Error), FColor::Red);
	}
}

void UEOSSessionSubsystem::Logout()
{
	IOnlineIdentityPtr Identity = GetIdentity();
	if (!Identity.IsValid())
	{
		ShowMsg(TEXT("No identity interface."), FColor::Red);
		return;
	}
	if (Identity->GetLoginStatus(0) != ELoginStatus::LoggedIn)
	{
		ShowMsg(TEXT("Not logged in."), FColor::Yellow);
		return;
	}

	// Voice is tied to the login, so drop it before logging out. The UI's live
	// IsLoggedIn() attribute flips the button back to "Epic Login" once the logout
	// completes, so no completion delegate (and no extra member) is needed here.
	TeardownVoiceChatWorkaround();

	ShowMsg(TEXT("Logging out..."), FColor::Yellow);
	Identity->Logout(0);
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
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &UEOSSessionSubsystem::HandleDestroySessionComplete)
		);
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
	Settings.bUsesPresence = false;
	Settings.bAllowJoinViaPresence = false;
	Settings.bUseLobbiesIfAvailable = true;
	Settings.bUseLobbiesVoiceChatIfAvailable = true;
	Settings.bUsesStats = false;
	Settings.Set(FName(BucketKeyName), FString(SessionBucket), EOnlineDataAdvertisementType::ViaOnlineService);

	CreateSessionDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UEOSSessionSubsystem::HandleCreateSessionComplete)
	);

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
	SessionRole = ESessionRole::Host;
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
	SessionRole = ESessionRole::None;
	ShowMsg(
		bWasSuccessful
		? TEXT("Session destroyed.")
		: TEXT("Failed to destroy session."),
		bWasSuccessful
		? FColor::Green
		: FColor::Red
	);

	// LeaveSession() path: drop back to a solo copy of the map (works the same for a
	// former host or client - the net connection/listen server is torn down).
	const bool bReturnHome = bReturnToMenuAfterDestroy;
	bReturnToMenuAfterDestroy = false;
	if (bReturnHome)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UWorld* World = GI->GetWorld())
			{
				UGameplayStatics::OpenLevel(World, FName(TEXT("Lvl_ThirdPerson")));
			}
		}
	}
}

void UEOSSessionSubsystem::LeaveSession()
{
	IOnlineSessionPtr Sessions = GetSessions();
	if (!Sessions.IsValid() || Sessions->GetNamedSession(NAME_GameSession) == nullptr)
	{
		ShowMsg(TEXT("Not in a session"), FColor::Yellow);
		return;
	}
	if (DestroySessionDelegateHandle.IsValid())
	{
		ShowMsg(TEXT("Leave already in progress..."));
		return;
	}

	bReturnToMenuAfterDestroy = true;
	DestroySessionDelegateHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &UEOSSessionSubsystem::HandleDestroySessionComplete)
	);
	ShowMsg(TEXT("Leaving session..."));
	Sessions->DestroySession(NAME_GameSession);
}

bool UEOSSessionSubsystem::IsInSession() const
{
	IOnlineSessionPtr Sessions = GetSessions();
	return Sessions.IsValid() && Sessions->GetNamedSession(NAME_GameSession) != nullptr;
}

bool UEOSSessionSubsystem::IsHosting() const
{
	return SessionRole == ESessionRole::Host;
}

bool UEOSSessionSubsystem::IsVoiceActive() const
{
	return VoiceChatUser != nullptr && VoiceChatUser->GetChannels().Num() > 0;
}

void UEOSSessionSubsystem::SetMicMuted(bool bMuted)
{
	if (VoiceChatUser)
	{
		VoiceChatUser->SetAudioInputDeviceMuted(bMuted);
	}
}

bool UEOSSessionSubsystem::IsMicMuted() const
{
	return VoiceChatUser != nullptr && VoiceChatUser->GetAudioInputDeviceMuted();
}

void UEOSSessionSubsystem::SetSpeakerMuted(bool bMuted)
{
	if (VoiceChatUser)
	{
		VoiceChatUser->SetAudioOutputDeviceMuted(bMuted);
	}
}

bool UEOSSessionSubsystem::IsSpeakerMuted() const
{
	return VoiceChatUser != nullptr && VoiceChatUser->GetAudioOutputDeviceMuted();
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
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UEOSSessionSubsystem::HandleFindSessionsComplete)
	);

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
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UEOSSessionSubsystem::HandleJoinSessionComplete)
	);
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

	SessionRole = ESessionRole::Client;
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

void UEOSSessionSubsystem::Deinitialize()
{
	TeardownVoiceChatWorkaround();
	Super::Deinitialize();
}

IVoiceChatUser* UEOSSessionSubsystem::GetVoiceChatUser() const
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	IOnlineSubsystem* OSS = Online::GetSubsystem(World);
	if (!OSS || OSS->GetSubsystemName() != FName(TEXT("EOS")))
	{
		return nullptr;
	}

	IOnlineIdentityPtr Identity = GetIdentity();
	if (!Identity.IsValid())
	{
		return nullptr;
	}

	FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
	if (!UserId.IsValid())
	{
		return nullptr;
	}

	// IOnlineSubsystemEOS is a base of the concrete FOnlineSubsystemEOS along a single
	// inheritance chain, so this downcast is safe once we've confirmed the subsystem is EOS.
	IOnlineSubsystemEOS* EOSSubsystem = static_cast<IOnlineSubsystemEOS*>(OSS);
	return EOSSubsystem->GetVoiceChatUserInterface(*UserId);
}

void UEOSSessionSubsystem::SetupVoiceChatWorkaround()
{
	if (VoiceChatUser)
	{
		return; // already armed
	}

	VoiceChatUser = GetVoiceChatUser();
	if (!VoiceChatUser)
	{
		ShowMsg(TEXT("Voice fix: no VoiceChatUser (EOS RTC disabled?)"), FColor::Yellow);
		return;
	}

	// Re-apply receiving for players already in a channel when we join it...
	VoiceChannelJoinedHandle = VoiceChatUser->OnVoiceChatChannelJoined().AddUObject(
		this, &UEOSSessionSubsystem::HandleVoiceChatChannelJoined);
	// ...and for players who join after us.
	VoicePlayerAddedHandle = VoiceChatUser->OnVoiceChatPlayerAdded().AddUObject(
		this, &UEOSSessionSubsystem::HandleVoiceChatPlayerAdded);

	// CLIENT path: a joining client travels into the game map right after joining the voice
	// room, so its in-storm re-applies are rejected (EOS 7002 "already applied in another
	// streaming process") and the map-load hitch eats the delayed GameInstance timers.
	// Re-apply once travel completes - reliably post-storm - to catch the remote host.
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &UEOSSessionSubsystem::HandlePostLoadMap);

	ShowMsg(TEXT("Voice receive-fix armed"), FColor::Green);
}

void UEOSSessionSubsystem::TeardownVoiceChatWorkaround()
{
	if (VoiceChatUser)
	{
		if (VoiceChannelJoinedHandle.IsValid())
		{
			VoiceChatUser->OnVoiceChatChannelJoined().Remove(VoiceChannelJoinedHandle);
		}
		if (VoicePlayerAddedHandle.IsValid())
		{
			VoiceChatUser->OnVoiceChatPlayerAdded().Remove(VoicePlayerAddedHandle);
		}
	}
	VoiceChannelJoinedHandle.Reset();
	VoicePlayerAddedHandle.Reset();

	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		FTimerManager& TM = GI->GetTimerManager();
		for (FTimerHandle& Handle : VoiceReapplyTimers)
		{
			TM.ClearTimer(Handle);
		}
	}
	VoiceReapplyTimers.Reset();
	VoiceChatUser = nullptr;
}

void UEOSSessionSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (!VoiceChatUser)
	{
		return;
	}

	// Post-travel, post-storm: re-apply now, then once more shortly after on the freshly
	// loaded world's timer manager (reliable, unlike GameInstance timers that the travel
	// hitch dropped). This is what makes the CLIENT hear the host.
	ReapplyAllRemotePlayers();

	if (LoadedWorld)
	{
		FTimerHandle Handle;
		FTimerDelegate Del = FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			ReapplyAllRemotePlayers();
		});
		LoadedWorld->GetTimerManager().SetTimer(Handle, Del, 1.5f, false);
		VoiceReapplyTimers.Add(Handle);
	}
}

void UEOSSessionSubsystem::ReapplyAllRemotePlayers()
{
	if (!VoiceChatUser)
	{
		return;
	}

	const FString LocalPlayerName = VoiceChatUser->GetLoggedInPlayerName();
	for (const FString& ChannelName : VoiceChatUser->GetChannels())
	{
		for (const FString& PlayerName : VoiceChatUser->GetPlayersInChannel(ChannelName))
		{
			if (PlayerName != LocalPlayerName)
			{
				ReapplyVoiceReceiving(PlayerName);
			}
		}
	}
}

void UEOSSessionSubsystem::HandleVoiceChatChannelJoined(const FString& ChannelName)
{
	if (!VoiceChatUser)
	{
		return;
	}

	const FString LocalPlayerName = VoiceChatUser->GetLoggedInPlayerName();
	for (const FString& PlayerName : VoiceChatUser->GetPlayersInChannel(ChannelName))
	{
		if (PlayerName != LocalPlayerName)
		{
			ScheduleVoiceReceivingReapply(PlayerName);
		}
	}
}

void UEOSSessionSubsystem::HandleVoiceChatPlayerAdded(const FString& ChannelName, const FString& PlayerName)
{
	if (VoiceChatUser && PlayerName != VoiceChatUser->GetLoggedInPlayerName())
	{
		ScheduleVoiceReceivingReapply(PlayerName);
	}
}

void UEOSSessionSubsystem::ScheduleVoiceReceivingReapply(const FString& PlayerName)
{
	// Re-apply once immediately (in case we're already past the join storm)...
	ReapplyVoiceReceiving(PlayerName);

	// ...but the EOS block/unblock sequence at join runs for a few frames and stomps an
	// in-storm re-apply back to subscribed=[No]. Re-apply again AFTER it settles so the
	// subscription sticks. Timers live on the GameInstance, so they survive map travel.
	if (UGameInstance* GI = GetGameInstance())
	{
		FTimerManager& TM = GI->GetTimerManager();
		for (const float Delay : { 0.75f, 2.0f, 4.0f })
		{
			FTimerHandle Handle;
			FTimerDelegate Del = FTimerDelegate::CreateWeakLambda(this, [this, PlayerName]()
			{
				ReapplyVoiceReceiving(PlayerName);
			});
			TM.SetTimer(Handle, Del, Delay, false);
			VoiceReapplyTimers.Add(Handle);
		}
	}
}

void UEOSSessionSubsystem::ReapplyVoiceReceiving(const FString& PlayerName)
{
	if (!VoiceChatUser)
	{
		return;
	}

	// SetPlayerVolume() unconditionally calls ApplyPlayerReceivingOptions() inside the EOS
	// voice plugin, re-subscribing this participant's audio - the step the buggy
	// OnBlockParticipant() skips. 1.0 leaves the volume unchanged, so audibly this only
	// restores the dropped subscription.
	VoiceChatUser->SetPlayerVolume(PlayerName, 1.0f);
	ShowMsg(FString::Printf(TEXT("Voice: re-subscribed %s"), *PlayerName), FColor::Cyan);
}
