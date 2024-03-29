// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "HelpWidget.generated.h"

UCLASS()
class ACCELBYTEWARS_API UHelpWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
