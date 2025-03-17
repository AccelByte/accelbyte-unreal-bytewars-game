// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "LoginQueueWidget.h"

#include "CommonButtonBase.h"
#include "Access/AuthEssentials/UI/LoginWidget.h"
#include "Access/LoginQueue/LoginQueueLog.h"
#include "Access/LoginQueue/LoginQueueSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

// @@@SNIPSTART LoginQueueWidget.cpp-NativeOnActivated
// @@@MULTISNIP W_Parent {"selectedLines": ["1-2", "8-13", "19"]}
// @@@MULTISNIP LoginQueueSubsystem {"selectedLines": ["1-2", "5", "19"]}
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "9-15", "19"]}
// @@@MULTISNIP PutItAllTogether {"highlightedLines": "{16-18}"}
void ULoginQueueWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	LoginQueueSubsystem = GetGameInstance()->GetSubsystem<ULoginQueueSubsystem>();
	ensure(LoginQueueSubsystem);

	W_Parent = GetFirstOccurenceOuter<ULoginWidget>();
	if (!W_Parent)
	{
		UE_LOG_LOGIN_QUEUE(Warning, TEXT("Deactivate Auth Essentials's starter mode or activate this module's starter mode to make this widget work properly."))
		return;
	}

	Btn_CancelQueue->OnClicked().AddUObject(this, &ThisClass::CancelQueue);
	LoginQueueSubsystem->OnLoginQueueCancelCompletedDelegates.AddUObject(this, &ThisClass::OnCancelQueueCompleted);
	LoginQueueSubsystem->OnLoginQueuedDelegates.AddUObject(this, &ThisClass::OnLoginQueued);
	LoginQueueSubsystem->OnLoginTicketStatusUpdatedDelegates.AddUObject(this, &ThisClass::OnLoginStatusUpdated);
}
// @@@SNIPEND

// @@@SNIPSTART LoginQueueWidget.cpp-NativeOnDeactivated
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-5", "9"]}
// @@@MULTISNIP PutItAllTogether {"highlightedLines": "{6-8}"}
void ULoginQueueWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_CancelQueue->OnClicked().RemoveAll(this);
	LoginQueueSubsystem->OnLoginQueueCancelCompletedDelegates.RemoveAll(this);
	LoginQueueSubsystem->OnLoginQueuedDelegates.RemoveAll(this);
	LoginQueueSubsystem->OnLoginTicketStatusUpdatedDelegates.RemoveAll(this);
}
// @@@SNIPEND

// @@@SNIPSTART LoginQueueWidget.cpp-CancelQueue
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-3", "5"]}
void ULoginQueueWidget::CancelQueue() const
{
	Ws_Root->SetActiveWidget(W_Loading);
	LoginQueueSubsystem->CancelLoginQueue(GetOwningPlayer());
}
// @@@SNIPEND

// @@@SNIPSTART LoginQueueWidget.cpp-OnCancelQueueCompleted
void ULoginQueueWidget::OnCancelQueueCompleted(const APlayerController* PC, const FOnlineError& Error) const
{
	// Abort if Player Controller is not the current player.
	if (PC != GetOwningPlayer())
	{
		return;
	}

	if (Error.bSucceeded)
	{
		W_Parent->SetLoginState(ELoginState::Default);
	}
	else
	{
		W_Parent->OnLoginComplete(false, Error.ErrorRaw);
	}
}
// @@@SNIPEND

// @@@SNIPSTART LoginQueueWidget.cpp-OnLoginQueued
void ULoginQueueWidget::OnLoginQueued(
	const APlayerController* PC,
	const FAccelByteModelsLoginQueueTicketInfo& TicketInfo)
{
	// Abort if Player Controller is not the current player.
	if (PC != GetOwningPlayer())
	{
		return;
	}

	W_Parent->Ws_LoginState->SetActiveWidget(this);
	Btn_CancelQueue->SetUserFocus(GetOwningPlayer());

	Tb_EstimatedWaitingTime->SetText(FText::AsNumber(TicketInfo.EstimatedWaitingTimeInSeconds));
	Tb_PositionInQueue->SetText(FText::AsNumber(TicketInfo.Position));
	Tb_UpdatedAt->SetText(FText::FromString(FDateTime::Now().ToString(TEXT("%H:%M:%S"))));
	Ws_Root->SetActiveWidget(W_InQueue);
}
// @@@SNIPEND

// @@@SNIPSTART LoginQueueWidget.cpp-OnLoginStatusUpdated
void ULoginQueueWidget::OnLoginStatusUpdated(
	const APlayerController* PC,
	const FAccelByteModelsLoginQueueTicketInfo& TicketInfo,
	const FOnlineError& Error) const
{
	// Abort if Player Controller is not the current player.
	if (PC != GetOwningPlayer())
	{
		return;
	}

	if (!Error.bSucceeded)
	{
		W_Parent->OnLoginComplete(false, Error.ErrorMessage.ToString());
	}

	Tb_EstimatedWaitingTime->SetText(FText::AsNumber(TicketInfo.EstimatedWaitingTimeInSeconds));
	Tb_PositionInQueue->SetText(FText::AsNumber(TicketInfo.Position));
	Tb_UpdatedAt->SetText(FText::FromString(FDateTime::Now().ToString(TEXT("%H:%M:%S"))));
}
// @@@SNIPEND