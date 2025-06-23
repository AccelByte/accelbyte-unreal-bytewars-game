// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PartyWidget.h"

#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "Play/SessionEssentials/SessionEssentialsModel.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"

#include "TutorialModuleUtilities/StartupSubsystem.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#include "PartyWidgetEntry.h"
#include "Components/HorizontalBox.h"
#include "CommonButtonBase.h"

#define PARTY_WIDGET_ENTRY_CLASS UPartyWidgetEntry

void UPartyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PartyOnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
	ensure(PartyOnlineSession);
}

// @@@SNIPSTART PartyWidget.cpp-NativeOnActivated
// @@@MULTISNIP UE5_2 {"selectedLines": ["1-17", "24-31", "33-40"]}
void UPartyWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Leave->OnClicked().Clear();
	Btn_Leave->OnClicked().AddUObject(this, &ThisClass::OnLeaveButtonClicked);
	
	// Update the displayed party members on any changes.
	PartyOnlineSession->GetOnCreatePartyCompleteDelegates()->AddWeakLambda(this, [this](FName SessionName, bool bWasSuccessful)
	{
		DisplayParty();
	});
	PartyOnlineSession->GetOnLeavePartyCompleteDelegates()->AddWeakLambda(this, [this](FName SessionName, bool bWasSuccessful)
	{
		DisplayParty();
	});

#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
	PartyOnlineSession->GetOnPartyMembersChangeDelegates()->AddWeakLambda(this, [this](FName SessionName, const FUniqueNetId& Participant, bool bJoined)
	{
		DisplayParty();
	});
#else
	PartyOnlineSession->GetOnPartyMemberJoinedDelegates()->AddWeakLambda(this, [this](FName SessionName, const FUniqueNetId& Participant)
	{
		DisplayParty();
	});
	PartyOnlineSession->GetOnPartyMemberLeftDelegates()->AddWeakLambda(this, [this](FName SessionName, const FUniqueNetId& Participant, EOnSessionParticipantLeftReason Reason)
	{
		DisplayParty();
	});
#endif

	PartyOnlineSession->GetOnPartySessionUpdateReceivedDelegates()->AddWeakLambda(this, [this](FName SessionName)
	{
		DisplayParty();
	});

	DisplayParty();
}
// @@@SNIPEND

// @@@SNIPSTART PartyWidget.cpp-NativeOnDeactivated
// @@@MULTISNIP UE5_2 {"selectedLines": ["1-8", "12-13", "15-19"]}
void UPartyWidget::NativeOnDeactivated()
{
	Btn_Leave->OnClicked().Clear();

	// Clear online delegates.
	PartyOnlineSession->GetOnCreatePartyCompleteDelegates()->RemoveAll(this);
	PartyOnlineSession->GetOnLeavePartyCompleteDelegates()->RemoveAll(this);

#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
	PartyOnlineSession->GetOnPartyMembersChangeDelegates()->RemoveAll(this);
#else
	PartyOnlineSession->GetOnPartyMemberJoinedDelegates()->RemoveAll(this);
	PartyOnlineSession->GetOnPartyMemberLeftDelegates()->RemoveAll(this);
#endif

	PartyOnlineSession->GetOnPartySessionUpdateReceivedDelegates()->RemoveAll(this);

	Super::NativeOnDeactivated();
}
// @@@SNIPEND

// @@@SNIPSTART PartyWidget.cpp-DisplayParty
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-5", "98"]}
void UPartyWidget::DisplayParty()
{
	// Show loading.
	Btn_Leave->SetVisibility(ESlateVisibility::Collapsed);
	Ws_Party->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	// Collect party member user IDs.
	const FUniqueNetIdPtr UserId = PartyOnlineSession->GetLocalPlayerUniqueNetId(GetOwningPlayer());
	TArray<FUniqueNetIdRef> PartyMemberUserIds = PartyOnlineSession->GetPartyMembers();

	// If not in any party, then only include player user ID to query its information.
	if (PartyMemberUserIds.IsEmpty())
	{
		PartyMemberUserIds.Add(UserId.ToSharedRef());
	}

	// Query and display party member information.
	if (UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStartupSubsystem>())
	{
		StartupSubsystem->QueryUserInfo(
			PartyOnlineSession->GetLocalUserNumFromPlayerController(GetOwningPlayer()),
			PartyMemberUserIds,
			FOnQueryUsersInfoCompleteDelegate::CreateWeakLambda(this, [this](
				const FOnlineError& Error,
				const TArray<TSharedPtr<FUserOnlineAccountAccelByte>> UsersInfo)
			{
				// If party members are changed during query, then re-query the party members' information.
				const bool bIsInParty = !PartyOnlineSession->GetPartyMembers().IsEmpty();
				if (bIsInParty)
				{
					TArray<FString> QueriedUserIds{};
					for (const TSharedPtr<FUserOnlineAccountAccelByte>& UserInfo : UsersInfo)
					{
						if (const FUniqueNetIdAccelByteUserPtr ABUserId = UTutorialModuleOnlineUtility::GetAccelByteUserId(UserInfo->GetUserId()))
						{
							QueriedUserIds.Add(ABUserId->GetAccelByteId());
						}
					}

					for (const FUniqueNetIdPtr PartyMember : PartyOnlineSession->GetPartyMembers()) 
					{
						const FUniqueNetIdAccelByteUserPtr ABUserId = UTutorialModuleOnlineUtility::GetAccelByteUserId(PartyMember);
						if (ABUserId->IsValid())
						{
							continue;
						}

						if (!QueriedUserIds.Contains(ABUserId->GetAccelByteId()))
						{
							DisplayParty();
							return;
						}
					}
				}

				// Request failed.
				if (!Error.bSucceeded)
				{
					Ws_Party->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
					return;
				}

				// Clean up last party member list.
				Ws_Party->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
				Hb_Party->ClearChildren();
				Btn_Leave->SetVisibility(ESlateVisibility::Visible);

				// Display party members.
				for (int32 i = 0; i < MaxPartyMembers; i++)
				{
					if (i < UsersInfo.Num() && !UsersInfo[i])
					{
						continue;
					}

					const TWeakObjectPtr<PARTY_WIDGET_ENTRY_CLASS> PartyMemberEntry =
						MakeWeakObjectPtr<PARTY_WIDGET_ENTRY_CLASS>(
							CreateWidget<PARTY_WIDGET_ENTRY_CLASS>(this, PartyWidgetEntryClass.Get()));
					Hb_Party->AddChild(PartyMemberEntry.Get());

					// Display party member information.
					if (i < UsersInfo.Num())
					{
						/* Mark party member as leader if valid.
						 * If not in any party, then assume the player as the leader.*/
						PartyMemberEntry->SetPartyMember(
							*UsersInfo[i],
							PartyOnlineSession->IsPartyLeader(UsersInfo[i]->GetUserId()) || !bIsInParty);
					}
					// Display button to add more party members.
					else
					{
						PartyMemberEntry->ResetPartyMember();
					}
				}
			}));
	}
}
// @@@SNIPEND

// @@@SNIPSTART PartyWidget.cpp-OnLeaveButtonClicked
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "5-6"]}
void UPartyWidget::OnLeaveButtonClicked()
{
	// Leave current party.
	PartyOnlineSession->LeaveParty(PartyOnlineSession->GetLocalUserNumFromPlayerController(GetOwningPlayer()));
	DeactivateWidget();
}
// @@@SNIPEND

#undef PARTY_WIDGET_ENTRY_CLASS
