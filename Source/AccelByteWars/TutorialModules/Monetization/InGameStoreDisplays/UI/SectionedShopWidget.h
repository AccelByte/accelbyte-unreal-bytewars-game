// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Monetization/InGameStoreDisplays/InGameStoreDisplaysSubsystem.h"
#include "SectionedShopWidget.generated.h"

class UListView;
class UAccelByteWarsWidgetSwitcher;

UCLASS(Abstract)
class ACCELBYTEWARS_API USectionedShopWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

// @@@SNIPSTART SectionedShopWidget.h-private
// @@@MULTISNIP InGameStoreDisplaysSubsystem {"selectedLines": ["1-3"]}
// @@@MULTISNIP SectionDatas {"selectedLines": ["1", "5-6"]}
// @@@MULTISNIP TargetDisplayName {"selectedLines": ["1", "8-9"]}
// @@@MULTISNIP Variables {"selectedLines": ["1", "11-12"]}
// @@@MULTISNIP Functions {"selectedLines": ["1", "14-26"]}
private:
	UPROPERTY()
	UInGameStoreDisplaysSubsystem* InGameStoreDisplaysSubsystem;

	UPROPERTY()
	TArray<USectionDataObject*> SectionDatas;
	
	UPROPERTY(EditAnywhere)
	FString TargetDisplayName;

	bool bRefreshing = false;
	int QueryOrGetOffersInSectionCount = 0;

	UFUNCTION()
	void OnParentRefreshButtonClicked();

	void OnQueryOrGetDisplaysCompleted(
		TArray<TSharedRef<FAccelByteModelsViewInfo>>& Displays,
		const FOnlineError& Error);
	void OnQueryOrGetSectionsCompleted(
		TArray<TSharedRef<FAccelByteModelsSectionInfo>>& Sections,
		const FOnlineError& Error);
	void OnQueryOrGetOffersInSectionCompleted(
		TArray<UStoreItemDataObject*>& Offers,
		const FOnlineError& Error,
		FString SectionId);
// @@@SNIPEND

#pragma region "UI"
// @@@SNIPSTART SectionedShopWidget.h-ui-private
// @@@MULTISNIP UI {"selectedLines": ["1", "10-14"]}
// @@@MULTISNIP SectionColor {"selectedLines": ["1-8"]}
private:
	FLinearColor GetSectionPresetColor(const int Index) const;

	const TArray<FLinearColor> SectionBackgroundColorPreset = {
		FLinearColor(0.27f, 0.004f, 0.3f),
		FLinearColor(0.2f, 0.25f, 0.3f),
		FLinearColor(0.32f, 0.24f, 0.23f)
	};

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Root;
// @@@SNIPEND
#pragma endregion 
};
