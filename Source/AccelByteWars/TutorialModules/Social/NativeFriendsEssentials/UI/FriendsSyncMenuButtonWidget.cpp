// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "FriendsSyncMenuButtonWidget.h"

#include "CommonButtonBase.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Social/NativeFriendsEssentials/NativeFriendsEssentialsLog.h"

void UFriendsSyncMenuButtonWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Get Online Subsystem and make sure it's valid.
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		Btn_FriendsSync->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// Cast Online Subsystem to AccelByte OSS.
	FOnlineSubsystemAccelByte* ABSubsystem = static_cast<FOnlineSubsystemAccelByte*>(Subsystem);
	if (!ensure(ABSubsystem))
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("The AccelByte online subsystem is invalid."));
		Btn_FriendsSync->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// Check if native subsystem active or not
	const IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::Get(ABSubsystem->GetNativePlatformName());
	if (!NativeSubsystem)
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Log, TEXT("The native online subsystem is invalid. Button is set to collapsed."))
		Btn_FriendsSync->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	Btn_FriendsSync->SetVisibility(ESlateVisibility::Visible);
	Btn_FriendsSync->OnClicked().AddUObject(this, &ThisClass::OpenFriendsSyncWidget);
}

void UFriendsSyncMenuButtonWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_FriendsSync->OnClicked().RemoveAll(this);
}

void UFriendsSyncMenuButtonWidget::OpenFriendsSyncWidget() const
{
	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);

	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);

	// Get tutorial module
	const UTutorialModuleDataAsset* NativeFriendModule = UTutorialModuleUtility::GetTutorialModuleDataAsset(NativeFriendAssetId, this);
	if (!ensure(NativeFriendModule))
	{
		UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("Can't find Native friend module. Openning Friend Sync menu canceled."))
		return;
	}

	// Push Native Friend widget
	BaseUIWidget->PushWidgetToStack(
		EBaseUIStackType::Menu,
		NativeFriendModule->GetTutorialModuleUIClass());
}
