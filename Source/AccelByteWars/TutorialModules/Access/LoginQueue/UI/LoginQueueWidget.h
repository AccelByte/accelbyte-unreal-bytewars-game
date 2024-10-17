// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Access/LoginQueue/LoginQueueModel.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "LoginQueueWidget.generated.h"

class ULoginQueueSubsystem;
class ULoginWidget;
class UCommonButtonBase;
class UTextBlock;
class UWidgetSwitcher;

UCLASS(Abstract)
class ACCELBYTEWARS_API ULoginQueueWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

// @@@SNIPSTART LoginQueueWidget.h-protected
// @@@MULTISNIP LoginQueueSubsystem {"selectedLines": ["1", "11-12"]}
// @@@MULTISNIP CancelQueueDeclaration {"selectedLines": ["1-3"]}
// @@@MULTISNIP LoginQueueDeclaration {"selectedLines": ["1", "5-9"]}
protected:
	void CancelQueue() const;
	void OnCancelQueueCompleted(const APlayerController* PC, const FOnlineError& Error) const;

	void OnLoginQueued(const APlayerController* PC, const FAccelByteModelsLoginQueueTicketInfo& TicketInfo);
	void OnLoginStatusUpdated(
		const APlayerController* PC,
		const FAccelByteModelsLoginQueueTicketInfo& TicketInfo,
		const FOnlineError& Error) const;

	UPROPERTY()
	ULoginQueueSubsystem* LoginQueueSubsystem;
// @@@SNIPEND

// @@@SNIPSTART LoginQueueWidget.h-private
// @@@MULTISNIP W_Parent {"selectedLines": ["1-3"]}
// @@@MULTISNIP StateComponentUI {"selectedLines": ["1", "5-12"]}
// @@@MULTISNIP QueueStateUI {"selectedLines": ["1", "14-24"]}
private:
	UPROPERTY()
	ULoginWidget* W_Parent;

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
// @@@SNIPEND
};
