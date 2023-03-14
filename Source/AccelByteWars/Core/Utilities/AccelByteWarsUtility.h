// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "IImageWrapper.h"

typedef TSharedPtr<const FSlateBrush> FCacheBrush;
DECLARE_DELEGATE_OneParam(FOnImageReceived, FCacheBrush);

class ACCELBYTEWARS_API AccelByteWarsUtility
{
public:
	static void GetImageFromURL(const FString& Url, const FString& ImageId, const FOnImageReceived& OnReceived);
	static FCacheBrush GetImageFromCache(const FString& ImageId);

private:
	static const TMap<FString, EImageFormat> ImageFormatMap;
};