// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "PopUpWidget.generated.h"

class UTextBlock;
class UAccelByteWarsButtonBase;

UENUM(BlueprintType)
enum EPopUpResult
{
	Confirmed,
	Declined
};

UENUM()
enum EPopUpType
{
	ConfirmationYesNo,
	ConfirmationConfirmCancel,
	MessageOk
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FPopUpResultDynamicDelegate, EPopUpResult, Result);

DECLARE_MULTICAST_DELEGATE_OneParam(FPopUpResult, EPopUpResult /* Result */);
typedef FPopUpResult::FDelegate FPopUpResultDelegate;

UCLASS()
class ACCELBYTEWARS_API UPopUpWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void SetPopUpText(const FText& Header, const FText& Body);
	void SetPopUpType(const EPopUpType Type);
	void SetDynamicCallback(FPopUpResultDynamicDelegate ResultCallback);
	void SetCallback(FPopUpResultDelegate ResultCallback);

protected:
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;

private:
	void SubmitResult(EPopUpResult Result);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Header;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Body;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Confirm;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Decline;

	FPopUpResultDynamicDelegate DynamicCallback;
	FPopUpResultDelegate Callback;
};