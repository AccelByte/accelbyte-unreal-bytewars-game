// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "AccelByteWarsGameplayEffect.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ACCELBYTEWARS_API UAccelByteWarsGameplayEffect_LivesUp : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UAccelByteWarsGameplayEffect_LivesUp(const FObjectInitializer& ObjectInitializer);
};

UCLASS(BlueprintType, Blueprintable)
class ACCELBYTEWARS_API UAccelByteWarsGameplayEffect_ScoreMultiplier : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UAccelByteWarsGameplayEffect_ScoreMultiplier(const FObjectInitializer& ObjectInitializer);
};

UCLASS(BlueprintType, Blueprintable)
class ACCELBYTEWARS_API UAccelByteWarsGameplayEffect_ScoreAddition : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UAccelByteWarsGameplayEffect_ScoreAddition(const FObjectInitializer& ObjectInitializer);
};

UCLASS(BlueprintType, Blueprintable)
class ACCELBYTEWARS_API UAccelByteWarsGameplayEffect_MissileSizeMultiplier : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UAccelByteWarsGameplayEffect_MissileSizeMultiplier(const FObjectInitializer& ObjectInitializer);
};