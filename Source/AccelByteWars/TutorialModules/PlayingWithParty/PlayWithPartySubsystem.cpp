// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/PlayingWithParty/PlayWithPartySubsystem.h"

#include "TutorialModules/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemUtils.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

void UPlayWithPartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (GetSessionInterface()) 
    {
        GetSessionInterface()->OnMatchmakingStartedDelegates.AddUObject(this, &ThisClass::OnStartPartyMatchmakingComplete);
        GetSessionInterface()->OnMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnPartyMatchmakingComplete);
        GetSessionInterface()->OnMatchmakingCanceledDelegates.AddUObject(this, &ThisClass::OnPartyMatchmakingCanceled);
        GetSessionInterface()->OnMatchmakingExpiredDelegates.AddUObject(this, &ThisClass::OnPartyMatchmakingExpired);

        GetSessionInterface()->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreatePartyMatchComplete);
        GetSessionInterface()->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinPartyMatchComplete);
        GetSessionInterface()->OnV2SessionInviteReceivedDelegates.AddUObject(this, &ThisClass::OnPartyMatchInviteReceived);
        GetSessionInterface()->OnDestroySessionCompleteDelegates.AddUObject(this, &ThisClass::OnLeavePartyMatchComplete);
    }

    // Dummy dumb.
    if (FTutorialModuleGeneratedWidget* PlayOnlineButtonMetadata = 
        FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_play_online"))) 
    {
        PlayOnlineButtonMetadata->ValidateButtonAction.Unbind();
        PlayOnlineButtonMetadata->ValidateButtonAction.BindWeakLambda(this, [this]()
        {
            // Get current player.
            FUniqueNetIdPtr UserId = nullptr;
            if (GetIdentityInterface())
            {
                UserId = GetIdentityInterface()->GetUniquePlayerId(0);
            }

            // Only able to play online if other party member is not in any game session.
            return !IsGameSessionDifferFromParty(UserId);
        });
    }
}

void UPlayWithPartySubsystem::Deinitialize()
{
    Super::Deinitialize();

    if (GetSessionInterface()) 
    {
        GetSessionInterface()->OnMatchmakingStartedDelegates.RemoveAll(this);
        GetSessionInterface()->OnMatchmakingCompleteDelegates.RemoveAll(this);
        GetSessionInterface()->OnMatchmakingCanceledDelegates.RemoveAll(this);
        GetSessionInterface()->OnMatchmakingExpiredDelegates.RemoveAll(this);

        GetSessionInterface()->OnCreateSessionCompleteDelegates.RemoveAll(this);
        GetSessionInterface()->OnJoinSessionCompleteDelegates.RemoveAll(this);
        GetSessionInterface()->OnV2SessionInviteReceivedDelegates.RemoveAll(this);
        GetSessionInterface()->OnDestroySessionCompleteDelegates.RemoveAll(this);
    }
}

UAccelByteWarsOnlineSessionBase* UPlayWithPartySubsystem::GetOnlineSession() const
{
    if (!GetGameInstance()) 
    {
        return nullptr;
    }

    return Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
}

FOnlineSessionV2AccelBytePtr UPlayWithPartySubsystem::GetSessionInterface() const
{
    const UWorld* World = GetWorld();
    if (!ensure(World))
    {
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Online::GetSessionInterface(World));
}

FOnlineIdentityAccelBytePtr UPlayWithPartySubsystem::GetIdentityInterface() const
{
    const UWorld* World = GetWorld();
    if (!ensure(World))
    {
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Online::GetIdentityInterface(World));
}

FOnlineUserAccelBytePtr UPlayWithPartySubsystem::GetUserInterface() const
{
    const UWorld* World = GetWorld();
    if (!ensure(World))
    {
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineUserAccelByte>(Online::GetUserInterface(World));
}

FOnlinePresenceAccelBytePtr UPlayWithPartySubsystem::GetPresenceInterface() const
{
    const UWorld* World = GetWorld();
    if (!ensure(World))
    {
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlinePresenceAccelByte>(Online::GetPresenceInterface(World));
}

UPromptSubsystem* UPlayWithPartySubsystem::GetPromptSubystem()
{
    if (UAccelByteWarsGameInstance * GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()))
    {
        return GameInstance->GetSubsystem<UPromptSubsystem>();
    }

    return nullptr;
}

void UPlayWithPartySubsystem::OnStartPartyMatchmakingComplete()
{
    // Abort if not a party match.
    if (!GetSessionInterface()->IsInPartySession() ||
        GetOnlineSession()->GetPartyMembers().Num() <= 1)
    {
        return;
    }

    /* Show notification that the party matchmaking is started.
     * Only show the notification if a party member.*/
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInterface())
    {
        UserId = GetIdentityInterface()->GetUniquePlayerId(0);
    }
    if (GetOnlineSession()->IsPartyLeader(UserId))
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Party matchmaking started by party leader."));

    // TODO: Make it localizable.
    if (GetPromptSubystem())
    {
        GetPromptSubystem()->ShowLoading(FText::FromString("Party Matchmaking Started by Party Leader"));
    }
}

