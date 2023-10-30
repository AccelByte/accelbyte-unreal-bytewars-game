// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteWarsWidgetInterface.generated.h"

UINTERFACE(BlueprintType)
class ACCELBYTEWARS_API UAccelByteWarsWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class ACCELBYTEWARS_API IAccelByteWarsWidgetInterface 
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "AccelByteWars Widget Interface")
	void ToggleHighlight(const bool bToHighlight);
};