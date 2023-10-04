// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/Utilities/AccelByteWarsBlueprintFunctionLibrary.h"

#include "Kismet/KismetMathLibrary.h"

EBPNetMode UAccelByteWarsBlueprintFunctionLibrary::GetNetMode(AActor* Actor)
{
	return static_cast<EBPNetMode>(Actor->GetNetMode());
}

bool UAccelByteWarsBlueprintFunctionLibrary::IsUniqueNetIdValid(const FUniqueNetIdRepl UniqueNetId)
{
	return UniqueNetId.IsValid();
}