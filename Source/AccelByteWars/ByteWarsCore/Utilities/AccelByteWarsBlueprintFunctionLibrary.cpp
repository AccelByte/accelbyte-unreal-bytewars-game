// Fill out your copyright notice in the Description page of Project Settings.


#include "ByteWarsCore/Utilities/AccelByteWarsBlueprintFunctionLibrary.h"

EBPNetMode UAccelByteWarsBlueprintFunctionLibrary::GetNetMode(AActor* Actor)
{
	return static_cast<EBPNetMode>(Actor->GetNetMode());
}
