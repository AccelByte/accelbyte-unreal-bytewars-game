// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/UI/AccelByteWarsActivatableWidget.h"
#include "LoginWidget.generated.h"

UENUM(BlueprintType)
enum ELoginState 
{
	Default,
	InProgress,
	Failed
};

UCLASS()
class ACCELBYTEWARS_API ULoginWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
};