// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "ShopWidget.generated.h"

class UInGameStoreEssentialsSubsystem;
class UCommonButtonBase;
class UTileView;
class UWidgetSwitcher;
class UStoreItemListEntry;
class UPanelWidget;
class UStoreItemDetailWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UShopWidget final : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnActivated() override;

protected:
	virtual void NativeOnDeactivated() override;
	virtual void NativeConstruct() override;

	void LoadStoreItems();
	void OnQueryOffersComplete(bool bWasSuccessful, FString ErrorMessage);

	void OnStoreItemClicked(UObject* Item) const;

private:
	UPROPERTY(EditAnywhere)
	TArray<FString> Categories;

	UPROPERTY()
	UInGameStoreEssentialsSubsystem* StoreSubsystem;

#pragma region "UI related"
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void SwitchContent(EAccelByteWarsWidgetSwitcherState State) const;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_ListOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Loader;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTileView* Tv_ContentOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UStoreItemDetailWidget> DetailWidgetClass;
#pragma endregion 
};
