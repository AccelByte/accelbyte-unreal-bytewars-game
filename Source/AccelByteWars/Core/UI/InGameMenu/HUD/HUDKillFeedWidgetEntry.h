// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "HUDKillFeedWidgetEntry.generated.h"

class UTextBlock;

UCLASS()
class ACCELBYTEWARS_API UHUDKillFeedData : public UObject
{
	GENERATED_BODY()

public:
	FGameplayPlayerData Killer {};
	FGameplayPlayerData Victim {};
};

UCLASS(Abstract)
class ACCELBYTEWARS_API UHUDKillFeedWidgetEntry : public UAccelByteWarsWidgetEntry
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Killer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Victim;
};
