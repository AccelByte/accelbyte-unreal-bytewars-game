// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "OnlineSettings/RegionPreferencesEssentials/RegionPreferencesSubsystem.h"
#include "IngameRegionInfo.generated.h"

class UTextBlock;
UCLASS()
class ACCELBYTEWARS_API UIngameRegionInfo  : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
public:	
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Region;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Latency;
private:
	void OnLatencyRefreshed(float Latency);

	UPROPERTY()
	URegionPreferencesSubsystem* RegionPreferencesSubsystem;
	
};
