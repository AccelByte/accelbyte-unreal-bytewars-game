// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "ReconnectingWidget.generated.h"

UCLASS()
class ACCELBYTEWARS_API UReconnectingWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
};