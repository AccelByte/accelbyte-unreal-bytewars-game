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

    if (GetABSessionInt()) 
    {
        GetABSessionInt()->OnMatchmakingStartedDelegates.AddUObject(this, &ThisClass::OnStartPartyMatchmakingComplete);
        GetABSessionInt()->OnMatchmakingCompleteDelegates.AddUObject(this, &ThisClass::OnPartyMatchmakingComplete);
        GetABSessionInt()->OnMatchmakingCanceledDelegates.AddUObject(this, &ThisClass::OnPartyMatchmakingCanceled);
        GetABSessionInt()->OnMatchmakingExpiredDelegates.AddUObject(this, &ThisClass::OnPartyMatchmakingExpired);

        GetABSessionInt()->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreatePartyMatchComplete);
        GetABSessionInt()->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinPartyMatchComplete);
        GetABSessionInt()->OnV2SessionInviteReceivedDelegates.AddUObject(this, &ThisClass::OnPartyMatchInviteReceived);
    }

    // Only shows Play Online button on Party Leader.
    //if (FTutorialModuleGeneratedWidget* PlayOnlineButtonMetadata = 
    //    FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_play_online"))) 
    //{
    //    PlayOnlineButtonMetadata->ValidateButtonAction.Unbind();
    //    PlayOnlineButtonMetadata->ValidateButtonAction.BindWeakLambda(this, [this]()
    //    {
    //        FUniqueNetIdPtr UserId = nullptr;
    //        if (GetIdentityInt())
    //        {
    //            UserId = GetIdentityInt()->GetUniquePlayerId(0);
    //        }

    //        const bool bResult = IsPartyLeader(UserId);
    //        if (!bResult && GetPromptSubystem()) 
    //        {
    //            // TODO: Make it localizable.
    //            GetPromptSubystem()->PushNotification(
    //                FText::FromString("Only Party Leader is allowed to start online plays"),
    //                FString(""));
    //        }
    //        return bResult;
    //    });
    //}

    // Cannot browse match when in a party.
    //if (FTutorialModuleGeneratedWidget* BrowseMatchButtonMetadata =
    //    FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_browse_match")))
    //{
    //    BrowseMatchButtonMetadata->ValidateButtonAction.Unbind();
    //    BrowseMatchButtonMetadata->ValidateButtonAction.BindWeakLambda(this, [this]()
    //    {
    //        const bool bResult = GetPartyMembers().Num() <= 1;
    //        if (!bResult && GetPromptSubystem())
    //        {
    //            // TODO: Make it localizable.
    //            GetPromptSubystem()->PushNotification(
    //                FText::FromString("Cannot browse match when you are in a party."),
    //                FString(""));
    //        }
    //        return bResult;
    //    });
    //}
}

void UPlayWithPartySubsystem::Deinitialize()
{
    Super::Deinitialize();

    if (GetABSessionInt()) 
    {
        GetABSessionInt()->OnMatchmakingStartedDelegates.RemoveAll(this);
        GetABSessionInt()->OnMatchmakingCompleteDelegates.RemoveAll(this);
        GetABSessionInt()->OnMatchmakingCanceledDelegates.RemoveAll(this);
        GetABSessionInt()->OnMatchmakingExpiredDelegates.RemoveAll(this);

        GetABSessionInt()->OnCreateSessionCompleteDelegates.RemoveAll(this);
        GetABSessionInt()->OnJoinSessionCompleteDelegates.RemoveAll(this);
        GetABSessionInt()->OnV2SessionInviteReceivedDelegates.RemoveAll(this);
    }
}

UAccelByteWarsOnlineSessionBase* UPlayWithPartySubsystem::GetOnlineSession()
{
    if (!GetGameInstance()) 
    {
        return nullptr;
    }

    return Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
}

FOnlineSessionV2AccelBytePtr UPlayWithPartySubsystem::GetABSessionInt()
{
    const UWorld* World = GetWorld();
    if (!ensure(World))
    {
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Online::GetSessionInterface(World));
}

IOnlineIdentityPtr UPlayWithPartySubsystem::GetIdentityInt() const
{
    const UWorld* World = GetWorld();
    if (!ensure(World))
    {
        return nullptr;
    }

    return Online::GetIdentityInterface(World);
}

IOnlineUserPtr UPlayWithPartySubsystem::GetUserInt() const
{
    const UWorld* World = GetWorld();
    if (!ensure(World))
    {
        return nullptr;
    }

    return Online::GetUserInterface(World);
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
    if (!GetABSessionInt()->IsInPartySession() ||
        GetOnlineSession()->GetPartyMembers().Num() <= 1)
    {
        return;
    }

    /* Show notification that the party matchmaking is started.
     * Only show the notification if a party member.*/
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInt())
    {
        UserId = GetIdentityInt()->GetUniquePlayerId(0);
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
    if (!GetABSessionInt()->IsInPartySession() ||
        GetOnlineSession()->GetPartyMembers().Num() <= 1 ||
        !GetOnlineSession()->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession).IsEqual(SessionName))
    {
        return;
    }

    /* Show notification that the party matchmaking is completed.
     * Only show the notification if a party member.*/
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInt())
    {
        UserId = GetIdentityInt()->GetUniquePlayerId(0);
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
    if (!GetABSessionInt()->IsInPartySession() ||
        GetOnlineSession()->GetPartyMembers().Num() <= 1)
    {
        return;
    }

    /* Show notification that the party matchmaking is canceled.
     * Only show the notification if a party member.*/
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInt())
    {
        UserId = GetIdentityInt()->GetUniquePlayerId(0);
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
    if (!GetABSessionInt()->IsInPartySession() ||
        GetOnlineSession()->GetPartyMembers().Num() <= 1)
    {
        return;
    }

    /* Show notification that the party matchmaking is expired.
     * Only show the notification if a party member.*/
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInt())
    {
        UserId = GetIdentityInt()->GetUniquePlayerId(0);
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
    if (!GetABSessionInt()->IsInPartySession() ||
        GetOnlineSession()->GetPartyMembers().Num() <= 1 ||
        !GetOnlineSession()->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession).IsEqual(SessionName))
    {
        return;
    }

    /* Send party match invitation to each party members.
     * Only party leader can send party match invitation.*/
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInt())
    {
        UserId = GetIdentityInt()->GetUniquePlayerId(0);
    }
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
            GetABSessionInt()->SendSessionInviteToFriend(UserId.ToSharedRef().Get(), SessionName, Member.Get());
        }
    }
}

void UPlayWithPartySubsystem::OnJoinPartyMatchComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    // Abort if not a party match.
    if (!GetABSessionInt()->IsInPartySession() ||
        GetOnlineSession()->GetPartyMembers().Num() <= 1 ||
        !GetOnlineSession()->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession).IsEqual(SessionName))
    {
        return;
    }

    /* Show notification that failed to join party match.
     * Only show the notification if a party member.*/
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInt())
    {
        UserId = GetIdentityInt()->GetUniquePlayerId(0);
    }
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
    }
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