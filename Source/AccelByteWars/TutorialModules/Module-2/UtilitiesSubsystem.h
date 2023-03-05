// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AuthEssentialsLog.h"
#include "IImageWrapper.h"
#include "UtilitiesSubsystem.generated.h"

typedef TSharedPtr<const FSlateBrush> FCacheBrush;
DECLARE_DELEGATE_OneParam(FOnImageReceived, FCacheBrush);

UCLASS()
class ACCELBYTEWARS_API UUtilitiesSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	bool IsAccelByteSDKInitialized();

	static void GetImageFromURL(const FString& Url, const FString& ImageId, const FOnImageReceived& OnReceived);
	static FCacheBrush GetImageFromCache(const FString& ImageId);

	static TMap<FString, EImageFormat> ImageFormatMap;
};