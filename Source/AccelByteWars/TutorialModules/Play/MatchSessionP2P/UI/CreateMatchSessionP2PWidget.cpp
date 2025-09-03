// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "CreateMatchSessionP2PWidget.h"

#include "CommonButtonBase.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Play/MatchSessionEssentials/MatchSessionEssentialsModels.h"
#include "Play/MatchSessionEssentials/UI/CreateMatchSessionWidget.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

// @@@SNIPSTART CreateMatchSessionP2PWidget.cpp-NativeOnActivated
// @@@MULTISNIP HelperDefinition {"selectedLines": ["1-2", "5-19", "26"]}
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "25-26"]}
void UCreateMatchSessionP2PWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Get online session
	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(BaseOnlineSession);
	ensure(OnlineSession);

	// Get parent menu widget
	W_Parent = GetFirstOccurenceOuter<UCreateMatchSessionWidget>();
	if (!ensure(W_Parent))
	{
		return;
	}

	OnlineSession->GetOnCreateSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnCreateSessionComplete);
	OnlineSession->GetOnLeaveSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnCancelJoiningSessionComplete);
	OnlineSession->GetOnSessionServerUpdateReceivedDelegates()->AddUObject(this, &ThisClass::OnSessionServerUpdateReceived);

	Btn_StartMatchSessionP2P->OnClicked().AddUObject(this, &ThisClass::CreateSession);
}
// @@@SNIPEND

// @@@SNIPSTART CreateMatchSessionP2PWidget.cpp-NativeOnDeactivated
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-2", "9-12"]}
void UCreateMatchSessionP2PWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	OnlineSession->GetOnCreateSessionCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnLeaveSessionCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnSessionServerUpdateReceivedDelegates()->RemoveAll(this);

	Btn_StartMatchSessionP2P->OnClicked().RemoveAll(this);
	W_Parent->GetProcessingWidgetComponent()->OnRetryClicked.RemoveAll(this);
	W_Parent->GetProcessingWidgetComponent()->OnCancelClicked.RemoveAll(this);
}
// @@@SNIPEND

// @@@SNIPSTART CreateMatchSessionP2PWidget.cpp-CreateSession
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-7", "15-16", "22"]}
void UCreateMatchSessionP2PWidget::CreateSession() const
{
	if (OnlineSession->ValidateToStartSession.IsBound() &&
		!OnlineSession->ValidateToStartSession.Execute())
	{
		return;
	}

	// Make sure the retry and cancel game session is performed by this class when the P2P network type is selected.
	W_Parent->GetProcessingWidgetComponent()->OnCancelClicked.Clear();
	W_Parent->GetProcessingWidgetComponent()->OnRetryClicked.Clear();
	W_Parent->GetProcessingWidgetComponent()->OnCancelClicked.AddUObject(this, &ThisClass::CancelJoiningSession);
	W_Parent->GetProcessingWidgetComponent()->OnRetryClicked.AddUObject(this, &ThisClass::CreateSession);

	W_Parent->SetLoadingMessage(TEXT_REQUESTING_SESSION_CREATION, false);
	W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::LOADING);

	OnlineSession->CreateMatchSession(
		OnlineSession->GetLocalUserNumFromPlayerController(GetOwningPlayer()),
		EGameModeNetworkType::P2P,
		W_Parent->GetSelectedGameModeType(), W_Parent->GetSelectedGameStyle());
}
// @@@SNIPEND

// @@@SNIPSTART CreateMatchSessionP2PWidget.cpp-OnCreateSessionComplete
void UCreateMatchSessionP2PWidget::OnCreateSessionComplete(FName SessionName, bool bSucceeded) const
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
// @@@SNIPEND

// @@@SNIPSTART CreateMatchSessionP2PWidget.cpp-CancelJoiningSession
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-4", "8"]}
void UCreateMatchSessionP2PWidget::CancelJoiningSession() const
{
	W_Parent->SetLoadingMessage(TEXT_LEAVING_SESSION, false);
	W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::LOADING);

	OnlineSession->LeaveSession(
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
}
// @@@SNIPEND

// @@@SNIPSTART CreateMatchSessionP2PWidget.cpp-OnCancelJoiningSessionComplete
void UCreateMatchSessionP2PWidget::OnCancelJoiningSessionComplete(FName SessionName, bool bSucceeded) const
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
// @@@SNIPEND

// @@@SNIPSTART CreateMatchSessionP2PWidget.cpp-OnSessionServerUpdateReceived
void UCreateMatchSessionP2PWidget::OnSessionServerUpdateReceived(
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
		// Keep showing the loading state until the client travels to the P2P host.
		W_Parent->SetLoadingMessage(TEXT_JOINING_SESSION, true);
		W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::LOADING);
	}
	else if (!bHasClientTravelTriggered && !Error.bSucceeded)
	{
		W_Parent->SetErrorMessage(TEXT_FAILED_TO_JOIN_SESSION, false);
		W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::ERROR);
	}
}
// @@@SNIPEND
