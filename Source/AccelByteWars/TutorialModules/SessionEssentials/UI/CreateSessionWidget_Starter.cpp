// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "CreateSessionWidget_Starter.h"

#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "TutorialModules/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"
#include "TutorialModules/SessionEssentials/SessionEssentialsModel.h"

void UCreateSessionWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	// TODO: Bind UI delegates

	SessionOnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
	if (!ensure(SessionOnlineSession))
	{
		return;
	}

	// TODO: Bind delegates

	// Set initial UI state.
	SwitchContent(EContentType::CREATE);

	// Set UI state to success if already in a session.
	const FName SessionName = SessionOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession);
	const FNamedOnlineSession* OnlineSession = SessionOnlineSession->GetSession(SessionName);
	if (OnlineSession)
	{
		// TODO: Call function to set the success state.
	}
}

void UCreateSessionWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Back->OnClicked().RemoveAll(this);
	// TODO: Unbind UI delegates

	if (SessionOnlineSession)
	{
		// TODO: Unbind delegates
	}
}

#pragma region "Function defenitions"
// TODO: Add your function declarations here
#pragma endregion 

#pragma region "UI related"
void UCreateSessionWidget_Starter::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, CameraTargetY, 160.0f));
}

UWidget* UCreateSessionWidget_Starter::NativeGetDesiredFocusTarget() const
{
	return Btn_Back;
}

void UCreateSessionWidget_Starter::SwitchContent(const EContentType Type)
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

	Ws_ContentOuter->SetActiveWidget(ContentTarget);
	if (ProcessingWidgetState != EAccelByteWarsWidgetSwitcherState::Empty)
	{
		Ws_Processing->SetWidgetState(ProcessingWidgetState);
	}

	Btn_Back->SetIsEnabled(bEnableBackButton);
	bIsBackHandler = bEnableBackButton;

	if (FocusTarget)
	{
		FocusTarget->SetUserFocus(GetOwningPlayer());
	}

	// Set FTUEs
	if (Type == EContentType::SUCCESS)
	{
		InitializeFTEUDialogues(true);
	}
	else
	{
		DeinitializeFTUEDialogues();
	}
}
#pragma endregion 
