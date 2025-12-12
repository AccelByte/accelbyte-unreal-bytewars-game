// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "HUDShipLabelWidget.generated.h"

class UTextBlock;

UCLASS()
class ACCELBYTEWARS_API UHUDShipLabelWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	void Setup(const FGameplayPlayerData& Data);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_PlayerName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_LocalPlayerIndicator;
};
