// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PartyWidgetEntry.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/MultiplayerEntries/PlayerEntryWidget.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/Utilities/AccelByteWarsUtility.h"

#include "Components/WidgetSwitcher.h"
#include "CommonButtonBase.h"

#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"

void UPartyWidgetEntry::NativeConstruct()
{
	Super::NativeConstruct();
	
	Btn_AddPartyMember->OnClicked().Clear();
	Btn_AddPartyMember->OnClicked().AddUObject(this, &ThisClass::AddPartyMember);

	Btn_PartyMember->OnClicked().Clear();
	Btn_PartyMember->OnClicked().AddUObject(this, &ThisClass::OpenPlayerActionMenu);

	PartyOnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
}

void UPartyWidgetEntry::SetPartyMember(const FUserOnlineAccountAccelByte& PartyMember, const bool bIsLeader)
{
	// Display party member information.
	Ws_PartyMemberState->SetActiveWidget(W_PartyMemberPanel);

	// Set display name.
	if (PartyMember.GetDisplayName().IsEmpty())
	{
		const FUniqueNetIdAccelByteUserRef MemberABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(PartyMember.GetUserId());
		W_PartyMember->SetUsername(FText::FromString(UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(MemberABId.Get())));
	}
	else
	{
		W_PartyMember->SetUsername(FText::FromString(PartyMember.GetDisplayName()));
	}

	// Set avatar
	FString AvatarURL;
	PartyMember.GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);
	W_PartyMember->SetAvatar(AvatarURL);

	// Set color tint.
	const FLinearColor MemberColor = bIsLeader ? PartyLeaderColor : PartyMemberColor;
	SetPartyMemberColor(MemberColor);
	W_PartyMember->SetTextColor(MemberColor);
	W_PartyMember->SetAvatarTint(AvatarURL.IsEmpty() ? MemberColor : FLinearColor::White);

	// Set cached party member data as friend data.
	// Will be used to open player action menu widget.
	if (!CachedFriendData)
	{
		CachedFriendData = NewObject<UFriendData>();
	}
	CachedFriendData->UserId = PartyMember.GetUserId();
	CachedFriendData->DisplayName = PartyMember.GetDisplayName();
	CachedFriendData->AvatarURL = AvatarURL;
}

void UPartyWidgetEntry::ResetPartyMember()
{
	// Display add party member button.
	SetPartyMemberColor(FLinearColor::White);
	Ws_PartyMemberState->SetActiveWidget(Btn_AddPartyMember);
}

void UPartyWidgetEntry::AddPartyMember()
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
	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, FriendsWidgetClass);
}

void UPartyWidgetEntry::OpenPlayerActionMenu()
{
	if (!PartyOnlineSession)
	{
		UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot open player action menu. Party Online Session is not valid."));
		return;
	}

	if (!CachedFriendData || 
		!CachedFriendData->UserId ||
		!CachedFriendData->UserId.IsValid() ||
		CachedFriendData->UserId.GetSharedReferenceCount() <= 0)
	{
		UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot open player action menu. Cached party member NetId is not valid."));
		return;
	}

	// Cannot open player action menu for the current logged-in player.
	const FUniqueNetIdPtr UserId = PartyOnlineSession->GetLocalPlayerUniqueNetId(GetOwningPlayer());
	const FUniqueNetIdPtr CachedUserId = CachedFriendData->UserId;
	if (!UserId || !UserId.IsValid() ||
		!CachedUserId || !CachedUserId.IsValid())
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
	UFriendDetailsWidget* DetailsWidget = 
		Cast<UFriendDetailsWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, FriendDetailsWidgetClass));
	if (!DetailsWidget) 
	{
		UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot open player action menu. Player action menu is not valid."));
		return;
	}

	DetailsWidget->InitData(CachedFriendData);
}
