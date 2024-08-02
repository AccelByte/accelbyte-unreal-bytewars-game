// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CommonVideoPlayer.h"
#include "AccelByteWarsVideoPlayer.generated.h"

UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsVideoPlayer : public UCommonVideoPlayer
{
	GENERATED_BODY()

	virtual void HandleMediaPlayerEvent(EMediaEvent EventType) override;
};
