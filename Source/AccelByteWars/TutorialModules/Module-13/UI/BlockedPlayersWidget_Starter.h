// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Module-13/ManagingFriendsSubsystem_Starter.h"
#include "BlockedPlayersWidget_Starter.generated.h"

class UAccelByteWarsWidgetList;

UCLASS(Abstract)
class ACCELBYTEWARS_API UBlockedPlayersWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void GetBlockedPlayerList();

	UManagingFriendsSubsystem_Starter* ManagingFriendsSubsystem;
	FDelegateHandle OnBlockedPlayerListUpdatedDelegateHandle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetList* WidgetList;
};