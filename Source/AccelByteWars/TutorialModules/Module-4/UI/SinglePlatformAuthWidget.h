// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "SinglePlatformAuthWidget.generated.h"

class UCommonButtonBase;

UCLASS()
class ACCELBYTEWARS_API USinglePlatformAuthWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

protected:
	void NativeConstruct() override;

private:
	void OnLoginWithSinglePlatformAuthButtonClicked();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_LoginWithSinglePlatformAuth;
};