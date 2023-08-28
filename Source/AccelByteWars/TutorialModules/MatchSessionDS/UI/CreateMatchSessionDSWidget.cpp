// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "CreateMatchSessionDSWidget.h"

#include "CommonButtonBase.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "TutorialModules/MatchSessionEssentials/MatchSessionEssentialsModels.h"
#include "TutorialModules/MatchSessionEssentials/UI/CreateMatchSessionWidget.h"
#include "TutorialModules/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UCreateMatchSessionDSWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_StartMatchSessionDS->OnClicked().AddUObject(this, &ThisClass::CreateSession);

	W_Parent = GetFirstOccurenceOuter<UCreateMatchSessionWidget>();
	if (!ensure(W_Parent))
	{
		return;
	}

	W_Parent->GetProcessingWidgetComponent()->OnCancelClicked.AddUObject(this, &ThisClass::CancelJoiningSession);
	W_Parent->GetProcessingWidgetComponent()->OnRetryClicked.AddUObject(this, &ThisClass::CreateSession);

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(BaseOnlineSession);

	OnlineSession->GetOnCreateSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnCreateSessionComplete);
	OnlineSession->GetOnLeaveSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnCancelJoiningSessionComplete);
	OnlineSession->GetOnSessionServerUpdateReceivedDelegates()->AddUObject(
		this, &ThisClass::OnSessionServerUpdateReceived);
}

void UCreateMatchSessionDSWidget::NativeDestruct()
{
	Super::NativeDestruct();

	Btn_StartMatchSessionDS->OnClicked().RemoveAll(this);

	W_Parent->GetProcessingWidgetComponent()->OnRetryClicked.RemoveAll(this);
	W_Parent->GetProcessingWidgetComponent()->OnCancelClicked.RemoveAll(this);

	OnlineSession->GetOnCreateSessionCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnLeaveSessionCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnSessionServerUpdateReceivedDelegates()->RemoveAll(this);
}

void UCreateMatchSessionDSWidget::CreateSession() const
{
	W_Parent->SetLoadingMessage(TEXT_REQUESTING_SESSION_CREATION, false);
	W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::LOADING);

	OnlineSession->CreateMatchSession(
		OnlineSession->GetLocalUserNumFromPlayerController(GetOwningPlayer()),
		EGameModeNetworkType::DS,
		W_Parent->GetSelectedGameModeType());
}

void UCreateMatchSessionDSWidget::CancelJoiningSession() const
{
	W_Parent->SetLoadingMessage(TEXT_LEAVING_SESSION, false);
	W_Parent->SwitchContent(UCreateMatchSessionWidget::EContentType::LOADING);

	OnlineSession->LeaveSession(
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
}

void UCreateMatchSessionDSWidget::OnCreateSessionComplete(FName SessionName, bool bSucceeded) const
{
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

void UCreateMatchSessionDSWidget::OnCancelJoiningSessionComplete(FName SessionName, bool bSucceeded) const
{
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

#pragma region "UI Related"
UWidget* UCreateMatchSessionDSWidget::NativeGetDesiredFocusTarget() const
{
	return Btn_StartMatchSessionDS;
}
#pragma endregion 
