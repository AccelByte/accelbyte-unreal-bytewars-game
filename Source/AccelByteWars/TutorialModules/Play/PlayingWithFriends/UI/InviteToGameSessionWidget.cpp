// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InviteToGameSessionWidget.h"

#include "CommonButtonBase.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Play/PlayingWithFriends/PlayingWithFriendsSubsystem.h"

// @@@SNIPSTART InviteToGameSessionWidget.cpp-NativeOnActivated
// @@@MULTISNIP Subsystem {"selectedLines": ["1-2", "5-6", "21"]}
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "19-21"]}
void UInviteToGameSessionWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Subsystem = GetGameInstance()->GetSubsystem<UPlayingWithFriendsSubsystem>();
	check(Subsystem)

	if(Subsystem->IsInMatchSessionGameSession())
	{
		const TWeakObjectPtr<AAccelByteWarsInGameGameState> ABInGameGameState = 
				MakeWeakObjectPtr<AAccelByteWarsInGameGameState>(Cast<AAccelByteWarsInGameGameState>(GetWorld()->GetGameState()));		
		SetVisibility(ABInGameGameState != nullptr && ABInGameGameState->HasGameStarted() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}

	Btn_Invite->SetIsEnabled(true);
	Btn_Invite->OnClicked().AddUObject(this, &ThisClass::InviteToSession);
}
// @@@SNIPEND

// @@@SNIPSTART InviteToGameSessionWidget.cpp-NativeOnDeactivated
void UInviteToGameSessionWidget::NativeOnDeactivated()
{
	Btn_Invite->OnClicked().RemoveAll(this);

	if (InviteDelayTimerHandle.IsValid())
	{
		GetGameInstance()->GetTimerManager().ClearTimer(InviteDelayTimerHandle);
	}

	Super::NativeOnDeactivated();
}
// @@@SNIPEND

#pragma region "Helper"
UFriendData* UInviteToGameSessionWidget::GetFriendDataFromParentWidget()
{
	if (const UFriendDetailsWidget* Parent = GetFirstOccurenceOuter<UFriendDetailsWidget>())
	{
		return Parent->GetCachedFriendData();
	}

	return nullptr;
}
#pragma endregion 

// @@@SNIPSTART InviteToGameSessionWidget.cpp-InviteToSession
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-19", "22"]}
void UInviteToGameSessionWidget::InviteToSession()
{
	const UFriendData* FriendData = GetFriendDataFromParentWidget();
	if (!FriendData)
	{
		return;
	}

	// disable button for 5 seconds to avoid spamming
	Btn_Invite->SetIsEnabled(false);
	GetGameInstance()->GetTimerManager().SetTimer(
		InviteDelayTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			Btn_Invite->SetIsEnabled(true);
		}),
		5.0f,
		false,
		5.0f);

	Subsystem->SendGameSessionInvite(GetOwningPlayer(), FriendData->UserId);
}
// @@@SNIPEND