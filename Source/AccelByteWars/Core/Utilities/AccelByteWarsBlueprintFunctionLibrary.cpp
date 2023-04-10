// Fill out your copyright notice in the Description page of Project Settings.


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