// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Core/System/AccelByteWarsGameInstance.h"

#include "CoreMinimal.h"
#include "OnlineError.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "InventoryWidget_Starter.generated.h"

class UCommonButtonBase;
class UAccelByteWarsAsyncImageWidget;
class UTextBlock;
class UTileView;
class UImage;
class UEntitlementsEssentialsSubsystem_Starter;
class UAccelByteWarsWidgetSwitcher;
class UWidgetSwitcher;
class UMultiLineEditableText;
class UMediaPlayer;

UCLASS(Abstract)
class ACCELBYTEWARS_API UInventoryWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	inline static FSimpleMulticastDelegate OnInventoryMenuDeactivated;
	FSimpleMulticastDelegate OnEquipped;
	FSimpleMulticastDelegate OnUnequipped;

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

private:
	UPROPERTY()
	UEntitlementsEssentialsSubsystem_Starter* EntitlementsSubsystem;

	UPROPERTY(EditAnywhere)
	bool bIsConsumable = true;

#pragma region "Tutorial"
	// Put your code here.
#pragma endregion

#pragma region "Byte Wars specific"
private:
	void OnClickListItem(UObject* Object);
	void OnClickEquip() const;
	void OnClickUnEquip() const;
#pragma endregion 

#pragma region "UI"
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMediaPlayer* MediaPlayer;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	void SwitchActionButton(bool bShowEquipButton) const;
	void SwitchToDefaultState() const;

private:
	UPROPERTY()
	UStoreItemDataObject* SelectedItem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Root;

	UPROPERTY(EditAnywhere)
	FSlateBrush DefaultPreviewImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UImage* W_SelectedItemPreview;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_SelectedItemTitle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UMultiLineEditableText* Tb_Description;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTileView* Tv_Content;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Equip;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Unequip;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;
#pragma endregion 
};
