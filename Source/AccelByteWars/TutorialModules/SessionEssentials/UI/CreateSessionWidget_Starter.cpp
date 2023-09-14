// Fill out your copyright notice in the Description page of Project Settings.


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

	SessionOnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
	if (!ensure(SessionOnlineSession))
	{
		return;
	}

	// TODO: Bind delegates

	// Change UI state based on the current player's session status
	const FNamedOnlineSession* OnlineSession = SessionOnlineSession->GetSession(
		SessionOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	SwitchContent(OnlineSession ? EContentType::SUCCESS : EContentType::CREATE);
}

void UCreateSessionWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_CreateSession->OnClicked().RemoveAll(this);
	Btn_Back->OnClicked().RemoveAll(this);
	Btn_Leave->OnClicked().RemoveAll(this);
	Ws_Processing->OnRetryClicked.RemoveAll(this);

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
}
#pragma endregion 
