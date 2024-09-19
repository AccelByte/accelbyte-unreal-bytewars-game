// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "OnlineStatisticInterfaceAccelByte.h"
#include "StatsProfileWidget.generated.h"

class UAccelByteWarsWidgetSwitcher;
class UCommonButtonBase;
class UDynamicEntryBox;
class UStatsEssentialsSubsystem;

UCLASS(Abstract)
class ACCELBYTEWARS_API UStatsProfileWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

// @@@SNIPSTART StatsProfileWidget.h-protected
// @@@MULTISNIP QueryStatsDeclaration {"selectedLines": ["1", "7-10"]}
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void QueryLocalUserStats();

	void OnQueryLocalUserStatsComplete(const FOnlineError& ResultState, const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStatsResult);
// @@@SNIPEND
	
// @@@SNIPSTART StatsProfileWidget.h-private
// @@@MULTISNIP StatsProfileUI {"selectedLines": ["1", "5-18"]}
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
	UStatsEssentialsSubsystem* StatsEssentialsSubsystem;
// @@@SNIPEND
};
