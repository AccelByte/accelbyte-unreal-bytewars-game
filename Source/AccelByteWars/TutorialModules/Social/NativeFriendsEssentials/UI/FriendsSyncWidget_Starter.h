// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "FriendsSyncWidget_Starter.generated.h"

class UAccelByteWarsGameInstance;
class UFriendsSubsystem_Starter;
class UNativeFriendsSubsystem_Starter;
class UManagingFriendsSubsystem_Starter;
class UPromptSubsystem;
class UAccelByteWarsWidgetSwitcher;
class UListView;
class UCommonButtonBase;
class UTextBlock;

UCLASS(Abstract)
class ACCELBYTEWARS_API UFriendsSyncWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void FetchNativeFriendList();
	void SyncNativePlatformFriendList();

	UAccelByteWarsGameInstance* GameInstance;
	UFriendsSubsystem_Starter* FriendsSubsystem;
	UNativeFriendsSubsystem_Starter* NativeFriendsSubsystem;
	UManagingFriendsSubsystem_Starter* ManagingFriendsSubsystem;
	UPromptSubsystem* PromptSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Friends;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_Friends;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_ExecuteFriendSync;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_SyncResult;
};