void UPlayWithPartySubsystem::OnPartyMatchmakingComplete(FName SessionName, bool bSucceeded)
{
    // Abort if not a party match.
    if (!GetSessionInterface()->IsInPartySession() ||
        GetOnlineSession()->GetPartyMembers().Num() <= 1 ||
        !GetOnlineSession()->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession).IsEqual(SessionName))
    {
        return;
    }

    /* Show notification that the party matchmaking is completed.
     * Only show the notification if a party member.*/
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInterface())
    {
        UserId = GetIdentityInterface()->GetUniquePlayerId(0);
    }
    if (GetOnlineSession()->IsPartyLeader(UserId))
    {
        return;
    }

    // TODO: Make it localizable.
    if (bSucceeded)
    {
        UE_LOG(LogTemp, Log, TEXT("Party matchmaking found. Currently joining the match."));

        if (GetPromptSubystem())
        {
            GetPromptSubystem()->ShowLoading(FText::FromString("Party Matchmaking Found, Joining Match"));
        }
    }
}

void UPlayWithPartySubsystem::OnPartyMatchmakingCanceled()
{
    // Abort if not a party match.
    if (!GetSessionInterface()->IsInPartySession() ||
        GetOnlineSession()->GetPartyMembers().Num() <= 1)
    {
        return;
    }

    /* Show notification that the party matchmaking is canceled.
     * Only show the notification if a party member.*/
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInterface())
    {
        UserId = GetIdentityInterface()->GetUniquePlayerId(0);
    }
    if (GetOnlineSession()->IsPartyLeader(UserId))
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Party Matchmaking is canceled by party leader."));

    if (GetPromptSubystem())
    {
        // TODO: Make it localizable.
        GetPromptSubystem()->HideLoading();
        GetPromptSubystem()->PushNotification(
            FText::FromString("Party Matchmaking is Canceled by Party Leader"),
            FString(""));
    }
}

void UPlayWithPartySubsystem::OnPartyMatchmakingExpired()
{
    // Abort if not a party match.
    if (!GetSessionInterface()->IsInPartySession() ||
        GetOnlineSession()->GetPartyMembers().Num() <= 1)
    {
        return;
    }

    /* Show notification that the party matchmaking is expired.
     * Only show the notification if a party member.*/
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInterface())
    {
        UserId = GetIdentityInterface()->GetUniquePlayerId(0);
    }
    if (GetOnlineSession()->IsPartyLeader(UserId))
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Party matchmaking expired."));

    // TODO: Make it localizable.
    if (GetPromptSubystem())
    {
        GetPromptSubystem()->HideLoading();
        GetPromptSubystem()->PushNotification(
            FText::FromString("Party Matchmaking Expired"),
            FString(""));
    }
}

void UPlayWithPartySubsystem::OnCreatePartyMatchComplete(FName SessionName, bool bSucceeded)
{
    // Abort if not a party match.
    if (!GetOnlineSession()->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession).IsEqual(SessionName) ||
        !GetSessionInterface()->IsInPartySession())
    {
        return;
    }

    // Update party member game session id.
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInterface())
    {
        UserId = GetIdentityInterface()->GetUniquePlayerId(0);

        UpdatePartyMemberGameSession(UserId);
    }

    // Not necessary to send party match invitation if there is only one member.
    if (GetOnlineSession()->GetPartyMembers().Num() <= 1) 
    {
        return;
    }

    /* Send party match invitation to each party members.
     * Only party leader can send party match invitation.*/
    if (!GetOnlineSession()->IsPartyLeader(UserId))
    {
        return;
    }
    for (auto& Member : GetOnlineSession()->GetPartyMembers())
    {
        if (GetOnlineSession()->IsPartyLeader(Member))
        {
            continue;
        }

        if (FUniqueNetIdAccelByteUserPtr MemberABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Member))
        {
            UE_LOG(LogTemp, Log, TEXT("Send party match invitation to: %s."), *MemberABId->GetAccelByteId());
            GetSessionInterface()->SendSessionInviteToFriend(UserId.ToSharedRef().Get(), SessionName, Member.Get());
        }
    }
}

