// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonListView.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "AccelByteWarsListView.generated.h"

UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsListView : public UCommonListView
{
	GENERATED_BODY()

public:
	void SetEntryWidgetClass(TSubclassOf<UAccelByteWarsWidgetEntry>& InEntryWidgetClass);
};