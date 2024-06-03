// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingP2PWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "CommonButtonBase.h"

#include "Play/MatchmakingEssentials/MatchmakingEssentialsModels.h"
#include "Play/MatchmakingEssentials/UI/QuickPlayWidget.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UMatchmakingP2PWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}

	OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(BaseOnlineSession);
	ensure(OnlineSession);

	OnlineSession->GetOnStartMatchmakingCompleteDelegates()->AddUObject(this, &ThisClass::OnStartMatchmakingComplete);
	OnlineSession->GetOnCancelMatchmakingCompleteDelegates()->AddUObject(this, &ThisClass::OnCancelMatchmakingComplete);
	OnlineSession->GetOnMatchmakingCompleteDelegates()->AddUObject(this, &ThisClass::OnMatchmakingComplete);
	OnlineSession->GetOnJoinSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnJoinSessionComplete);
	OnlineSession->GetOnLeaveSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnCancelJoinSessionComplete);

	Btn_StartMatchmakingP2P->OnClicked().AddUObject(this, &ThisClass::StartMatchmaking);

	W_Parent = GetFirstOccurenceOuter<UQuickPlayWidget>();
	if (!ensure(W_Parent))
	{
		return;
	}
}

void UMatchmakingP2PWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	OnlineSession->GetOnStartMatchmakingCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnCancelMatchmakingCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnMatchmakingCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnJoinSessionCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnLeaveSessionCompleteDelegates()->RemoveAll(this);

	Btn_StartMatchmakingP2P->OnClicked().RemoveAll(this);

	W_Parent->GetProcessingWidget()->OnCancelClicked.RemoveAll(this);
	W_Parent->GetProcessingWidget()->OnRetryClicked.RemoveAll(this);
}

void UMatchmakingP2PWidget::StartMatchmaking() const
{
	if (OnlineSession->ValidateToStartMatchmaking.IsBound() &&
		!OnlineSession->ValidateToStartMatchmaking.Execute(W_Parent->GetSelectedGameModeType()))
	{
		return;
	}

	W_Parent->GetProcessingWidget()->OnCancelClicked.Clear();
	W_Parent->GetProcessingWidget()->OnRetryClicked.Clear();
	W_Parent->GetProcessingWidget()->OnCancelClicked.AddUObject(this, &ThisClass::CancelButtonClicked);
	W_Parent->GetProcessingWidget()->OnRetryClicked.AddUObject(this, &ThisClass::StartMatchmaking);

	// Otherwise, start matchmaking.
	W_Parent->SetLoadingMessage(TEXT_FINDING_MATCH, false);
	W_Parent->SwitchContent(UQuickPlayWidget::EContentType::LOADING);

	OnlineSession->StartMatchmaking(
		GetOwningPlayer(),
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		EGameModeNetworkType::P2P,
		W_Parent->GetSelectedGameModeType());
}

void UMatchmakingP2PWidget::CancelButtonClicked() const
{
	bool bIsJoiningMatch = W_Parent->GetProcessingWidget()->LoadingMessage.EqualTo(TEXT_JOINING_MATCH);
	
	W_Parent->SetLoadingMessage(TEXT_CANCEL_MATCHMAKING, false);
	W_Parent->SwitchContent(UQuickPlayWidget::EContentType::LOADING);
	
	if(bIsJoiningMatch)
	{
		OnlineSession->LeaveSession(
			OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	}
	else
	{
		OnlineSession->CancelMatchmaking(
			GetOwningPlayer(),
			OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	}
}

void UMatchmakingP2PWidget::OnStartMatchmakingComplete(FName SessionName, bool bSucceeded) const
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	if (bSucceeded)
	{
		W_Parent->SetLoadingMessage(TEXT_FINDING_MATCH, true);
		W_Parent->SwitchContent(UQuickPlayWidget::EContentType::LOADING);
	}
	else
	{
		W_Parent->SetErrorMessage(TEXT_FAILED_FIND_MATCH, true);
		W_Parent->SwitchContent(UQuickPlayWidget::EContentType::ERROR);
	}
}

void UMatchmakingP2PWidget::OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded) const
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	if (bSucceeded)
	{
		W_Parent->SwitchContent(UQuickPlayWidget::EContentType::SELECTSERVERTYPE);
	}
	else
	{
		W_Parent->SetErrorMessage(TEXT_FAILED_CANCEL_MATCH, false);
		W_Parent->SwitchContent(UQuickPlayWidget::EContentType::ERROR);
	}
}

void UMatchmakingP2PWidget::OnMatchmakingComplete(FName SessionName, bool bSucceeded) const
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	if (bSucceeded)
	{
		W_Parent->SetLoadingMessage(TEXT_JOINING_MATCH, false);
		W_Parent->SwitchContent(UQuickPlayWidget::EContentType::LOADING);
	}
	else
	{
		W_Parent->SetErrorMessage(TEXT_FAILED_FIND_MATCH, true);
		W_Parent->SwitchContent(UQuickPlayWidget::EContentType::ERROR);
	}
}

void UMatchmakingP2PWidget::OnCancelJoinSessionComplete(FName SessionName, bool bSucceeded) const
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	if (bSucceeded)
	{
		W_Parent->SwitchContent(UQuickPlayWidget::EContentType::SELECTGAMEMODE);
	}
	else
	{
		W_Parent->SetErrorMessage(TEXT_FAILED_CANCEL_MATCH, false);
		W_Parent->SwitchContent(UQuickPlayWidget::EContentType::ERROR);
	}
}

void UMatchmakingP2PWidget::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) const
{
	if(Result == EOnJoinSessionCompleteResult::Success)
	{
		W_Parent->SetLoadingMessage(TEXT_JOINING_MATCH, true);
		W_Parent->SwitchContent(UQuickPlayWidget::EContentType::LOADING);
	}
	else
	{
		W_Parent->SetErrorMessage(TEXT_FAILED_JOIN_MATCH, true);
		W_Parent->SwitchContent(UQuickPlayWidget::EContentType::ERROR);
	}
}