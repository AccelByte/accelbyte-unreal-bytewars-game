// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogManagingFriends, Log, All);

#define UE_LOG_MANAGING_FRIENDS(Verbosity, Format, ...) \
{ \
	UE_LOG(LogManagingFriends, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}
