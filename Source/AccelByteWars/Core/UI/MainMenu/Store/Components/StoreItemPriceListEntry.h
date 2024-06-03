// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "StoreItemPriceListEntry.generated.h"

class UTextBlock;
class UImage;

UCLASS(Abstract)
class ACCELBYTEWARS_API UStoreItemPriceListEntry final : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual void NativePreConstruct() override;
#endif

public:
	void Setup(const UStoreItemPriceDataObject* DataObject, const int32 Multiplier = 1) const;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Change symbol for preview purpose"))
	ECurrencyType DebugCurrencyType;

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnEntryReleased() override;
	void ResetUI() const;

private:
	UPROPERTY(EditDefaultsOnly)
	FSlateBrush Brush_Coin;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush Brush_Gem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UImage* I_CurrencySymbol;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_RegularPrice;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_FinalPrice;
};
