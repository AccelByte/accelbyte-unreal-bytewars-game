// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Access/LoginQueue/LoginQueueModel.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "LoginQueueWidget_Starter.generated.h"

class ULoginQueueSubsystem_Starter;
class ULoginWidget_Starter;
class UCommonButtonBase;
class UTextBlock;
class UWidgetSwitcher;

UCLASS(Abstract)
class ACCELBYTEWARS_API ULoginQueueWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

protected:
#pragma region "Tutorial"
	// place your code here
#pragma endregion

	UPROPERTY()
	ULoginQueueSubsystem_Starter* LoginQueueSubsystem;

#pragma region "UI"
private:
	UPROPERTY()
	ULoginWidget_Starter* W_Parent;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_InQueue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_Loading;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_CancelQueue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_PositionInQueue;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_EstimatedWaitingTime;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_UpdatedAt;
#pragma endregion 
};
