// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteWarsFStringUtils.h"

#undef UE_LOG_FUNC_CATEGORY

#if PLATFORM_WINDOWS
#define UE_LOG_FUNC(LogCategory, Verbosity, Format, ...) \
{ \
	UE_LOG(LogCategory, Verbosity, TEXT("%s %s"), __FUNCTIONW__, *FString::Printf(Format, ##__VA_ARGS__)); \
}
#else
#define UE_LOG_FUNC(LogCategory, Verbosity, Format, ...) \
{ \
	UE_LOG(LogCategory, Verbosity, TEXT("%s %s"), *FString(__FUNCTION__), *FString::Printf(Format, ##__VA_ARGS__)); \
}
#endif

// Gets the class, and function name
#define GET_CLASS_FUNC_NAME (FString(__FUNCTION__))

// Gets the current line of code
#define GET_LINE (FString::FromInt(__LINE__))

// Combines the class, function, and line
#define GET_CLASS_FUNC_LINE (GET_CLASS_FUNC_NAME + ":(" + GET_LINE + ")")

// Logs a string to the console along with where it was called from
#define LOG_TO_CONSOLE(Param1) UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *GET_CLASS_FUNC_LINE, *FString(Param1))

namespace
{
	// Logs a string tp the game window
	static void LogToScreen(const FString text, FColor color = FColor::Yellow)
	{
	#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT || WITH_EDITOR
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, color, text);
	#endif
	}

	// Logs a string to console
	static void LogToConsole(const FString text)
	{
		LOG_TO_CONSOLE(text);
	}

	// Outputs a string to the player screen and console
	static void LogToScreenAndConsole(const FString text, FColor color = FColor::Yellow)
	{
		LogToScreen(text, color);
		LogToConsole(text);
	}

	// Detects all network roles of any given actor
	static void LogActorNetworkIdentity(AActor* actor)
	{
		if (actor == nullptr)
			return;

		FString local_role = EnumToFString("ENetRole", actor->GetLocalRole());
		FString remote_role = EnumToFString("ENetRole", actor->GetRemoteRole());
		FString net_role = EnumToFString("ENetMode", actor->GetNetMode());

		bool is_dedicated_server = false;
		UWorld* const world = actor->GetWorld();
		if (world == nullptr)
			return;

		if (world->IsNetMode(NM_DedicatedServer))
			is_dedicated_server = true;

		LogToScreenAndConsole("IsDedicatedServer: " + BoolToFString(is_dedicated_server) + " Local: " + local_role + " Remote: " + remote_role + " Net: " + net_role);
	}
}
