// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"
#include "Monetization/InGameStoreEssentials/InGameStoreEssentialsSubsystem_Starter.h"
#include "ShopWidget_Starter.generated.h"

class UCommonButtonBase;
class UTileView;
class UWidgetSwitcher;
class UStoreItemListEntry;
class UPanelWidget;
class UStoreItemDetailWidget;
class UAccelByteWarsTabListWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UShopWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

protected:
#pragma region "Tutorial"
	// put your code here
#pragma endregion 

private:
	UPROPERTY(EditAnywhere)
	FString RootPath;

	UPROPERTY()
	UInGameStoreEssentialsSubsystem_Starter* StoreSubsystem;

#pragma region "UI"
public:
	inline static TMulticastDelegate<void(const APlayerController*)> OnActivatedMulticastDelegate;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	void SwitchContent(EAccelByteWarsWidgetSwitcherState State) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "Pressed Sound"))
	FSlateSound PressedSlateSound;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_ListOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Loader;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsTabListWidget* Tl_ItemCategory;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTileView* Tv_ContentOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> DetailWidgetClass;
#pragma endregion 
};
