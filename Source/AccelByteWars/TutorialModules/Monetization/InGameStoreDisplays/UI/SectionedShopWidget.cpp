// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "SectionedShopWidget.h"

#include "Components/ListView.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Monetization/InGameStoreEssentials/UI/ShopWidget.h"
#include "Monetization/InGameStoreEssentials/UI/ShopWidget_Starter.h"

// @@@SNIPSTART SectionedShopWidget.cpp-NativeOnActivated
// @@@MULTISNIP InGameStoreDisplaysSubsystem {"selectedLines": ["1-2", "5-6", "23"]}
// @@@MULTISNIP Setup {"selectedLines": ["1-8", "14-23"]}
void USectionedShopWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	InGameStoreDisplaysSubsystem = GetGameInstance()->GetSubsystem<UInGameStoreDisplaysSubsystem>();
	ensure(InGameStoreDisplaysSubsystem);

	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	InGameStoreDisplaysSubsystem->QueryOrGetDisplays(
		GetOwningPlayer(),
		FOnQueryOrGetDisplaysComplete::CreateUObject(this, &ThisClass::OnQueryOrGetDisplaysCompleted),
		bRefreshing);

	// Bind parent's (store essentials) refresh function with this refresh function
	if (UShopWidget* ShopWidget = GetFirstOccurenceOuter<UShopWidget>())
	{
		ShopWidget->OnRefreshButtonClickedDelegates.AddUObject(this, &ThisClass::OnParentRefreshButtonClicked);
	}
	else if (UShopWidget_Starter* ShopWidget_Starter = GetFirstOccurenceOuter<UShopWidget_Starter>())
	{
		ShopWidget_Starter->OnRefreshButtonClickedDelegates.AddUObject(this, &ThisClass::OnParentRefreshButtonClicked);
	}
}
// @@@SNIPEND

// @@@SNIPSTART SectionedShopWidget.cpp-NativeOnDeactivated
void USectionedShopWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (UShopWidget* ShopWidget = GetFirstOccurenceOuter<UShopWidget>())
	{
		ShopWidget->OnRefreshButtonClickedDelegates.RemoveAll(this);
	}
	else if (UShopWidget_Starter* ShopWidget_Starter = GetFirstOccurenceOuter<UShopWidget_Starter>())
	{
		ShopWidget_Starter->OnRefreshButtonClickedDelegates.RemoveAll(this);
	}
}
// @@@SNIPEND

// @@@SNIPSTART SectionedShopWidget.cpp-OnParentRefreshButtonClicked
// @@@MULTISNIP Setup {"selectedLines": ["1-9", "14"]}
void USectionedShopWidget::OnParentRefreshButtonClicked()
{
	if (bRefreshing)
	{
		return;
	}
	bRefreshing = true;

	Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	InGameStoreDisplaysSubsystem->QueryOrGetDisplays(
		GetOwningPlayer(),
		FOnQueryOrGetDisplaysComplete::CreateUObject(this, &ThisClass::OnQueryOrGetDisplaysCompleted),
		bRefreshing);
}
// @@@SNIPEND

// @@@SNIPSTART SectionedShopWidget.cpp-OnQueryOrGetDisplaysCompleted
// @@@MULTISNIP Setup {"selectedLines": ["1-15", "21-32"]}
void USectionedShopWidget::OnQueryOrGetDisplaysCompleted(
	TArray<TSharedRef<FAccelByteModelsViewInfo>>& Displays,
	const FOnlineError& Error)
{
	if (Error.bSucceeded)
	{
		for (const TSharedRef<FAccelByteModelsViewInfo>& Display : Displays)
		{
			/**
			 * Trigger query sections for target display only
			 * Use Name to find the Display Name set on Admin Portal
			 * Use Title to retrieve the localized name set on Admin Portal
			 */
			if (Display->Name.Equals(TargetDisplayName))
			{
				InGameStoreDisplaysSubsystem->QueryOrGetSectionsForDisplay(
					GetOwningPlayer(),
					Display->ViewId,
					FOnQueryOrGetSectionsInDisplayComplete::CreateUObject(this, &ThisClass::OnQueryOrGetSectionsCompleted),
					bRefreshing);
				return;
			}
		}
		Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
	}
	else
	{
		Ws_Root->ErrorMessage = Error.ErrorMessage;
		Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		bRefreshing = false;
	}
}
// @@@SNIPEND

