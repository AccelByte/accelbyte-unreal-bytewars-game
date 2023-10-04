// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AccelByteWarsBlueprintFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum class EBPNetMode : uint8
{
	Standalone = 0,
	DedicatedServer,
	ListenServer,
	Client,
	MAX,
};

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UAccelByteWarsBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * @brief Get Actor's Net mode
	 * @param Actor Target Actor
	 * @return NetMode
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByteWars Utilities", meta = (ExpandEnumAsExecs = "ReturnValue", DefaultToSelf = "Actor"))
	static EBPNetMode GetNetMode(AActor* Actor);

	/**
	 * @brief Get whether unique net id valid or not
	 * @param UniqueNetId Target unique net id
	 * @return whether the provided unique net id valid or not
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByteWars Utilities", BlueprintPure)
	static bool IsUniqueNetIdValid(const FUniqueNetIdRepl UniqueNetId);
};
