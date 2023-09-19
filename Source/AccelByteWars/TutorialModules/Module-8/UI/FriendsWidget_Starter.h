// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Module-8/FriendsSubsystem_Starter.h"
#include "FriendsWidget_Starter.generated.h"

class UAccelByteWarsGameInstance;
class UAccelByteWarsWidgetSwitcher;
class UTileView;

UCLASS(Abstract)
class ACCELBYTEWARS_API UFriendsWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void GetFriendList();
	void OnFriendEntryClicked(UObject* Item);

	UAccelByteWarsGameInstance* GameInstance;
	UFriendsSubsystem_Starter* FriendsSubsystem;
	FDelegateHandle OnFriendListUpdatedDelegateHandle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Friends;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTileView* Tv_Friends;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> FriendDetailsWidgetClass;
};