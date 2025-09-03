// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteWarsButtonBase.h"
#include "Core/Utilities/AccelByteWarsUtilityLog.h"
#include "CommonTabListWidgetBase.h"
#include "CommonAnimatedSwitcher.h"
#include "AccelByteWarsTabListWidget.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteWarsTabListWidget, Log, All);
#define UE_LOG_ACCELBYTEWARSTABLISTWIDGET(Verbosity, Format, ...) \
{ \
	UE_LOG_FUNC(LogAccelByteWarsTabListWidget, Verbosity, Format, ##__VA_ARGS__) \
}

class UCommonActionWidget;
class UCommonButtonStyle;

USTRUCT(BlueprintType)
struct FAccelByteWarsTabDescriptor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FName TabId;

	UPROPERTY(EditAnywhere)
	FText ButtonText;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCommonButtonBase> ButtonClass;

	UPROPERTY(EditAnywhere)
	UWidget* TabContent;

	UPROPERTY(EditAnywhere)
	int32 TabIndex = INDEX_NONE;
};

UCLASS(Abstract)
class ACCELBYTEWARS_API UAccelByteWarsTabListWidget : public UCommonTabListWidgetBase
{
	GENERATED_BODY()

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;
	virtual void HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;

public:
	void ParentOnActivated();
	void ParentOnDeactivated();

	UFUNCTION()
	bool GetUsePresetButtonClass() const { return bUsePresetButtonClass; }

	/**
	 * @brief Use this instead of RegisterTab to use the PresetButtonClass
	 * @param TabNameID The ID of the new tab
	 * @param ButtonText Button text of the new tab
	 * @param ContentWidget Content widget of the tab to switch to. Will be created if not yet exist in the LinkedSwitcher
	 * @param TabIndex Where should the tab button be placed. -1 to add to last.
	 * @param bForce if true and tab with the same TabNameID found, unregister it and register this new one
	 */
	UFUNCTION(BlueprintCallable, Category = TabList)
	bool RegisterTabWithPresets(
		const FName TabNameID,
		const FText ButtonText,
		UWidget* ContentWidget = nullptr,
		const int32 TabIndex = -1,
		const bool bForce = false);

protected:
	UPROPERTY(EditAnywhere)
	TArray<FAccelByteWarsTabDescriptor> PreRegisteredTabInfos;

	UPROPERTY(EditAnywhere)
	UCommonAnimatedSwitcher* PreLinkedSwitcher;

	/**
	 * @brief If true, use the PresetButtonClass as the tab button instead of the setting it individually per tab entry
	 */
	UPROPERTY(EditAnywhere)
	bool bUsePresetButtonClass = false;

	UPROPERTY(EditAnywhere, meta = (EditConditionHides, EditCondition = bUsePresetButtonClass))
	TSubclassOf<UAccelByteWarsButtonBase> PresetButtonClass;

	UPROPERTY(EditAnywhere, meta = (EditConditionHides, EditCondition = bUsePresetButtonClass))
	TSubclassOf<UCommonButtonStyle> PresetButtonStyle;

	UPROPERTY(EditAnywhere, meta = (EditConditionHides, EditCondition = bUsePresetButtonClass))
	FMargin PaddingBetweenButtons;

	UPROPERTY(EditAnywhere, meta = (EditConditionHides, EditCondition = bUsePresetButtonClass))
	int32 ButtonPreviewNum = 0;

private:
	UFUNCTION()
	void HandleOnTabButtonCreation(FName TabId, UCommonButtonBase* TabButton);

	UFUNCTION()
	void HandleOnTabSelected(FName TabId);

	bool UpdateTabButtonStyle(const FName& TabNameID, const FText& ButtonText = FText());

	FName PreviouslySelectedTabId;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* PreviousTabActionWrapper;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActionWidget* PreviousTabAction;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* NextTabActionWrapper;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonActionWidget* NextTabAction;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* W_TabButtonContainer;
};
