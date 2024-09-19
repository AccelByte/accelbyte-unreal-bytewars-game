// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "PlatformIconWidgetEntry.h"
#include "Core/AccelByteUtilities.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "AccelByteWarsPlatformWidget.generated.h"

class UImage;
class UListView;

UCLASS(Abstract)
class ACCELBYTEWARS_API UAccelByteWarsPlatformWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

protected:
	UFUNCTION()
	void SetupByWidgetEntry(UAccelByteWarsWidgetEntry* WidgetEntry);

	void Setup(FUniqueNetIdPtr NetIdPtr);

	UPlatformWidgetData* UserIdToPlatformWidgetData(const FUniqueNetIdPtr NetId) const;

	bool bShowAll = false;

private:
#pragma region "UI components"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_Icon;
#pragma endregion
};
