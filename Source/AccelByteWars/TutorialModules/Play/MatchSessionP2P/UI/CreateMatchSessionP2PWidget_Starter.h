// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineError.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "CreateMatchSessionP2PWidget_Starter.generated.h"

class UCreateMatchSessionWidget;
class UCommonButtonBase;
class UAccelByteWarsOnlineSessionBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UCreateMatchSessionP2PWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

#pragma region "Match Session with P2P function declarations"
protected:
	// TODO: Add your function declarations here
#pragma endregion

private:
	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

#pragma region "UI related"
protected:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_StartMatchSessionP2P;

	UPROPERTY()
	UCreateMatchSessionWidget* W_Parent;
#pragma endregion
};
