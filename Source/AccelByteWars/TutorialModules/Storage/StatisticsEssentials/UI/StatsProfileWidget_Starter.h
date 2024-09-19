// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "OnlineStatisticInterfaceAccelByte.h"
#include "StatsProfileWidget_Starter.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UCommonButtonBase;
class UDynamicEntryBox;
class UStatsEssentialsSubsystem_Starter;

UCLASS()
class ACCELBYTEWARS_API UStatsProfileWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

#pragma region Module Function Declarations
	// TODO: Add your protected function declarations here
#pragma endregion 

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Loader;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UDynamicEntryBox* Deb_SinglePlayerStats;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UDynamicEntryBox* Deb_EliminationStats;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UDynamicEntryBox* Deb_TeamDeathmatchStats;

	UPROPERTY()
	TMap<FName, UDynamicEntryBox*> StatsDataEntryList{};

	UPROPERTY()
	UStatsEssentialsSubsystem_Starter* StatsEssentialsSubsystem;
};
