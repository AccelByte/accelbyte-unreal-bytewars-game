// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Components/TextBlock.h"
#include "HUDWidgetEntry.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UHUDWidgetEntry : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativePreConstruct() override;

public:
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

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Label_Left;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Label_Middle;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Label_Right;
};
