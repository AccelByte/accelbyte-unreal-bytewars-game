// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/PartyEssentials/UI/PartyWidget.h"

#include "TutorialModules/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "TutorialModules/SessionEssentials/SessionEssentialsModel.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "TutorialModules/PartyEssentials/UI/PartyWidgetEntry.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Components/HorizontalBox.h"
#include "CommonButtonBase.h"

void UPartyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PartyOnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
	ensure(PartyOnlineSession);
}

void UPartyWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Leave->OnClicked().AddUObject(this, &ThisClass::OnLeaveButtonClicked);
	
	// Update the displayed party members on any changes.
	PartyOnlineSession->GetOnCreatePartyCompleteDelegates()->AddWeakLambda(this, [this](FName SessionName, bool bWasSuccessful)
	{
		DisplayParty();
	});
	PartyOnlineSession->GetOnPartyMembersChangeDelegates()->AddWeakLambda(this, [this](FName SessionName, const FUniqueNetId& Participant, bool bJoined)
	{
		DisplayParty();
	});
	PartyOnlineSession->GetOnPartySessionUpdateReceivedDelegates()->AddWeakLambda(this, [this](FName SessionName)
	{
		DisplayParty();
	});

	DisplayParty();
}

void UPartyWidget::NativeOnDeactivated()
{
	Btn_Leave->OnClicked().Clear();

	// Clear online delegates.
	PartyOnlineSession->GetOnCreatePartyCompleteDelegates()->RemoveAll(this);
	PartyOnlineSession->GetOnPartyMembersChangeDelegates()->RemoveAll(this);
	PartyOnlineSession->GetOnPartySessionUpdateReceivedDelegates()->RemoveAll(this);

	Super::NativeOnDeactivated();
}

void UPartyWidget::DisplayParty()
{
	// Not in party.
	if (!PartyOnlineSession->IsInParty(PartyOnlineSession->GetLocalPlayerUniqueNetId(GetOwningPlayer())))
	{
		Ws_Party->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
		return;
	}

	Btn_Leave->SetVisibility(ESlateVisibility::Collapsed);
	Ws_Party->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	// Query and display party member information.
	PartyOnlineSession->QueryUserInfo(
		PartyOnlineSession->GetLocalUserNumFromPlayerController(GetOwningPlayer()), 
		PartyOnlineSession->GetPartyMembers(),
		FOnQueryUsersInfoComplete::CreateWeakLambda(this, [this]
		(const bool bSucceeded, const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
		{
			// If party members are changed during query, requery the party members information.
			if (UsersInfo.Num() != PartyOnlineSession->GetPartyMembers().Num()) 
			{
				DisplayParty();
				return;
			}
			for (int32 i = 0; i < UsersInfo.Num(); i++) 
			{
				if (!UsersInfo[i]->GetUserId()->IsValid() ||
					!PartyOnlineSession->GetPartyMembers()[i]->IsValid() ||
					PartyOnlineSession->GetPartyMembers()[i].Get() != UsersInfo[i]->GetUserId().Get())
				{
					DisplayParty();
					return;
				}
			}

			// Request failed.
			if (!bSucceeded)
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

				const TWeakObjectPtr<UPartyWidgetEntry> PartyMemberEntry =
					MakeWeakObjectPtr<UPartyWidgetEntry>(CreateWidget<UPartyWidgetEntry>(this, PartyWidgetEntryClass.Get()));
				Hb_Party->AddChild(PartyMemberEntry.Get());

				// Display party member information.
				if (i < UsersInfo.Num())
				{
					PartyMemberEntry->SetPartyMember(*UsersInfo[i], PartyOnlineSession->IsPartyLeader(UsersInfo[i]->GetUserId()));
				}
				// Display button to add more party member.
				else
				{
					PartyMemberEntry->ResetPartyMember();
				}
			}
		}
	));
}

void UPartyWidget::OnLeaveButtonClicked()
{
	// Leave current party.
	PartyOnlineSession->LeaveParty(PartyOnlineSession->GetLocalUserNumFromPlayerController(GetOwningPlayer()));
	DeactivateWidget();
}