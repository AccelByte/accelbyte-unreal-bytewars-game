// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TutorialModuleOnlineUtility.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteWarsTutorialModuleOnlineUtility, Log, All);
#define UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAccelByteWarsTutorialModuleOnlineUtility, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}

UCLASS()
class ACCELBYTEWARS_API UTutorialModuleOnlineUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Tutorial Module Online Utility", meta = (WorldContext = "WorldContextObject"))
	static bool IsAccelByteSDKInitialized(const UObject* Target);
};