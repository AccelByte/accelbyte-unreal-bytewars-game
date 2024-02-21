// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/PowerUps/PowerUpModels.h"
#include "HUDWidgetEntry.generated.h"

class UTextBlock;
class UPowerUpWidgetEntry;
class UHorizontalBox;

UCLASS()
class ACCELBYTEWARS_API UHUDWidgetEntry : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativePreConstruct() override;

public:
	UFUNCTION(BlueprintCallable)
	void SetPowerUpValues(const TArray<TEnumAsByte<EPowerUpSelection>>& SelectedPowerUps, const TArray<int32>& PowerUpCounts);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Value_Left;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Value_Middle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Value_Right;

	UPROPERTY(meta = (ExposeOnSpawn = true), EditAnywhere, BlueprintReadWrite)
	FLinearColor Color;

	UPROPERTY(meta = (ExposeOnSpawn = true), EditAnywhere, BlueprintReadWrite)
	FText LabelLeftText;

	UPROPERTY(meta = (ExposeOnSpawn = true), EditAnywhere, BlueprintReadWrite)
	FText LabelMiddleText;

	UPROPERTY(meta = (ExposeOnSpawn = true), EditAnywhere, BlueprintReadWrite)
	FText LabelRightText;

	UPROPERTY(meta = (ExposeOnSpawn = true), EditAnywhere, BlueprintReadWrite)
	bool bHideLeftRight = false;

	UPROPERTY(meta = (ExposeOnSpawn = true), EditAnywhere, BlueprintReadWrite)
	bool bHidePowerUpWidgets = false;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Label_Left;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Label_Middle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Label_Right;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHorizontalBox* Hb_PowerUps;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPowerUpWidgetEntry* Widget_PowerUpP1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPowerUpWidgetEntry* Widget_PowerUpP2;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPowerUpWidgetEntry* Widget_PowerUpP3;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPowerUpWidgetEntry* Widget_PowerUpP4;

	TArray<UPowerUpWidgetEntry*> PowerUpWidgets;
};