// @@@SNIPSTART SectionedShopWidget.cpp-OnQueryOrGetSectionsCompleted
// @@@MULTISNIP Setup {"selectedLines": ["1-20", "28-36"]}
void USectionedShopWidget::OnQueryOrGetSectionsCompleted(
	TArray<TSharedRef<FAccelByteModelsSectionInfo>>& Sections,
	const FOnlineError& Error)
{
	if (Error.bSucceeded)
	{
		if (Sections.IsEmpty())
		{
			Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Empty);
			return;
		}

		SectionDatas.Empty();
		QueryOrGetOffersInSectionCount = Sections.Num();
		for (int i = 0; i < Sections.Num(); ++i)
		{
			USectionDataObject* Data = NewObject<USectionDataObject>();
			Data->SectionInfo = Sections[i].Get();
			Data->SectionColor = GetSectionPresetColor(i);
			SectionDatas.Add(Data);

			InGameStoreDisplaysSubsystem->QueryOrGetOffersInSection(
				GetOwningPlayer(),
				Sections[i]->SectionId,
				FOnQueryOrGetOffersInSectionComplete::CreateUObject(
					this, &ThisClass::OnQueryOrGetOffersInSectionCompleted, Sections[i]->SectionId),
				bRefreshing);
		}
	}
	else
	{
		Ws_Root->ErrorMessage = Error.ErrorMessage;
		Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		bRefreshing = false;
	}
}
// @@@SNIPEND

// @@@SNIPSTART SectionedShopWidget.cpp-OnQueryOrGetOffersInSectionCompleted
void USectionedShopWidget::OnQueryOrGetOffersInSectionCompleted(
	TArray<UStoreItemDataObject*>& Offers,
	const FOnlineError& Error,
	const FString SectionId)
{
	if (Error.bSucceeded)
	{
		USectionDataObject* Filter = NewObject<USectionDataObject>();
		Filter->SectionInfo.SectionId = SectionId;
		if (USectionDataObject** Data = SectionDatas.FindByPredicate([SectionId](const USectionDataObject* Obj)
			{
				return Obj->SectionInfo.SectionId.Equals(SectionId);
			}))
		{
			(*Data)->Offers = Offers;
		}
	}
	else
	{
		Ws_Root->ErrorMessage = Error.ErrorMessage;
		Ws_Root->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);
		bRefreshing = false;
		return;
	}

	QueryOrGetOffersInSectionCount--;
	if (QueryOrGetOffersInSectionCount <= 0)
	{
		// Check if all data is empty or not.
		bool bIsNoOffers = true;
		for (const USectionDataObject* SectionData : SectionDatas)
		{
			if (!SectionData->Offers.IsEmpty())
			{
				bIsNoOffers = false;
				break;
			}
		}

		Ws_Root->SetWidgetState(bIsNoOffers ? EAccelByteWarsWidgetSwitcherState::Empty : EAccelByteWarsWidgetSwitcherState::Not_Empty);
		Lv_Root->SetListItems(SectionDatas);
		bRefreshing = false;
	}
}
// @@@SNIPEND

#pragma region "UI"
FLinearColor USectionedShopWidget::GetSectionPresetColor(const int Index) const
{
	const int TrueIndex = AccelByteWarsUtility::PositiveModulo(Index, SectionBackgroundColorPreset.Num());
	return SectionBackgroundColorPreset[TrueIndex];
}
#pragma endregion 
