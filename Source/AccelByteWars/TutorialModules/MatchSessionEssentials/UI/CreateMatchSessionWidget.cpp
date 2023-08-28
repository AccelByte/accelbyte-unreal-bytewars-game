// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "CreateMatchSessionWidget.h"

#include "CommonButtonBase.h"
#include "Components/WidgetSwitcher.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"

void UCreateMatchSessionWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_GameModeType_BackToCreateSession->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_ServerType_BackToCreateSession->OnClicked().AddUObject(
		this, &ThisClass::SwitchContent, EContentType::SELECT_GAMEMODE);
	Btn_Error_BackToCreateSession->OnClicked().AddUObject(
		this, &ThisClass::SwitchContent, EContentType::SELECT_GAMEMODE);

	Btn_Elimination->OnClicked().AddUObject(this, &ThisClass::SetSelectedGameMode, EGameModeType::FFA);
	Btn_TeamDeathMatch->OnClicked().AddUObject(this, &ThisClass::SetSelectedGameMode, EGameModeType::TDM);

	SwitchContent(EContentType::SELECT_GAMEMODE);
}

void UCreateMatchSessionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, CameraTargetY, 160.0f));
}

void UCreateMatchSessionWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_GameModeType_BackToCreateSession->OnClicked().RemoveAll(this);
	Btn_ServerType_BackToCreateSession->OnClicked().RemoveAll(this);
	Btn_Error_BackToCreateSession->OnClicked().RemoveAll(this);
	Btn_Elimination->OnClicked().RemoveAll(this);
	Btn_TeamDeathMatch->OnClicked().RemoveAll(this);
}

UWidget* UCreateMatchSessionWidget::NativeGetDesiredFocusTarget() const
{
	return Btn_TeamDeathMatch;
}

void UCreateMatchSessionWidget::SetSelectedGameMode(EGameModeType GameModeType)
{
	SelectedGameModeType = GameModeType;

	SwitchContent(EContentType::SELECT_NETWORKTYPE);
}

void UCreateMatchSessionWidget::SetLoadingMessage(const FText& Message, const bool bEnableCancelButton) const
{
	Ws_Processing->LoadingMessage = Message;
	Ws_Processing->bEnableCancelButton = bEnableCancelButton;
}

void UCreateMatchSessionWidget::SetErrorMessage(const FText& Message, const bool bShowRetryButton) const
{
	Ws_Processing->ErrorMessage = Message;
	Ws_Processing->bShowRetryButtonOnError = bShowRetryButton;
}

void UCreateMatchSessionWidget::SwitchContent(const EContentType ContentType)
{
	UWidget* Target = nullptr;
	UWidget* FocusTarget = Btn_GameModeType_BackToCreateSession;

	bool bShowBackButton = true;

	switch (ContentType)
	{
	case EContentType::SELECT_GAMEMODE:
		Target = W_SelectGameModeType;
		CameraTargetY = 600.0f;
		FocusTarget = Btn_Elimination;
		break;
	case EContentType::SELECT_NETWORKTYPE:
		Target = W_SelectGameModeNetworkType;
		CameraTargetY = 750.0f;
		FocusTarget = W_SelectGameModeNetworkTypeButtonOuter->HasAnyChildren() ?
			W_SelectGameModeNetworkTypeButtonOuter->GetChildAt(0) :
			Btn_ServerType_BackToCreateSession;
		break;
	case EContentType::LOADING:
		Target = W_Processing;
		CameraTargetY = 825.0f;
		Ws_Processing->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
		FocusTarget = Ws_Processing;
		bShowBackButton = false;
		break;
	case EContentType::ERROR:
		Target = W_Processing;
		Ws_Processing->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		CameraTargetY = 825.0f;
		FocusTarget = Ws_Processing;
		bShowBackButton = false;
		break;
	}

	FocusTarget->SetUserFocus(GetOwningPlayer());
	Ws_ContentOuter->SetActiveWidget(Target);
	Btn_GameModeType_BackToCreateSession->SetVisibility(bShowBackButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
