// Copyright (c) 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef BLACKBOX_COMMON_H
#define BLACKBOX_COMMON_H
#pragma once

#ifdef __cplusplus
#    define EXTERN_C extern "C"
#else
#    define EXTERN_C
#endif // __cplusplus

#ifndef BLACKBOX_INTERFACE
#    ifdef _MSC_VER
#        if defined BLACKBOX_BUILD_SHARED_LIBRARY
#            define BLACKBOX_INTERFACE __declspec(dllexport)
#        elif defined BLACKBOX_USE_SHARED_LIBRARY
#            define BLACKBOX_INTERFACE __declspec(dllimport)
#        else
#            define BLACKBOX_INTERFACE
#        endif
#    elif __GNUC__ >= 4 || defined(__clang__)
#        define BLACKBOX_INTERFACE __attribute__((visibility("default")))
#    else
#        define BLACKBOX_INTERFACE
#    endif
#endif

#ifndef BLACKBOX_API
#    ifdef _MSC_VER
#        if defined BLACKBOX_BUILD_SHARED_LIBRARY
#            define BLACKBOX_API(rtype) __declspec(dllexport) rtype __cdecl
#        elif defined BLACKBOX_USE_SHARED_LIBRARY
#            define BLACKBOX_API(rtype) __declspec(dllimport) rtype __cdecl
#        else
#            define BLACKBOX_API(rtype) rtype
#        endif
#    elif __GNUC__ >= 4 || defined(__clang__)
#        define BLACKBOX_API(rtype) __attribute__((visibility("default"))) rtype __cdecl
#    else
#        define BLACKBOX_API(rtype) rtype
#    endif
#endif

// Platform definitions
#if !defined BLACKBOX_WINDOWS_PLATFORM
#    define BLACKBOX_WINDOWS_PLATFORM 0
#endif
#if !defined BLACKBOX_XBOX_ONE_PLATFORM
#    define BLACKBOX_XBOX_ONE_PLATFORM 0
#endif
#if !defined BLACKBOX_XBOXGDK_PLATFORM
#    define BLACKBOX_XBOXGDK_PLATFORM 0
#endif
#if !defined BLACKBOX_PS4_PLATFORM
#    define BLACKBOX_PS4_PLATFORM 0
#endif
#if !defined BLACKBOX_PS5_PLATFORM
#    define BLACKBOX_PS5_PLATFORM 0
#endif
#if !defined BLACKBOX_LINUX_PLATFORM
#    define BLACKBOX_LINUX_PLATFORM 0
#endif
#if !defined BLACKBOX_MAC_PLATFORM
#    define BLACKBOX_MAC_PLATFORM 0
#endif
#if !defined BLACKBOX_USE_UNITY_ENGINE
#    define BLACKBOX_USE_UNITY_ENGINE 0
#endif

#if defined(_WIN32) && !defined(_XBOX_ONE) && !defined(_GAMING_XBOX_SCARLETT) && !defined(_GAMING_XBOX_XBOXONE)
#    if defined BLACKBOX_WINDOWS_PLATFORM
#        undef BLACKBOX_WINDOWS_PLATFORM
#    endif
#    define BLACKBOX_WINDOWS_PLATFORM 1
#elif defined(_XBOX_ONE) && !defined(_GAMING_XBOX_SCARLETT) && !defined(_GAMING_XBOX_XBOXONE)
#    if defined BLACKBOX_XBOX_ONE_PLATFORM
#        undef BLACKBOX_XBOX_ONE_PLATFORM
#    endif
#    define BLACKBOX_XBOX_ONE_PLATFORM 1
#elif defined(_GAMING_XBOX_SCARLETT) || defined(_GAMING_XBOX_XBOXONE)
#    if defined BLACKBOX_XBOXGDK_PLATFORM
#        undef BLACKBOX_XBOXGDK_PLATFORM
#    endif
#    define BLACKBOX_XBOXGDK_PLATFORM 1
#elif defined(__ORBIS__)
#    if defined BLACKBOX_PS4_PLATFORM
#        undef BLACKBOX_PS4_PLATFORM
#    endif
#    define BLACKBOX_PS4_PLATFORM 1
#elif defined(__PROSPERO__)
#    if defined BLACKBOX_PS5_PLATFORM
#        undef BLACKBOX_PS5_PLATFORM
#    endif
#    define BLACKBOX_PS5_PLATFORM 1
#elif defined(__linux__)
#    if defined BLACKBOX_LINUX_PLATFORM
#        undef BLACKBOX_LINUX_PLATFORM
#    endif
#    define BLACKBOX_LINUX_PLATFORM 1
#elif defined(__APPLE__) // Strictly speaking this could be either iOS or macOS but for now we only support macOS. You
                         // can use TARGET_OS_IPHONE and TARGET_OS_EMBEDDED to distinguish
#    if defined BLACKBOX_MAC_PLATFORM
#        undef BLACKBOX_MAC_PLATFORM
#    endif
#    define BLACKBOX_MAC_PLATFORM 1
#endif

#if defined(__clang__)
#    define SUPRESS_WARNING_BEGIN _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Weverything\"")
#    define SUPRESS_WARNING_END _Pragma("clang diagnostic pop")
#elif defined(_MSC_VER)
#    define SUPRESS_WARNING_BEGIN __pragma(warning(push, 0))
#    define SUPRESS_WARNING_END __pragma(warning(pop))
#endif

#endif // BLACKBOX_COMMON_H