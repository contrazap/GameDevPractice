#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/TimerHandle.h"
#include "EOSSessionSubsystem.generated.h"

class IVoiceChatUser;

/**
 * Minimal EOS login + lobby host/join flow.
 * Flow: Login() -> HostSession() (server) / FindAndJoinSession() (client).
 * Host travels to the game map with ?listen; clients ClientTravel to the
 * resolved EOS P2P address ("EOS:<PUID>:GameNetDriver:...").
 */

UCLASS()
class MULTIPLAYERCOURSE_API UEOSSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "EOS")
	void Login();

	/** Logs the Epic account out. Tears down voice; leave any session first. */
	UFUNCTION(BlueprintCallable, Category = "EOS")
	void Logout();

	UFUNCTION(BlueprintCallable, Category = "EOS")
	void HostSession();

	UFUNCTION(BlueprintCallable, Category = "EOS")
	void FindAndJoinSession();

	UFUNCTION(BlueprintCallable, Category = "EOS")
	void PrintStatus();

	/** Leaves the current session (host or client) and returns to a standalone map. */
	UFUNCTION(BlueprintCallable, Category = "EOS")
	void LeaveSession();

	bool IsLoggedIn() const;

	/** True if we currently have a named game session (hosting or joined). */
	bool IsInSession() const;

	/** True if we created the current session (listen server), false if we joined one. */
	bool IsHosting() const;

	// --- Voice controls (device-level mute on our EOS RTC user). ---
	/** True once we're logged in and joined to a voice channel. */
	bool IsVoiceActive() const;
	/** Mute/unmute our microphone (stops sending our voice). */
	void SetMicMuted(bool bMuted);
	bool IsMicMuted() const;
	/** Mute/unmute incoming voice (stops us hearing others). */
	void SetSpeakerMuted(bool bMuted);
	bool IsSpeakerMuted() const;

	virtual void Deinitialize() override;

private:
	void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	IOnlineIdentityPtr GetIdentity() const;
	IOnlineSessionPtr GetSessions() const;
	void ShowMsg(const FString& Msg, const FColor& Color = FColor::Cyan) const;

	// --- Workaround for the UE 5.8 EOSVoiceChat bug where a joining participant is
	// unblocked but never has ApplyPlayerReceivingOptions() re-applied, leaving their
	// audio subscribed=[No] (host/clients hear nothing). We re-apply receiving by
	// calling SetPlayerVolume() whenever a player joins the lobby voice channel. ---
	IVoiceChatUser* GetVoiceChatUser() const;
	void SetupVoiceChatWorkaround();
	void TeardownVoiceChatWorkaround();
	void HandleVoiceChatChannelJoined(const FString& ChannelName);
	void HandleVoiceChatPlayerAdded(const FString& ChannelName, const FString& PlayerName);
	void ScheduleVoiceReceivingReapply(const FString& PlayerName);
	void ReapplyVoiceReceiving(const FString& PlayerName);
	void ReapplyAllRemotePlayers();
	void HandlePostLoadMap(class UWorld* LoadedWorld);

	/** Whether we host, joined, or are out of session - drives leave/travel behaviour. */
	enum class ESessionRole : uint8 { None, Host, Client };
	ESessionRole SessionRole = ESessionRole::None;

	/** When a destroy is triggered by LeaveSession(), travel back to a solo map on completion. */
	bool bReturnToMenuAfterDestroy = false;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	FDelegateHandle LoginDelegateHandle;
	FDelegateHandle CreateSessionDelegateHandle;
	FDelegateHandle FindSessionsDelegateHandle;
	FDelegateHandle JoinSessionDelegateHandle;
	FDelegateHandle DestroySessionDelegateHandle;

	IVoiceChatUser* VoiceChatUser = nullptr;
	FDelegateHandle VoiceChannelJoinedHandle;
	FDelegateHandle VoicePlayerAddedHandle;
	FDelegateHandle PostLoadMapHandle;
	TArray<FTimerHandle> VoiceReapplyTimers;
};
