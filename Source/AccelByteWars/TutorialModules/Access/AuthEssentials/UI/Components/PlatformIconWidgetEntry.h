// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CommonUserWidget.h"
#include "Models/AccelByteUserModels.h"
#include "PlatformIconWidgetEntry.generated.h"

class UImage;

UCLASS()
class UPlatformWidgetData : public UObject
{
	GENERATED_BODY()

public:
	void Init(const FUniqueNetIdPtr Id)
	{
		NetId = Id;
	}

	FUniqueNetIdPtr GetNetId() const
	{
		return NetId;
	}

protected:
	FUniqueNetIdPtr NetId;
};

UCLASS(Abstract)
class ACCELBYTEWARS_API UPlatformIconWidgetEntry : public UCommonUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

private:
	UPROPERTY(EditAnywhere)
	TMap<EAccelBytePlatformType, FSlateBrush> BrushMap;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UImage* Img_Icon;
};