void UPlayWithPartySubsystem::OnJoinPartyMatchComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    // Abort if not a party match.
    if (!GetOnlineSession()->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession).IsEqual(SessionName) ||
        !GetSessionInterface()->IsInPartySession())
    {
        return;
    }
    
    // Update party member game session id.
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInterface())
    {
        UserId = GetIdentityInterface()->GetUniquePlayerId(0);

        //UpdatePartyMemberGameSession(UserId);
    }

    // Not necessary to show notification if there is only one party member.
    if (GetOnlineSession()->GetPartyMembers().Num() <= 1)
    {
        return;
    }

    /* Show notification that failed to join party match.
     * Only show the notification if a party member.*/
    if (GetOnlineSession()->IsPartyLeader(UserId))
    {
        return;
    }
    if (Result != EOnJoinSessionCompleteResult::Type::Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to join party match."));

        // TODO: Make it localizable.
        if (GetPromptSubystem())
        {
            GetPromptSubystem()->HideLoading();
            GetPromptSubystem()->PushNotification(
                FText::FromString("Failed to Join Party Match"),
                FString(""));
        }

        return;
    }

    //FNamedOnlineSession* PartySession = GetSessionInterface()->GetPartySession();
    //if (!PartySession)
    //{
    //    return;
    //}

    //for (auto& Setting : PartySession->SessionSettings.MemberSettings)
    //{
    //    const FUniqueNetIdAccelByteUserRef UserABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Setting.Key);
    //    if (Setting.Value.Contains(GAME_SESSION_ID))
    //    {
    //        UE_LOG(LogTemp, Warning, TEXT("Nani Kore What: %s %s"), *UserABId->GetAccelByteId(), *Setting.Value[GAME_SESSION_ID].Data.ToString());
    //    }
    //}
}

void UPlayWithPartySubsystem::OnPartyMatchInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FOnlineSessionInviteAccelByte& Invite)
{
    // Abort if not a party match and if the invitation is not from the party leader.
    if (Invite.SessionType != EAccelByteV2SessionType::GameSession ||
        !GetOnlineSession()->IsPartyLeader(FromId.AsShared()) ||
        GetOnlineSession()->GetPartyMembers().Num() <= 1)
    {
        return;
    }

    // Abort if the receiver is the party leader.
    if (GetOnlineSession()->IsPartyLeader(UserId.AsShared()))
    {
        return;
    }

    // Join party match.
    const APlayerController* PC = GetOnlineSession()->GetPlayerControllerByUniqueNetId(UserId.AsShared());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot join a party match invitation from party leader. PlayerController is not valid."));
        return;
    }

    const int32 LocalUserNum = GetOnlineSession()->GetLocalUserNumFromPlayerController(PC);

    UE_LOG(LogTemp, Log, TEXT("Received a party match invitation from party leader. Joining the party match."));

    // TODO: Make it localizable.
    if (GetPromptSubystem())
    {
        GetPromptSubystem()->ShowLoading(FText::FromString("Joining Party Leader Match"));
    }

    GetOnlineSession()->JoinSession(LocalUserNum,
        GetOnlineSession()->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
        Invite.Session);
}

void UPlayWithPartySubsystem::OnLeavePartyMatchComplete(FName SessionName, bool bSucceeded)
{
    // Abort if not a party match.
    if (!GetOnlineSession()->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession).IsEqual(SessionName) ||
        !GetSessionInterface()->IsInPartySession())
    {
        return;
    }
    
    FNamedOnlineSession* PartySession = GetSessionInterface()->GetPartySession();
    if (!PartySession)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot clear party member game session. Party session is not valid."));
        return;
    }

    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInterface())
    {
        UserId = GetIdentityInterface()->GetUniquePlayerId(0);
    }
    if (!UserId)
    {
        return;
    }

    // Get game session from party session settings.
    // If there is no game session found, we don't need to clear it, simply abort.
    FSessionSettings* MemberPartySessionSettings = PartySession->SessionSettings.MemberSettings.Find(UserId.ToSharedRef());
    if (!MemberPartySessionSettings)
    {
        return;
    }

    // Clear saved game session from the party session settings.
    MemberPartySessionSettings->Remove(GAME_SESSION_ID);
    PartySession->SessionSettings.MemberSettings.Add(UserId.ToSharedRef(), *MemberPartySessionSettings);

    // Update party session to clear game session data.
    GetSessionInterface()->UpdateSession(NAME_PartySession, PartySession->SessionSettings);
}

