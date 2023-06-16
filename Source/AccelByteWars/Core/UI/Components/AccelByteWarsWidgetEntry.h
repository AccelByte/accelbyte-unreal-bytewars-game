// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserListEntry.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "CommonUserWidget.h"
#include "AccelByteWarsWidgetEntry.generated.h"

UCLASS(Abstract)
class ACCELBYTEWARS_API UAccelByteWarsWidgetEntry : public UCommonUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
};