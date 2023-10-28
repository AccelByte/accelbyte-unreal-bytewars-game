// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineError.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "CreateMatchSessionDSWidget.generated.h"

class UCommonButtonBase;
class UAccelByteWarsOnlineSessionBase;
class UCreateMatchSessionWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UCreateMatchSessionDSWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

protected:
	UFUNCTION()
	void CreateSession() const;

	UFUNCTION()
	void CancelJoiningSession() const;

private:
	void OnCreateSessionComplete(FName SessionName, bool bSucceeded) const;
	void OnCancelJoiningSessionComplete(FName SessionName, bool bSucceeded) const;
	void OnSessionServerUpdateReceived(
		const FName SessionName,
		const FOnlineError& Error,
		const bool bHasClientTravelTriggered) const;

	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;

#pragma region "UI related"
protected:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_StartMatchSessionDS;

	UPROPERTY()
	UCreateMatchSessionWidget* W_Parent;
#pragma endregion
};
