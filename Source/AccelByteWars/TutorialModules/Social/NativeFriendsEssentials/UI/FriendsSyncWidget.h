// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "FriendsSyncWidget.generated.h"

class UAccelByteWarsGameInstance;
class UFriendsSubsystem;
class UNativeFriendsSubsystem;
class UManagingFriendsSubsystem;
class UPromptSubsystem;
class UAccelByteWarsWidgetSwitcher;
class UFriendData;
class UNativeFriendData;
class UListView;
class UCommonButtonBase;
class UTextBlock;

UCLASS(Abstract)
class ACCELBYTEWARS_API UFriendsSyncWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void FetchNativeFriendList();
	void OnGetFriendListComplete(bool bWasSuccessful, TArray<UFriendData*> Friends, const FString& ErrorMessage);
	void OnGetBlockedPlayerListCompelete(bool bWasSuccessful, TArray<UFriendData*> BlockedPlayers, const FString& ErrorMessage);
	void OnGetNativeFriendListComplete(bool bWasSuccessful, TArray<UNativeFriendData*> Friends, const FString& ErrorMessage);
	void SyncNativePlatformFriendList();

	UAccelByteWarsGameInstance* GameInstance;
	UFriendsSubsystem* FriendsSubsystem;
	UNativeFriendsSubsystem* NativeFriendsSubsystem;
	UManagingFriendsSubsystem* ManagingFriendsSubsystem;
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
