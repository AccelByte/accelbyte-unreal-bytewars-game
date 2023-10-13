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

void UMatchmakingDSWidget::NativeOnActivated()
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

	Btn_StartMatchmakingDS->OnClicked().AddUObject(this, &ThisClass::StartMatchmaking);

	W_Parent = GetFirstOccurenceOuter<UQuickPlayWidget>();
	if (!ensure(W_Parent))
	{
		return;
	}

	W_Parent->GetProcessingWidget()->OnCancelClicked.AddUObject(this, &ThisClass::CancelMatchmaking);
	W_Parent->GetProcessingWidget()->OnRetryClicked.AddUObject(this, &ThisClass::StartMatchmaking);
}

void UMatchmakingDSWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	OnlineSession->GetOnStartMatchmakingCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnCancelMatchmakingCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnMatchmakingCompleteDelegates()->RemoveAll(this);

	Btn_StartMatchmakingDS->OnClicked().RemoveAll(this);

	W_Parent->GetProcessingWidget()->OnCancelClicked.RemoveAll(this);
	W_Parent->GetProcessingWidget()->OnRetryClicked.RemoveAll(this);
}

void UMatchmakingDSWidget::StartMatchmaking() const
{
	if (OnlineSession->ValidateToStartMatchmaking.IsBound() && 
		!OnlineSession->ValidateToStartMatchmaking.Execute(W_Parent->GetSelectedGameModeType()))
	{
		return;
	}

	// Otherwise, start matchmaking.
	W_Parent->SetLoadingMessage(TEXT_FINDING_MATCH, false);
	W_Parent->SwitchContent(UQuickPlayWidget::EContentType::LOADING);

	OnlineSession->StartMatchmaking(
		GetOwningPlayer(),
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		EGameModeNetworkType::DS,
		W_Parent->GetSelectedGameModeType());
}

void UMatchmakingDSWidget::CancelMatchmaking() const
{
	W_Parent->SetLoadingMessage(TEXT_CANCEL_MATCHMAKING, false);
	W_Parent->SwitchContent(UQuickPlayWidget::EContentType::LOADING);

	OnlineSession->CancelMatchmaking(
		GetOwningPlayer(),
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
}

void UMatchmakingDSWidget::OnStartMatchmakingComplete(FName SessionName, bool bSucceeded) const
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

void UMatchmakingDSWidget::OnCancelMatchmakingComplete(FName SessionName, bool bSucceeded) const
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

void UMatchmakingDSWidget::OnMatchmakingComplete(FName SessionName, bool bSucceeded) const
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

#pragma region "UI Related"
UWidget* UMatchmakingDSWidget::NativeGetDesiredFocusTarget() const
{
	return Btn_StartMatchmakingDS;
}
#pragma endregion 
