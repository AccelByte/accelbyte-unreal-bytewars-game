// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "InviteFriendsWidget.h"

#include "CommonButtonBase.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Play/PlayingWithFriends/PlayingWithFriendsSubsystem.h"

void UInviteFriendsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	bool bShowButton = false;

	if (const UPlayingWithFriendsSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UPlayingWithFriendsSubsystem>())
	{
		bShowButton =
			Subsystem->IsInMatchSessionGameSession() &&
			UTutorialModuleUtility::IsTutorialModuleActive(FPrimaryAssetId{ "TutorialModule:FRIENDSESSENTIALS" }, this);
	}

	SetVisibility(bShowButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	Btn_Friends->OnClicked().AddUObject(this, &ThisClass::OpenFriendsMenu);
}

void UInviteFriendsWidget::NativeOnDeactivated()
{
	Btn_Friends->OnClicked().RemoveAll(this);

	Super::NativeOnDeactivated();
}

void UInviteFriendsWidget::OpenFriendsMenu() const
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	if (!BaseUIWidget)
	{
		return;
	}

	const UTutorialModuleDataAsset* DataAsset = UTutorialModuleUtility::GetTutorialModuleDataAsset(
		FPrimaryAssetId{ "TutorialModule:FRIENDSESSENTIALS" },
		this,
		true);
	if (!DataAsset)
	{
		return;
	}

	BaseUIWidget->PushWidgetToStack(
		EBaseUIStackType::Menu,
		DataAsset->IsStarterModeActive() ? FriendsWidgetClass_Starter : FriendsWidgetClass);
}
