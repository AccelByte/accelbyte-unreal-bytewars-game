// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Social/FriendsEssentials/FriendsSubsystem.h"
#include "FindFriendsWidget.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UWidgetSwitcher;
class UListView;
class UTextBlock;
class UEditableText;
class UThrobber;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UFindFriendsWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	UFUNCTION()
	void OnSearchBarCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	void CopyFriendCodeToClipboard();
	void DisplaySelfFriendCode();

	void FindFriendByDisplayName(const FString& DisplayName);
	void SendFriendRequestByFriendCode(const FString& FriendCode);

	UFriendsSubsystem* FriendsSubsystem;
	FDelegateHandle OnFriendListUpdatedDelegateHandle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_FriendCode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_FriendCode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_FriendCodeEmpty;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UThrobber* Th_FriendCodeLoader;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_CopyFriendCode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UEditableText* Edt_SearchBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_FindFriends;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_FindFriends;
};