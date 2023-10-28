// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineError.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "CreateMatchSessionP2PWidget.generated.h"

class UCreateMatchSessionWidget;
class UCommonButtonBase;
class UAccelByteWarsOnlineSessionBase;

UCLASS()
class ACCELBYTEWARS_API UCreateMatchSessionP2PWidget : public UAccelByteWarsActivatableWidget
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
	UCommonButtonBase* Btn_StartMatchSessionP2P;

	UPROPERTY()
	UCreateMatchSessionWidget* W_Parent;
#pragma endregion
};
