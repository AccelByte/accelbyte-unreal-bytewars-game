// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AccelByteWarsGameplayObjectComponent.generated.h"

UENUM(BlueprintType)
enum class EGameplayObjectType : uint8
{
	SHIP,
	PLANET,
	STAR
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACCELBYTEWARS_API UAccelByteWarsGameplayObjectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Mass = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Radius = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EGameplayObjectType ObjectType = EGameplayObjectType::PLANET;
};
