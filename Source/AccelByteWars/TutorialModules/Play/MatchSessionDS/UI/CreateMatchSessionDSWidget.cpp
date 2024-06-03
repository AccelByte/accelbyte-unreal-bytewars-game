// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "CreateMatchSessionDSWidget.h"

#include "CommonButtonBase.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Play/MatchSessionEssentials/MatchSessionEssentialsModels.h"
#include "Play/MatchSessionEssentials/UI/CreateMatchSessionWidget.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UCreateMatchSessionDSWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Get Online Session
	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(BaseOnlineSession);

	// Get parent menu widget
	W_Parent = GetFirstOccurenceOuter<UCreateMatchSessionWidget>();
	if (!ensure(W_Parent))
	{
		return;
	}

	OnlineSession->GetOnCreateSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnCreateSessionComplete);
	OnlineSession->GetOnLeaveSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnCancelJoiningSessionComplete);
	OnlineSession->GetOnSessionServerUpdateReceivedDelegates()->AddUObject(
		this, &ThisClass::OnSessionServerUpdateReceived);

	Btn_StartMatchSessionDS->OnClicked().AddUObject(this, &ThisClass::CreateSession);
}

void UCreateMatchSessionDSWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	OnlineSession->GetOnCreateSessionCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnLeaveSessionCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnSessionServerUpdateReceivedDelegates()->RemoveAll(this);

	Btn_StartMatchSessionDS->OnClicked().RemoveAll(this);
	W_Parent->GetProcessingWidgetComponent()->OnRetryClicked.RemoveAll(this);
	W_Parent->GetProcessingWidgetComponent()->OnCancelClicked.RemoveAll(this);
}

void UCreateMatchSessionDSWidget::CreateSession() const
{
	if (OnlineSession->ValidateToStartSession.IsBound() &&
		!OnlineSession->ValidateToStartSession.Execute())
	{
		return;
	}

	// Make sure the retry and cancel game session is performed by this class when the DS network type is selected.
	W_Parent->GetProcessingWidgetComponent()->OnCancelClicked.Clear();
	W_Parent->GetProcessingWidgetComponent()->OnRetryClicked.Clear();
	W_Parent->GetProcessingWidgetComponent()->OnCancelClicked.AddUObject(this, &ThisClass::CancelJoiningSession);
	W_Parent->GetProcessingWidgetComponent()->OnRetryClicked.AddUObject(this, &ThisClass::CreateSession);

	W_Parent->SetLoadingMessage(TEXT_REQUESTING_SESSION_CREATION, false);
	W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::LOADING);

	OnlineSession->CreateMatchSession(
		OnlineSession->GetLocalUserNumFromPlayerController(GetOwningPlayer()),
		EGameModeNetworkType::DS,
		W_Parent->GetSelectedGameModeType());
}

void UCreateMatchSessionDSWidget::OnCreateSessionComplete(FName SessionName, bool bSucceeded) const
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession))) 
	{
		return;
	}

	if (bSucceeded)
	{
		W_Parent->SetLoadingMessage(TEXT_JOINING_SESSION, true);
		W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::LOADING);
	}
	else
	{
		W_Parent->SetErrorMessage(TEXT_FAILED_TO_CREATE_SESSION, true);
		W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::ERROR);
	}
}

void UCreateMatchSessionDSWidget::CancelJoiningSession() const
{
	W_Parent->SetLoadingMessage(TEXT_LEAVING_SESSION, false);
	W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::LOADING);

	OnlineSession->LeaveSession(
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
}

void UCreateMatchSessionDSWidget::OnCancelJoiningSessionComplete(FName SessionName, bool bSucceeded) const
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	if (bSucceeded)
	{
		W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::SELECT_GAMEMODE);
	}
	else
	{
		W_Parent->SetErrorMessage(TEXT_FAILED_TO_LEAVE_SESSION, false);
		W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::ERROR);
	}
}

void UCreateMatchSessionDSWidget::OnSessionServerUpdateReceived(
	const FName SessionName,
	const FOnlineError& Error,
	const bool bHasClientTravelTriggered) const
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	if (Error.bSucceeded && !bHasClientTravelTriggered)
	{
		// waiting for further update
		W_Parent->SetLoadingMessage(TEXT_JOINING_SESSION, true);
		W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::LOADING);
	}
	else if (!bHasClientTravelTriggered && !Error.bSucceeded)
	{
		W_Parent->SetErrorMessage(TEXT_FAILED_TO_JOIN_SESSION, false);
		W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::ERROR);
	}
}
