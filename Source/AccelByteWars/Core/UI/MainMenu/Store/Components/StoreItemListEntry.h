// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "StoreItemListEntry.generated.h"

class UTextBlock;
class UListView;
class UAccelByteWarsAsyncImageWidget;

UCLASS(Abstract)
class ACCELBYTEWARS_API UStoreItemListEntry final : public UAccelByteWarsActivatableWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

	virtual void NativePreConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;

public:
	void Setup(UItemDataObject* Object);
	void Setup(UStoreItemDataObject* Object);
	UItemDataObject* GetItemData() const { return ItemData; }

private:
	UPROPERTY(EditAnywhere)
	bool bShowPrices = true;

	UPROPERTY()
	UItemDataObject* ItemData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* W_EntitlementOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsAsyncImageWidget* W_Image;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Name;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_Prices;
};
