// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonTileView.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "AccelByteWarsTileView.generated.h"

class UAccelByteWarsWidgetEntry;

UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsTileView : public UCommonTileView
{
	GENERATED_BODY()

public:
	void SetEntryWidgetClass(TSubclassOf<UAccelByteWarsWidgetEntry>& InEntryWidgetClass);
};