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

private:
	void OnQueryOrGetDisplaysCompleted(TArray<TSharedRef<FAccelByteModelsViewInfo>>& Displays, const FOnlineError& Error);
	void OnQueryOrGetSectionsCompleted(TArray<TSharedRef<FAccelByteModelsSectionInfo>>& Sections, const FOnlineError& Error);
	void OnQueryOrGetOffersInSectionCompleted(TArray<UStoreItemDataObject*>& Offers, const FOnlineError& Error, FString SectionId);

	UPROPERTY()
	TArray<USectionDataObject*> SectionDatas;

	int QueryOrGetOffersInSectionCount = 0;

	UPROPERTY(EditAnywhere)
	FString TargetDisplayName;

	UPROPERTY()
	UInGameStoreDisplaysSubsystem* InGameStoreDisplaysSubsystem;

#pragma region "UI"
private:
	FLinearColor GetSectionPresetColor(const int Index) const;

	UFUNCTION()
	void OnParentRefreshButtonClicked();

	bool bRefreshing = false;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Root;

	const TArray<FLinearColor> SectionBackgroundColorPreset = {
		FLinearColor(0.27f, 0.004f, 0.3f),
		FLinearColor(0.2f, 0.25f, 0.3f),
		FLinearColor(0.32f, 0.24f, 0.23f)
	};
#pragma endregion 
};
