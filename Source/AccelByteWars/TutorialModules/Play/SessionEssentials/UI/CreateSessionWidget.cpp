// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "CreateSessionWidget.h"

#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "Play/SessionEssentials/SessionEssentialsModel.h"

// @@@SNIPSTART CreateSessionWidget.cpp-NativeOnActivated
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-10", "31"]}
void UCreateSessionWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

#pragma region "UI related"
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_CreateSession->OnClicked().AddUObject(this, &ThisClass::CreateSession);
	Btn_Leave->OnClicked().AddUObject(this, &ThisClass::LeaveSession);
	Ws_Processing->OnRetryClicked.AddUObject(this, &ThisClass::CreateSession);
#pragma endregion 

	SessionOnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
	if (!ensure(SessionOnlineSession))
	{
		return;
	}

	SessionOnlineSession->GetOnCreateSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnCreateSessionComplete);
	SessionOnlineSession->GetOnLeaveSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnLeaveSessionComplete);

	// Set initial UI state.
	SwitchContent(EContentType::CREATE);

	// Set UI state to success if already in a session.
	const FName SessionName = SessionOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession);
	const FNamedOnlineSession* OnlineSession = SessionOnlineSession->GetSession(SessionName);
	if (OnlineSession) 
	{
		OnCreateSessionComplete(SessionName, true);
	}
}
// @@@SNIPEND

// @@@SNIPSTART CreateSessionWidget.cpp-NativeOnDeactivated
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-10", "17"]}
void UCreateSessionWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

#pragma region "UI related"
	Btn_CreateSession->OnClicked().RemoveAll(this);
	Btn_Back->OnClicked().RemoveAll(this);
	Btn_Leave->OnClicked().RemoveAll(this);
	Ws_Processing->OnRetryClicked.RemoveAll(this);
#pragma endregion 

	if (SessionOnlineSession)
	{
		SessionOnlineSession->GetOnCreateSessionCompleteDelegates()->RemoveAll(this);
		SessionOnlineSession->GetOnLeaveSessionCompleteDelegates()->RemoveAll(this);
	}
}
// @@@SNIPEND

// @@@SNIPSTART CreateSessionWidget.cpp-CreateSession
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-16", "26"]}
void UCreateSessionWidget::CreateSession()
{
	if (!SessionOnlineSession)
	{
		return;
	}

	// An event to validate to start the session will be used in the Play with Party module.
	if (SessionOnlineSession->ValidateToStartSession.IsBound() &&
		!SessionOnlineSession->ValidateToStartSession.Execute())
	{
		return;
	}

	Ws_Processing->LoadingMessage = TEXT_REQUESTING_SESSION_CREATION;
	SwitchContent(EContentType::LOADING);

	// Create dummy session. Override dummy session template name if applicable.
	SessionOnlineSession->CreateSession(
		SessionOnlineSession->GetLocalUserNumFromPlayerController(GetOwningPlayer()),
		SessionOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		FOnlineSessionSettings(),
		EAccelByteV2SessionType::GameSession,
		UTutorialModuleOnlineUtility::GetDummySessionTemplateOverride().IsEmpty() ? 
			SessionTemplateName_Dummy : UTutorialModuleOnlineUtility::GetDummySessionTemplateOverride());
}
// @@@SNIPEND

// @@@SNIPSTART CreateSessionWidget.cpp-OnCreateSessionComplete
void UCreateSessionWidget::OnCreateSessionComplete(FName SessionName, bool bSucceeded)
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(SessionOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	if (bSucceeded)
	{
		// Get session ID
		const FNamedOnlineSession* OnlineSession = SessionOnlineSession->GetSession(SessionName);
		Tb_SessionId->SetText(FText::FromString(OnlineSession->GetSessionIdStr()));

		SwitchContent(EContentType::SUCCESS);
	}
	else
	{
		Ws_Processing->ErrorMessage = TEXT_FAILED_TO_CREATE_SESSION;
		Ws_Processing->bShowRetryButtonOnError = true;
		SwitchContent(EContentType::ERROR);
	}
}
// @@@SNIPEND

// @@@SNIPSTART CreateSessionWidget.cpp-LeaveSession
// @@@MULTISNIP ReadyUI {"selectedLines": ["1-9", "13"]}
void UCreateSessionWidget::LeaveSession()
{
	if (!SessionOnlineSession)
	{
		return;
	}

	Ws_Processing->LoadingMessage = TEXT_LEAVING_SESSION;
	SwitchContent(EContentType::LOADING);

	SessionOnlineSession->LeaveSession(
		SessionOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
}
// @@@SNIPEND

// @@@SNIPSTART CreateSessionWidget.cpp-OnLeaveSessionComplete
void UCreateSessionWidget::OnLeaveSessionComplete(FName SessionName, bool bSucceeded)
{
	// Abort if not a game session.
	if (!SessionName.IsEqual(SessionOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	if (bSucceeded)
	{
		SwitchContent(EContentType::CREATE);
	}
	else
	{
		Ws_Processing->ErrorMessage = TEXT_FAILED_TO_LEAVE_SESSION;
		Ws_Processing->bShowRetryButtonOnError = false;
		SwitchContent(EContentType::ERROR);
	}
}
// @@@SNIPEND

#pragma region "UI related"
void UCreateSessionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, CameraTargetY, 160.0f));
}

UWidget* UCreateSessionWidget::NativeGetDesiredFocusTarget() const
{
	return DesiredFocusTargetButton;
}

// @@@SNIPSTART CreateSessionWidget.cpp-SwitchContent
void UCreateSessionWidget::SwitchContent(const EContentType Type)
{
	UWidget* ContentTarget = nullptr;
	UWidget* FocusTarget = nullptr;
	EAccelByteWarsWidgetSwitcherState ProcessingWidgetState = EAccelByteWarsWidgetSwitcherState::Empty;
	bool bEnableBackButton = true;

	switch (Type)
	{
	case EContentType::CREATE:
		ContentTarget = W_Selection;
		FocusTarget = Btn_CreateSession;
		break;
	case EContentType::LOADING:
		ContentTarget = Ws_Processing;
		ProcessingWidgetState = EAccelByteWarsWidgetSwitcherState::Loading;
		bEnableBackButton = false;
		break;
	case EContentType::SUCCESS:
		ContentTarget = Ws_Processing;
		ProcessingWidgetState = EAccelByteWarsWidgetSwitcherState::Not_Empty;
		FocusTarget = Btn_Leave;
		break;
	case EContentType::ERROR:
		ContentTarget = Ws_Processing;
		ProcessingWidgetState = EAccelByteWarsWidgetSwitcherState::Error;
		FocusTarget = Btn_Back;
		break;
	default: ;
	}

	if (FocusTarget)
	{
		DesiredFocusTargetButton = FocusTarget;
		FocusTarget->SetUserFocus(GetOwningPlayer());
	}
	Ws_ContentOuter->SetActiveWidget(ContentTarget);
	if (ProcessingWidgetState != EAccelByteWarsWidgetSwitcherState::Empty)
	{
		Ws_Processing->SetWidgetState(ProcessingWidgetState);
	}

	Btn_Back->SetIsEnabled(bEnableBackButton);
	bIsBackHandler = bEnableBackButton;
	RequestRefreshFocus();

	// Set FTUEs
	if (Type == EContentType::SUCCESS)
	{
		InitializeFTUEDialogues(true);
	}
	else
	{
		DeinitializeFTUEDialogues();
	}
}
// @@@SNIPEND

#pragma endregion 