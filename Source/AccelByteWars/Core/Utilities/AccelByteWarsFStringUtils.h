// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

static FString UInt32ToFString(uint32 input)
{
	return FString::Printf(TEXT("%d"), input);
}

static FString Int32ToFString(int32 input)
{
	return FString::Printf(TEXT("%i"), input);
}

static FString FloatToFString(float input)
{
	return FString::Printf(TEXT("%f"), input);
}

static FString BoolToFString(bool input)
{
	return input ? "True" : "False";
}

static const TCHAR* FStringToTCHAR(const FString& input)
{
	return *input;
}

static FString FNameToFString(FName input)
{
	return input.ToString();
}

static FString FTextToFString(FText input)
{
	return input.ToString();
}

static FString FVector2DToFString(FVector2D input)
{
	return input.ToString();
}

static FString FVectorToFString(FVector input)
{
	return input.ToString();
}

static FString FRotatorToFString(FRotator input)
{
	return input.ToString();
}

static FString FLinearColorToFString(FLinearColor input)
{
	return "R: " + FloatToFString(input.R) + " G: " + FloatToFString(input.G) + " B: " + FloatToFString(input.B) + " A: " + FloatToFString(input.A);
}

template<typename T>
static FString EnumToFString(const FString& enumTypeName, const T enumValue)
{
	UEnum* pEnum = FindFirstObjectSafe<UEnum>(*enumTypeName);
	return *(pEnum ? pEnum->GetNameStringByIndex(static_cast<uint8>(enumValue)) : "null");
}