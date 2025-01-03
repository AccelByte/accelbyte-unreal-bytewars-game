// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleOnlineSession.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "SessionEssentialsOnlineSession_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API USessionEssentialsOnlineSession_Starter : public UAccelByteWarsOnlineSessionBase
{
	GENERATED_BODY()

public:
	virtual void RegisterOnlineDelegates() override;
	virtual void ClearOnlineDelegates() override;

	virtual FNamedOnlineSession* GetSession(const FName SessionName) override;
	virtual EAccelByteV2SessionType GetSessionType(const FName SessionName) override;
	virtual FName GetPredefinedSessionNameFromType(const EAccelByteV2SessionType SessionType) override;

#pragma region "Funtions Declarations"
	// TODO: declare your public functions here
#pragma endregion 

	virtual FOnCreateSessionComplete* GetOnCreateSessionCompleteDelegates() override
	{
		return &OnCreateSessionCompleteDelegates;
	}
	virtual FOnJoinSessionComplete* GetOnJoinSessionCompleteDelegates() override
	{
		return &OnJoinSessionCompleteDelegates;
	}
	virtual FOnSendSessionInviteComplete* GetOnSendSessionInviteCompleteDelegates() override
	{
		return &OnSendSessionInviteCompleteDelegates;
	}
	virtual FOnRejectSessionInviteCompleteMulticast* GetOnRejectSessionInviteCompleteDelegate() override
	{
		return &OnRejectSessionInviteCompleteDelegates;
	}
	virtual FOnDestroySessionComplete* GetOnLeaveSessionCompleteDelegates() override
	{
		return &OnLeaveSessionCompleteDelegates;
	}
	virtual FOnUpdateSessionComplete* GetOnUpdateSessionCompleteDelegates() override
	{
		return &OnUpdateSessionCompleteDelegates;
	}

	virtual FOnV2SessionInviteReceived* GetOnSessionInviteReceivedDelegates() override
	{
		return &OnSessionInviteReceivedDelegates;
	}

#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
	virtual FOnSessionParticipantsChange* GetOnSessionParticipantsChange() override
	{
		return &OnSessionParticipantsChangeDelegates;
	}
#else
	virtual FOnSessionParticipantJoined* GetOnSessionParticipantJoined() override
	{
		return &OnSessionParticipantJoinedDelegates;
	}

	virtual FOnSessionParticipantLeft* GetOnSessionParticipantLeft() override
	{
		return &OnSessionParticipantLeftDelegates;
	}
#endif

protected:
	bool bLeavingSession = false;

#pragma region "Funtions Declarations"
	// TODO: declare your protected functions here
#pragma endregion 

private:
	const FName GameSessionName = NAME_GameSession;
	const FName PartySessionName = NAME_PartySession;

	FOnCreateSessionComplete OnCreateSessionCompleteDelegates;
	FOnJoinSessionComplete OnJoinSessionCompleteDelegates;
	FOnSendSessionInviteComplete OnSendSessionInviteCompleteDelegates;
	FOnRejectSessionInviteCompleteMulticast OnRejectSessionInviteCompleteDelegates;
	FOnDestroySessionComplete OnLeaveSessionCompleteDelegates;
	FOnUpdateSessionComplete OnUpdateSessionCompleteDelegates;

	FOnV2SessionInviteReceived OnSessionInviteReceivedDelegates;

#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
	FOnSessionParticipantsChange OnSessionParticipantsChangeDelegates;
#else
	FOnSessionParticipantJoined OnSessionParticipantJoinedDelegates;
	FOnSessionParticipantLeft OnSessionParticipantLeftDelegates;
#endif

#pragma region "Funtions Declarations"
	// TODO: declare your private functions here
#pragma endregion 
};