bool UPlayWithPartySubsystem::IsGameSessionDifferFromParty(FUniqueNetIdPtr MemberUserId)
{
    bool bResult = false;

    // Abort if interfaces and data is not valid.
    if (!GetSessionInterface() || !GetOnlineSession() || !MemberUserId)
    {
        return bResult;
    }

    // Abort if not in a party session.
    FNamedOnlineSession* PartySession = GetSessionInterface()->GetPartySession();
    if (!PartySession)
    {
        return bResult;
    }

    // Get current game session id.
    FString GameSessionId;
    FNamedOnlineSession* GameSession = GetSessionInterface()->GetNamedSession(NAME_GameSession);
    if (GameSession)
    {
        GameSessionId = GameSession->GetSessionIdStr();
    }

    // Check whether the current game session is different from the party.
    FString MemberGameSessionId;
    FSessionSettings* MemberSessionSettings = nullptr;
    FOnlineSessionSetting* MemberGameSessionData = nullptr;
    for (auto& Member : GetOnlineSession()->GetPartyMembers())
    {
        // Skip if the member is the current player.
        if (Member.Get() == MemberUserId.ToSharedRef().Get()) 
        {
            continue;
        }

        // If member settings is not found, assume its on other game session.
        MemberSessionSettings = PartySession->SessionSettings.MemberSettings.Find(Member);
        if (!MemberSessionSettings)
        {
            bResult = true;
            break;
        }

        // Get member game session id.
        MemberGameSessionId = FString("");
        //for (auto& Setting : *MemberSessionSettings)
        //{
        //    const FUniqueNetIdAccelByteUserRef UserABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Member);
        //    UE_LOG(LogTemp, Warning, TEXT("Nani Kore Wow: %s %s"), *UserABId->GetAccelByteId(), *Setting.Value.Data.ToString());
        //}

        MemberGameSessionData = MemberSessionSettings->Find(GAME_SESSION_ID);
        if (MemberGameSessionData)
        {
            MemberGameSessionId = MemberGameSessionData->Data.ToString();
        }

        // Check if current game session is the same as the party.
        if (!GameSessionId.Equals(MemberGameSessionId))
        {
            bResult = true;
            break;
        }
    }

    return bResult;
}

void UPlayWithPartySubsystem::UpdatePartyMemberGameSession(FUniqueNetIdPtr MemberUserId)
{
    if (!MemberUserId) 
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot update party member game session. Party member is not valid."));
        return;
    }

    FNamedOnlineSession* GameSession = GetSessionInterface()->GetNamedSession(NAME_GameSession);
    if (!GameSession)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot update party member game session. Game session is not valid."));
        return;
    }

    FNamedOnlineSession* PartySession = GetSessionInterface()->GetPartySession();
    if (!PartySession)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot update party member game session. Party session is not valid."));
        return;
    }

    // Construct game session data.
    FOnlineSessionSetting MemberGameSessionData;
    MemberGameSessionData.Data = GameSession->GetSessionIdStr();

    // Update game session data in the party session settings.
    FSessionSettings* MemberPartySessionSettings = PartySession->SessionSettings.MemberSettings.Find(MemberUserId.ToSharedRef());
    if (!MemberPartySessionSettings)
    {
        MemberPartySessionSettings = new FSessionSettings();
    }
    MemberPartySessionSettings->Remove(GAME_SESSION_ID);
    MemberPartySessionSettings->Add(GAME_SESSION_ID, MemberGameSessionData);
    PartySession->SessionSettings.MemberSettings.Add(MemberUserId.ToSharedRef(), *MemberPartySessionSettings);

    //GetSessionInterface()->AddOnUpdateSessionCompleteDelegate_Handle(
    //    FOnUpdateSessionCompleteDelegate::CreateWeakLambda(this, [this](FName SessionName, bool bSucceeded)
    //    {
    //        if (SessionName != NAME_PartySession || !bSucceeded)
    //        {
    //            return;
    //        }

    //        FNamedOnlineSession* PartySession = GetSessionInterface()->GetPartySession();
    //        if (!PartySession)
    //        {
    //            return;
    //        }

    //        for (auto& Setting : PartySession->SessionSettings.MemberSettings)
    //        {
    //            const FUniqueNetIdAccelByteUserRef UserABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Setting.Key);
    //            if (Setting.Value.Contains(GAME_SESSION_ID)) 
    //            {
    //                UE_LOG(LogTemp, Warning, TEXT("Nani Kore Damn: %s %s"), *UserABId->GetAccelByteId(), *Setting.Value[GAME_SESSION_ID].Data.ToString());
    //            }
    //        }
    //    }
    //));

    // Update party session to store game session data.
    GetSessionInterface()->UpdateSession(NAME_PartySession, PartySession->SessionSettings);
}
