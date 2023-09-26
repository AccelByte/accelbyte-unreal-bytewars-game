// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingP2PWidget.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "CommonButtonBase.h"

#include "TutorialModules/MatchmakingEssentials/MatchmakingEssentialsModels.h"
#include "TutorialModules/MatchmakingEssentials/UI/QuickPlayWidget.h"
#include "TutorialModules/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UMatchmakingP2PWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(BaseOnlineSession);

	OnlineSession->GetOnStartMatchmakingCompleteDelegates()->AddUObject(this, &ThisClass::OnStartMatchmakingComplete);
	OnlineSession->GetOnCancelMatchmakingCompleteDelegates()->AddUObject(this, &ThisClass::OnCancelMatchmakingComplete);
	OnlineSession->GetOnMatchmakingCompleteDelegates()->AddUObject(this, &ThisClass::OnMatchmakingComplete);

	Btn_StartMatchmakingP2P->OnClicked().AddUObject(this, &ThisClass::StartMatchmaking);

	W_Parent = GetFirstOccurenceOuter<UQuickPlayWidget>();
	if (!ensure(W_Parent))
	{
		return;
	}

	W_Parent->GetProcessingWidget()->OnCancelClicked.AddUObject(this, &ThisClass::CancelMatchmaking);
	W_Parent->GetProcessingWidget()->OnRetryClicked.AddUObject(this, &ThisClass::StartMatchmaking);
}

void UMatchmakingP2PWidget::NativeDestruct()
{
	Super::NativeDestruct();

	OnlineSession->GetOnStartMatchmakingCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnCancelMatchmakingCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnMatchmakingCompleteDelegates()->RemoveAll(this);

	Btn_StartMatchmakingP2P->OnClicked().RemoveAll(this);
	W_Parent->GetProcessingWidget()->OnCancelClicked.RemoveAll(this);
	W_Parent->GetProcessingWidget()->OnRetryClicked.RemoveAll(this);
}

void UMatchmakingP2PWidget::StartMatchmaking() const
{
	/* Cannot start matchmaking in Elimination game mode if the player is in party.
     * Since Elimination game mode is matchmaking between individual players, not teams. */
	if (const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()))
	{
		if (OnlineSession)
		{
			const bool bIsInParty = OnlineSession->GetPartyMembers().Num() > 1;
			UPromptSubsystem* PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();

			if (bIsInParty &&
				W_Parent->GetSelectedGameModeType() == EGameModeType::FFA &&
				PromptSubsystem)
			{
				// TODO: Make it localizable.
				PromptSubsystem->PushNotification(
					FText::FromString("Cannot matchmake in Elimination mode when in a party"),
					FString(""));
				return;
			}
		}
	}

	// Otherwise, start matchmaking.
	W_Parent->SetLoadingMessage(TEXT_FINDING_MATCH, false);
	W_Parent->SwitchContent(UQuickPlayWidget::EContentType::LOADING);

	OnlineSession->StartMatchmaking(
		GetOwningPlayer(),
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		EGameModeNetworkType::P2P,
		W_Parent->GetSelectedGameModeType());
}

void UMatchmakingP2PWidget::CancelMatchmaking() const
{
	W_Parent->SetLoadingMessage(TEXT_CANCEL_MATCHMAKING, false);
	W_Parent->SwitchContent(UQuickPlayWidget::EContentType::LOADING);

	OnlineSession->CancelMatchmaking(
		GetOwningPlayer(),
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
}

void UMatchmakingP2PWidget::OnStartMatchmakingComplete(FName SessionName, bool bSucceeded) const
{
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
UWidget* UMatchmakingP2PWidget::NativeGetDesiredFocusTarget() const
{
	return Btn_StartMatchmakingP2P;
}
#pragma endregion 
