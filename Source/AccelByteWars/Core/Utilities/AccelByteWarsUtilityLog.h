// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

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
