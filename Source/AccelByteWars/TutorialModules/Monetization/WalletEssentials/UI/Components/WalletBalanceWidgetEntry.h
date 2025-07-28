// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "WalletBalanceWidgetEntry.generated.h"

class UTextBlock;
class UImage;

UCLASS(Abstract)
class ACCELBYTEWARS_API UWalletBalanceWidgetEntry : public UUserWidget
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual void NativePreConstruct() override;
#endif

// @@@SNIPSTART WalletBalanceWidgetEntry.h-public
// @@@MULTISNIP Setup {"selectedLines": ["1-2"]}
public:
	void Setup(const FText& Balance, const ECurrencyType CurrencyType) const;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Change symbol for preview purpose"))
	ECurrencyType DebugCurrencyType;
// @@@SNIPEND

// @@@SNIPSTART WalletBalanceWidgetEntry.h-private
private:
	UPROPERTY(EditDefaultsOnly)
	FSlateBrush Brush_Coin;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush Brush_Gem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Balance;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UImage* I_CurrencySymbol;
// @@@SNIPEND
};
