// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "BrowseMatchWidget.h"

#include "CommonButtonBase.h"
#include "Components/ListView.h"
#include "Components/WidgetSwitcher.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"

void UBrowseMatchWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_Joining_Back->OnClicked().AddUObject(this, &ThisClass::SwitchContent, EContentType::BROWSE_NOT_EMPTY);
}

void UBrowseMatchWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Back->OnClicked().RemoveAll(this);
	Btn_Joining_Back->OnClicked().RemoveAll(this);
}

void UBrowseMatchWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, CameraTargetY, 160.0f));
}

UWidget* UBrowseMatchWidget::NativeGetDesiredFocusTarget() const
{
	return DesiredFocusTargetButton;
}

void UBrowseMatchWidget::SetLoadingMessage(const FText& Text, const bool bBrowse, const bool bEnableCancelButton) const
{
	if (bBrowse)
	{
		Ws_Browse_Content->LoadingMessage = Text;
	}
	else
	{
		Ws_Joining->LoadingMessage = Text;
		Ws_Joining->bEnableCancelButton = bEnableCancelButton;
	}
}

void UBrowseMatchWidget::SetErrorMessage(const FText& Text, const bool bBrowse) const
{
	if (bBrowse)
	{
		Ws_Browse_Content->ErrorMessage = Text;
	}
	else
	{
		Ws_Joining->ErrorMessage = Text;
	}
}

void UBrowseMatchWidget::SwitchContent(const EContentType Type)
{
	bool bBrowseMenu = false;
	EAccelByteWarsWidgetSwitcherState State = EAccelByteWarsWidgetSwitcherState::Loading;

	bool bJoin_ShowBackButton = false;

	UWidget* FocusTarget = Btn_Back;

	switch (Type)
	{
	case EContentType::BROWSE_LOADING:
		bBrowseMenu = true;
		State = EAccelByteWarsWidgetSwitcherState::Loading;
		CameraTargetY = 600.0f;
		FocusTarget = Btn_Back;
		break;
	case EContentType::BROWSE_EMPTY:
		bBrowseMenu = true;
		State = EAccelByteWarsWidgetSwitcherState::Empty;
		CameraTargetY = 600.0f;
		FocusTarget = Btn_Back;
		break;
	case EContentType::BROWSE_NOT_EMPTY:
		bBrowseMenu = true;
		State = EAccelByteWarsWidgetSwitcherState::Not_Empty;
		CameraTargetY = 600.0f;
		FocusTarget = Lv_Sessions;
		break;
	case EContentType::BROWSE_ERROR:
		bBrowseMenu = true;
		State = EAccelByteWarsWidgetSwitcherState::Error;
		CameraTargetY = 600.0f;
		FocusTarget = W_ActionButtonsOuter->HasAnyChildren() ? W_ActionButtonsOuter->GetChildAt(0) : Btn_Back;
		break;
	case EContentType::JOIN_LOADING:
		bBrowseMenu = false;
		State = EAccelByteWarsWidgetSwitcherState::Loading;
		bJoin_ShowBackButton = false;
		CameraTargetY = 750.0f;
		FocusTarget = Ws_Joining;
		break;
	case EContentType::JOIN_ERROR:
		bBrowseMenu = false;
		State = EAccelByteWarsWidgetSwitcherState::Error;
		bJoin_ShowBackButton = true;
		CameraTargetY = 750.0f;
		FocusTarget = Btn_Joining_Back;
		break;
	}

	DesiredFocusTargetButton = FocusTarget;
	FocusTarget->SetUserFocus(GetOwningPlayer());
	Ws_Root->SetActiveWidget(bBrowseMenu ? W_Browse_Outer : W_Joining_Outer);
	if (bBrowseMenu)
	{
		Ws_Browse_Content->SetWidgetState(State);
	}
	else
	{
		Btn_Joining_Back->SetVisibility(bJoin_ShowBackButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		Ws_Joining->SetWidgetState(State);
	}
	RequestRefreshFocus();

	// Set FTUE
	if (Type == EContentType::BROWSE_EMPTY || Type == EContentType::BROWSE_NOT_EMPTY) 
	{
		InitializeFTUEDialogues(true);
	}
	else 
	{
		DeinitializeFTUEDialogues();
	}
}
