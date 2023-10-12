// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingDSWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "CommonButtonBase.h"

#include "TutorialModules/MatchmakingEssentials/UI/QuickPlayWidget.h"
#include "TutorialModules/MatchmakingEssentials/MatchmakingEssentialsModels.h"
#include "TutorialModules/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UMatchmakingDSWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	MatchmakingOnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(BaseOnlineSession);

	MatchmakingOnlineSession->GetOnStartMatchmakingCompleteDelegates()->AddUObject(this, &ThisClass::OnStartMatchmakingComplete);
	MatchmakingOnlineSession->GetOnCancelMatchmakingCompleteDelegates()->AddUObject(this, &ThisClass::OnCancelMatchmakingComplete);
	MatchmakingOnlineSession->GetOnMatchmakingCompleteDelegates()->AddUObject(this, &ThisClass::OnMatchmakingComplete);

	Btn_StartMatchmakingDS->OnClicked().AddUObject(this, &ThisClass::StartMatchmaking);

	W_Parent = GetFirstOccurenceOuter<UQuickPlayWidget>();
	if (!ensure(W_Parent))
	{
		return;
	}

	W_Parent->GetProcessingWidget()->OnCancelClicked.AddUObject(this, &ThisClass::CancelMatchmaking);
	W_Parent->GetProcessingWidget()->OnRetryClicked.AddUObject(this, &ThisClass::StartMatchmaking);
}

void UMatchmakingDSWidget::NativeDestruct()
{
	Super::NativeDestruct();

	MatchmakingOnlineSession->GetOnStartMatchmakingCompleteDelegates()->RemoveAll(this);
	MatchmakingOnlineSession->GetOnCancelMatchmakingCompleteDelegates()->RemoveAll(this);
	MatchmakingOnlineSession->GetOnMatchmakingCompleteDelegates()->RemoveAll(this);

	Btn_StartMatchmakingDS->OnClicked().RemoveAll(this);
	W_Parent->GetProcessingWidget()->OnCancelClicked.RemoveAll(this);
	W_Parent->GetProcessingWidget()->OnRetryClicked.RemoveAll(this);
}

void UMatchmakingDSWidget::StartMatchmaking() const
{
	if (MatchmakingOnlineSession->ValidateToStartMatchmaking.IsBound() && 
		!MatchmakingOnlineSession->ValidateToStartMatchmaking.Execute(W_Parent->GetSelectedGameModeType()))
	{
		return;
	}

	// Otherwise, start matchmaking.
	W_Parent->SetLoadingMessage(TEXT_FINDING_MATCH, false);
	W_Parent->SwitchContent(UQuickPlayWidget::EContentType::LOADING);

	MatchmakingOnlineSession->StartMatchmaking(
		GetOwningPlayer(),
		MatchmakingOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		EGameModeNetworkType::DS,
		W_Parent->GetSelectedGameModeType());
}

void UMatchmakingDSWidget::CancelMatchmaking() const
{
	W_Parent->SetLoadingMessage(TEXT_CANCEL_MATCHMAKING, false);
	W_Parent->SwitchContent(UQuickPlayWidget::EContentType::LOADING);

	MatchmakingOnlineSession->CancelMatchmaking(
		GetOwningPlayer(),
		MatchmakingOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
}

void UMatchmakingDSWidget::OnStartMatchmakingComplete(FName SessionName, bool bSucceeded) const
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(MatchmakingOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
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

void UMatchmakingDSWidget::OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded) const
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(MatchmakingOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
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

void UMatchmakingDSWidget::OnMatchmakingComplete(FName SessionName, bool bSucceeded) const
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(MatchmakingOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
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

#pragma region "UI Related"
UWidget* UMatchmakingDSWidget::NativeGetDesiredFocusTarget() const
{
	return Btn_StartMatchmakingDS;
}
#pragma endregion 
