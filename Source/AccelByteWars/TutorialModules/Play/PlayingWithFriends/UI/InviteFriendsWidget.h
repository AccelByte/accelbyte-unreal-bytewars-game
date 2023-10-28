// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "InviteFriendsWidget.generated.h"

class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UInviteFriendsWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
	void OpenFriendsMenu() const;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Friends;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAccelByteWarsActivatableWidget> FriendsWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAccelByteWarsActivatableWidget> FriendsWidgetClass_Starter;
};
