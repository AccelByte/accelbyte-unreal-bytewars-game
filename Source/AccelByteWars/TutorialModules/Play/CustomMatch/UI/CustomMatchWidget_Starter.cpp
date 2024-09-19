// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "CustomMatchWidget_Starter.h"

#include "CommonButtonBase.h"
#include "Core/UI/Components/AccelByteWarsSequentialSelectionWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Play/CustomMatch/CustomMatchSubsystem.h"
#include "Play/CustomMatch/CustomMatchModels.h"

void UCustomMatchWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	W_GameModeTypeSelection->OnSelectionChangedDelegate.AddUObject(this, &ThisClass::OnGameModeTypeSelectionChanged);
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Btn_Back_Error->OnClicked().AddUObject(this, &ThisClass::ReturnToCreateMenu);

	SwitchContent(EContentType::CREATE);
	OnGameModeTypeSelectionChanged(W_GameModeTypeSelection->GetSelectedIndex());

#pragma region "Tutorial"
	// Insert your codes here
#pragma endregion 
}

void UCustomMatchWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	W_GameModeTypeSelection->OnSelectionChangedDelegate.RemoveAll(this);
	Btn_Back->OnClicked().RemoveAll(this);
	Btn_Back_Error->OnClicked().RemoveAll(this);

#pragma region "Tutorial"
	// Insert your codes here
#pragma endregion 
}

#pragma region "Tutorial"
// Insert your codes here
#pragma endregion

#pragma region "UI related"
void UCustomMatchWidget_Starter::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 750.0f, 160.0f));
}

void UCustomMatchWidget_Starter::OnGameModeTypeSelectionChanged(int32 Index) const
{
	W_MaxTeamSelection->SetVisibility(static_cast<EGameModeType>(Index) == EGameModeType::TDM ?
		ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UCustomMatchWidget_Starter::SwitchContent(const EContentType State)
{
	ESlateVisibility BackButtonVisibility;
	ESlateVisibility ErrorBackButtonVisibility;
	EAccelByteWarsWidgetSwitcherState WidgetSwitcherState;
	UWidget* FocusTarget;

	switch (State)
	{
	case EContentType::CREATE:
		BackButtonVisibility = ESlateVisibility::Visible;
		ErrorBackButtonVisibility = ESlateVisibility::Collapsed;
		WidgetSwitcherState = EAccelByteWarsWidgetSwitcherState::Not_Empty;
		FocusTarget = W_GameModeTypeSelection;
		break;
	case EContentType::LOADING:
		BackButtonVisibility = ESlateVisibility::Collapsed;
		ErrorBackButtonVisibility = ESlateVisibility::Collapsed;
		WidgetSwitcherState = EAccelByteWarsWidgetSwitcherState::Loading;
		FocusTarget = Ws_Root;
		break;
	case EContentType::ERROR:
		BackButtonVisibility = ESlateVisibility::Collapsed;
		ErrorBackButtonVisibility = ESlateVisibility::Visible;
		WidgetSwitcherState = EAccelByteWarsWidgetSwitcherState::Error;
		FocusTarget = Ws_Root;
		break;
	default:
		BackButtonVisibility = ESlateVisibility::Visible;
		ErrorBackButtonVisibility = ESlateVisibility::Collapsed;
		WidgetSwitcherState = EAccelByteWarsWidgetSwitcherState::Not_Empty;
		FocusTarget = W_GameModeTypeSelection;
	}

	Ws_Root->SetWidgetState(WidgetSwitcherState);
	Btn_Back_Error->SetVisibility(ErrorBackButtonVisibility);
	Btn_Back->SetVisibility(BackButtonVisibility);
	FocusTarget->SetUserFocus(GetOwningPlayer());
	bIsBackHandler = BackButtonVisibility == ESlateVisibility::Visible;
}

void UCustomMatchWidget_Starter::ReturnToCreateMenu()
{
	SwitchContent(EContentType::CREATE);
#pragma region "Tutorial"
	// Insert your codes here
#pragma endregion
}
#pragma endregion 
