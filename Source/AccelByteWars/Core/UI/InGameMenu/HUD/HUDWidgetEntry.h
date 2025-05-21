// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
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
	void Init(const FGameplayTeamData& Team);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Lives;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Score;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Kills;

	UPROPERTY(meta = (ExposeOnSpawn = true), EditAnywhere, BlueprintReadWrite)
	bool bHidePowerUpWidgets = false;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHorizontalBox* Hb_PowerUps;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPowerUpWidgetEntry> PowerUpWidgetEntryClass;

	TMap<int32, TWeakObjectPtr<UPowerUpWidgetEntry>> PowerUpWidgetEntries;
};
