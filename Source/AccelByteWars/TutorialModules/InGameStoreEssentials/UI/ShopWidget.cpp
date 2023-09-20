// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "ShopWidget.h"

#include "CommonButtonBase.h"
#include "StoreItemDetailWidget.h"
#include "Components/TileView.h"
#include "Components/WidgetSwitcher.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "TutorialModules/InGameStoreEssentials/InGameStoreEssentialsSubsystem.h"

void UShopWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Back->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	StoreSubsystem->OnQueryOfferCompleteDelegate.AddUObject(this, &ThisClass::OnQueryOffersComplete);

	Tv_ContentOuter->OnItemClicked().AddUObject(this, &ThisClass::OnStoreItemClicked);

	SwitchContent(EAccelByteWarsWidgetSwitcherState::Loading);
	if (!StoreSubsystem->IsQueryRunning())
	{
		StoreSubsystem->QueryOffers(GetOwningPlayer());
	}

	OnActivatedMulticastDelegate.Broadcast(GetOwningPlayer());
}

void UShopWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	Btn_Back->OnClicked().RemoveAll(this);
	StoreSubsystem->OnQueryOfferCompleteDelegate.RemoveAll(this);
	Tv_ContentOuter->OnItemClicked().RemoveAll(this);

	SwitchContent(EAccelByteWarsWidgetSwitcherState::Loading);
}

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	StoreSubsystem = GetGameInstance()->GetSubsystem<UInGameStoreEssentialsSubsystem>();
	ensure(StoreSubsystem);
}

void UShopWidget::LoadStoreItems()
{
	Tv_ContentOuter->ClearListItems();

	TArray<UStoreItemDataObject*> StoreItems;
	for (const FString& Category : Categories)
	{
		StoreItems.Append(StoreSubsystem->GetOffersByCategory(Category));
	}

	if (StoreItems.IsEmpty())
	{
		SwitchContent(EAccelByteWarsWidgetSwitcherState::Empty);
	}
	else
	{
		Tv_ContentOuter->SetListItems(StoreItems);
		SwitchContent(EAccelByteWarsWidgetSwitcherState::Not_Empty);
	}
}

void UShopWidget::OnQueryOffersComplete(bool bWasSuccessful, FString ErrorMessage)
{
	if (bWasSuccessful)
	{
		LoadStoreItems();
	}
	else
	{
		Ws_Loader->ErrorMessage = FText::FromString(ErrorMessage);
		Ws_Loader->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
	}
}

void UShopWidget::OnStoreItemClicked(UObject* Item) const
{
	Tv_ContentOuter->ClearListItems();
	Tv_ContentOuter->RegenerateAllEntries();
	Ws_Loader->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);

	UStoreItemDataObject* StoreItemDataObject = Cast<UStoreItemDataObject>(Item);
	if (!StoreItemDataObject)
	{
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);
	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	ensure(BaseUIWidget);

	UStoreItemDetailWidget* DetailsWidget =
		Cast<UStoreItemDetailWidget>(BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, DetailWidgetClass));
	ensure(DetailsWidget);

	DetailsWidget->Setup(StoreItemDataObject);
}

void UShopWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

UWidget* UShopWidget::NativeGetDesiredFocusTarget() const
{
	UWidget* FocusTarget = Btn_Back;
	if (!Tv_ContentOuter->GetListItems().IsEmpty())
	{
		FocusTarget = Tv_ContentOuter;
	}
	return FocusTarget;
}

void UShopWidget::SwitchContent(EAccelByteWarsWidgetSwitcherState State) const
{
	UWidget* FocusTarget;

	switch (State)
	{
	case EAccelByteWarsWidgetSwitcherState::Loading:
		FocusTarget = Btn_Back;
		break;
	case EAccelByteWarsWidgetSwitcherState::Not_Empty:
		FocusTarget = Btn_Back;
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
