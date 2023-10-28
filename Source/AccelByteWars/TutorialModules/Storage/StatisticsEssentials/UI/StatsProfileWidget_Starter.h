// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "OnlineStatisticInterfaceAccelByte.h"
#include "StatsProfileWidget_Starter.generated.h"

class UStatsEssentialsSubsystem_Starter;
class UWidgetSwitcher;
class UCommonButtonBase;
class UDynamicEntryBox;

UCLASS()
class ACCELBYTEWARS_API UStatsProfileWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnActivated() override;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Retry;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_Outer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_StatsListOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_LoadingOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_FailedOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_EmptyOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UDynamicEntryBox* Deb_StatsList;

	UPROPERTY()
	UStatsEssentialsSubsystem_Starter* EssentialsSubsystem;
};
