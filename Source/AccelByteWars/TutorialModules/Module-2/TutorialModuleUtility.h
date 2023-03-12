// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuthEssentialsLog.h"
#include "TutorialModuleUtility.generated.h"

UCLASS()
class ACCELBYTEWARS_API UTutorialModuleUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Tutorial Module Utility", meta = (WorldContext = "WorldContextObject"))
	static bool IsAccelByteSDKInitialized(const UObject* Target);
};