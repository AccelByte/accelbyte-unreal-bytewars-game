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

UCLASS(Abstract)
class ACCELBYTEWARS_API UCreateMatchSessionP2PWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

// @@@SNIPSTART CreateMatchSessionP2PWidget.h-protected
// @@@MULTISNIP CreateSessionDeclaration {"selectedLines": ["1-5"]}
// @@@MULTISNIP CancelJoinSessionDeclaration {"selectedLines": ["1", "7-10"]}
// @@@MULTISNIP OnSessionServerUpdateReceived {"selectedLines": ["1", "12-15"]}
// @@@MULTISNIP OnlineSession {"selectedLines": ["1", "17-18"]}
protected:
	UFUNCTION()
	void CreateSession() const;

	void OnCreateSessionComplete(FName SessionName, bool bSucceeded) const;

	UFUNCTION()
	void CancelJoiningSession() const;

	void OnCancelJoiningSessionComplete(FName SessionName, bool bSucceeded) const;

	void OnSessionServerUpdateReceived(
		const FName SessionName,
		const FOnlineError& Error,
		const bool bHasClientTravelTriggered) const;

	UPROPERTY()
	UAccelByteWarsOnlineSessionBase* OnlineSession;
// @@@SNIPEND

#pragma region "UI related"
// @@@SNIPSTART CreateMatchSessionP2PWidget.h-private
// @@@MULTISNIP CreateMatchSessionUI {"selectedLines": ["1-6"]}
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_StartMatchSessionP2P;

	UPROPERTY()
	UCreateMatchSessionWidget* W_Parent;
// @@@SNIPEND
#pragma endregion
};
