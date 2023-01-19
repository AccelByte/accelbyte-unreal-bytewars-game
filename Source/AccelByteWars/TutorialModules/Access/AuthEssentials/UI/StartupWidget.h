// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Access/AuthEssentials/AuthEssentialsLog.h"
#include "StartupWidget.generated.h"

UCLASS()
class ACCELBYTEWARS_API UStartupWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
protected:
	UFUNCTION(BlueprintCallable)
	bool IsAccelByteSDKInitialized();
};
