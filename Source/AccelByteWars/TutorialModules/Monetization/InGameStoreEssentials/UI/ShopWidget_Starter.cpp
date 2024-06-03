// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "ShopWidget_Starter.h"

#include "CommonButtonBase.h"
#include "StoreItemDetailWidget.h"
#include "Components/TileView.h"
#include "Components/WidgetSwitcher.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/AccelByteWarsTabListWidget.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"

void UShopWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	StoreSubsystem = GetGameInstance()->GetSubsystem<UInGameStoreEssentialsSubsystem_Starter>();
	ensure(StoreSubsystem);

	// event binding
	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);

#pragma region "Tutorial"
	// put your code here
#pragma endregion 

	OnActivatedMulticastDelegate.Broadcast(GetOwningPlayer());
}

void UShopWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Back->OnClicked().RemoveAll(this);

#pragma region "Tutorial"
	// put your code here
#pragma endregion 
}

#pragma region "Tutorial"
// put your code here
#pragma endregion 

#pragma region "UI"
void UShopWidget_Starter::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

UWidget* UShopWidget_Starter::NativeGetDesiredFocusTarget() const
{
	UWidget* FocusTarget = Btn_Back;
	if (!Tv_ContentOuter->GetListItems().IsEmpty())
	{
		FocusTarget = Tv_ContentOuter;
	}
	return FocusTarget;
}

void UShopWidget_Starter::SwitchContent(EAccelByteWarsWidgetSwitcherState State) const
{
	UWidget* FocusTarget;

	switch (State)
	{
	case EAccelByteWarsWidgetSwitcherState::Loading:
		FocusTarget = Btn_Back;
		break;
	case EAccelByteWarsWidgetSwitcherState::Not_Empty:
		FocusTarget = Tv_ContentOuter;
		break;
	case EAccelByteWarsWidgetSwitcherState::Empty:
		FocusTarget = Btn_Back;
		break;
	default:
		FocusTarget = Btn_Back;
	}

	FocusTarget->SetUserFocus(GetOwningPlayer());
	Ws_Loader->SetWidgetState(State);
}
#pragma endregion 
