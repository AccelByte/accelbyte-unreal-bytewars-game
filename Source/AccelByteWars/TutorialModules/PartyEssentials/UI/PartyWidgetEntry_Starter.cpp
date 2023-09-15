// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/PartyEssentials/UI/PartyWidgetEntry_Starter.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/MultiplayerEntries/PlayerEntryWidget.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/Utilities/AccelByteWarsUtility.h"

#include "Components/WidgetSwitcher.h"
#include "CommonButtonBase.h"

#include "TutorialModules/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "TutorialModules/Module-1/TutorialModuleOnlineUtility.h"
#include "TutorialModules/Module-8/UI/FriendDetailsWidget.h"
#include "TutorialModules/Module-8/UI/FriendDetailsWidget_Starter.h"

void UPartyWidgetEntry_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_AddPartyMember->OnClicked().Clear();
	Btn_AddPartyMember->OnClicked().AddUObject(this, &ThisClass::AddPartyMember);

	Btn_PartyMember->OnClicked().Clear();
	Btn_PartyMember->OnClicked().AddUObject(this, &ThisClass::OpenPlayerActionMenu);

	PartyOnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
}

void UPartyWidgetEntry_Starter::SetPartyMember(const FUserOnlineAccountAccelByte& PartyMember, const bool bIsLeader)
{
	// Display party member information.
	Ws_PartyMemberState->SetActiveWidget(W_PartyMemberPanel);

	// TODO: Set and display party member information.
}

void UPartyWidgetEntry_Starter::ResetPartyMember()
{
	// Display add party member button.
	SetPartyMemberColor(FLinearColor::White);
	Ws_PartyMemberState->SetActiveWidget(Btn_AddPartyMember);
}

void UPartyWidgetEntry_Starter::AddPartyMember()
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot open friends menu. Game Instance is not valid."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget)
	{
		UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot open friends menu. Base UI is not valid."));
		return;
	}

	if (!FriendsEssentialsModule || !FriendsEssentialsModule->IsActiveAndDependenciesChecked())
	{
		UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot open friends menu. Tutorial module dependencies are not active."));
		return;
	}

	// Display friends widget.
	if (!FriendsEssentialsModule->IsStarterModeActive())
	{
		BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, FriendsWidgetClass);
	}
	else
	{
		BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, FriendsWidgetStarterClass);
	}
}

void UPartyWidgetEntry_Starter::OpenPlayerActionMenu()
{
	if (!PartyOnlineSession)
	{
		UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot open player action menu. Party Online Session is not valid."));
		return;
	}

	if (!CachedFriendData || !CachedFriendData->UserId)
	{
		UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot open player action menu. Cached party member NetId is not valid."));
		return;
	}

	// Cannot open player action menu for the current logged-in player.
	const FUniqueNetIdPtr UserId = PartyOnlineSession->GetLocalPlayerUniqueNetId(GetOwningPlayer());
	const FUniqueNetIdPtr CachedUserId = CachedFriendData->UserId;
	if (!UserId || !CachedUserId)
	{
		UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot open player action menu. Cached party member NetId is not valid."));
		return;
	}
	if (UserId->AsShared().Get() == CachedUserId.ToSharedRef().Get())
	{
		UE_LOG_PARTYESSENTIALS(Log, TEXT("Cannot open player action menu for current logged-in player."));
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot open player action menu. Game Instance is not valid."));
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!BaseUIWidget)
	{
		UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot open player action menu. Base UI is not valid."));
		return;
	}

	if (!FriendsEssentialsModule || !FriendsEssentialsModule->IsActiveAndDependenciesChecked())
	{
		UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot open player action menu. Tutorial module dependencies are not active."));
		return;
	}

	// Display player action menu widget.
	if (!FriendsEssentialsModule->IsStarterModeActive())
	{
		UFriendDetailsWidget* DetailsWidget =
			Cast<UFriendDetailsWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, FriendDetailsWidgetClass));
		DetailsWidget->InitData(CachedFriendData);
	}
	else
	{
		UFriendDetailsWidget_Starter* DetailsWidget =
			Cast<UFriendDetailsWidget_Starter>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, FriendDetailsWidgetStarterClass));
		DetailsWidget->InitData(CachedFriendData);
	}
}