// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CustomMatchmakingWidget_Starter.h"

#include "CommonButtonBase.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "TutorialModules/Play/CustomMatchmaking/CustomMatchmakingLog.h"
#include "TutorialModules/Play/CustomMatchmaking/CustomMatchmakingSubsystem_Starter.h"

void UCustomMatchmakingWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	SwitchWidget(EAccelByteWarsWidgetSwitcherState::Not_Empty);

	Subsystem = GetGameInstance()->GetSubsystem<UCustomMatchmakingSubsystem_Starter>();
	if (!Subsystem)
	{
		UE_LOG_CUSTOMMATCHMAKING(Fatal, TEXT("Can't retrieve UCustomMatchmakingSubsystem"))
	}

	// Bind button
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

#pragma region "Tutorial"
	// Place your implementation here
#pragma endregion
}

void UCustomMatchmakingWidget_Starter::NativeOnDeactivated()
{	
	Super::NativeOnDeactivated();

	// Unbind button
	Btn_Back->OnClicked().RemoveAll(this);

#pragma region "Tutorial"
	// Place your implementation here
#pragma endregion
}

void UCustomMatchmakingWidget_Starter::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

#pragma region "Tutorial"
// Place your implementation here
#pragma endregion

void UCustomMatchmakingWidget_Starter::SwitchWidget(const EAccelByteWarsWidgetSwitcherState State)
{
	UWidget* FocusTarget = W_Root;
	bool bIsBackable = true;

	switch (State)
	{
	case EAccelByteWarsWidgetSwitcherState::Loading:
		bIsBackable = false;
		break;
	case EAccelByteWarsWidgetSwitcherState::Not_Empty:
		FocusTarget = Btn_StartMatchmaking;
		break;
	}

	W_Root->SetWidgetState(State);

	FocusTarget->SetUserFocus(GetOwningPlayer());

	bIsBackHandler = bIsBackable;
	Btn_Back->SetVisibility(bIsBackable ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	W_Root->ForceRefresh();
}
