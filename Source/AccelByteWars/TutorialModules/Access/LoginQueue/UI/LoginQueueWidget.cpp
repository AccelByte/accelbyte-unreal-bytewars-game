﻿// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "LoginQueueWidget.h"

#include "CommonButtonBase.h"
#include "Access/AuthEssentials/UI/LoginWidget.h"
#include "Access/LoginQueue/LoginQueueLog.h"
#include "Access/LoginQueue/LoginQueueSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

void ULoginQueueWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	LoginQueueSubsystem = GetGameInstance()->GetSubsystem<ULoginQueueSubsystem>();
	ensure(LoginQueueSubsystem);

	W_Parent = GetFirstOccurenceOuter<ULoginWidget>();
	if (!ensure(W_Parent))
	{
		UE_LOG_LOGIN_QUEUE(Warning, TEXT("Deactivate Auth Essentials's starter mode or activate this module's starter mode to make this widget work properly."))
		return;
	}

	Btn_CancelQueue->OnClicked().AddUObject(this, &ThisClass::CancelQueue);
	LoginQueueSubsystem->OnLoginQueueCancelCompletedDelegates.AddUObject(this, &ThisClass::OnCancelQueueCompleted);
	LoginQueueSubsystem->OnLoginQueuedDelegates.AddUObject(this, &ThisClass::OnLoginQueued);
	LoginQueueSubsystem->OnLoginTicketStatusUpdatedDelegates.AddUObject(this, &ThisClass::OnLoginStatusUpdated);
}

void ULoginQueueWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_CancelQueue->OnClicked().RemoveAll(this);
	LoginQueueSubsystem->OnLoginQueueCancelCompletedDelegates.RemoveAll(this);
	LoginQueueSubsystem->OnLoginQueuedDelegates.RemoveAll(this);
	LoginQueueSubsystem->OnLoginTicketStatusUpdatedDelegates.RemoveAll(this);
}

void ULoginQueueWidget::CancelQueue() const
{
	Ws_Root->SetActiveWidget(W_Loading);
	LoginQueueSubsystem->CancelLoginQueue(GetOwningPlayer());
}

void ULoginQueueWidget::OnCancelQueueCompleted(const APlayerController* PC, const FOnlineError& Error) const
{
	// safety
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

void ULoginQueueWidget::OnLoginQueued(
	const APlayerController* PC,
	const FAccelByteModelsLoginQueueTicketInfo& TicketInfo)
{
	// safety
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

void ULoginQueueWidget::OnLoginStatusUpdated(
	const APlayerController* PC,
	const FAccelByteModelsLoginQueueTicketInfo& TicketInfo,
	const FOnlineError& Error) const
{
	// safety
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