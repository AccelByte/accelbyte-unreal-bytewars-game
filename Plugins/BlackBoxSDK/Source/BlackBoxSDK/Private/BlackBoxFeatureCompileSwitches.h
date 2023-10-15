// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

// The following preprocessor definition must be set from BlackBoxSDK.Build.cs.
// Don't modify them directly.

// Define this as 1 to disable everything except
// the most minimal crash report functionality
#ifdef BLACKBOX_SDK_MINIMAL_CRASH_REPORT_ONLY
#    if BLACKBOX_SDK_MINIMAL_CRASH_REPORT_ONLY
#        ifndef BLACKBOX_SDK_CRASH_REPORT_ONLY
#            define BLACKBOX_SDK_CRASH_REPORT_ONLY 1
#        endif

#        ifndef BLACKBOX_SDK_NO_CRASH_VIDEO
#            define BLACKBOX_SDK_NO_CRASH_VIDEO 1
#        endif

#        ifndef BLACKBOX_SDK_NO_ADDITIONAL_KEY_VALUE_DATA_GATHERING
#            define BLACKBOX_SDK_NO_ADDITIONAL_KEY_VALUE_DATA_GATHERING 1
#        endif
#    endif
#endif

// Define this as 1 to disable everything except
// the crash report functionality, crash report will still have its video recording
// and you'll still  able to put additional key value data into the crash report.
#ifdef BLACKBOX_SDK_CRASH_REPORT_ONLY
#    if BLACKBOX_SDK_CRASH_REPORT_ONLY
#        ifndef BLACKBOX_SDK_NO_PROFILING
#            define BLACKBOX_SDK_NO_PROFILING 1
#        endif

#        ifndef BLACKBOX_SDK_NO_ISSUE_REPORTER
#            define BLACKBOX_SDK_NO_ISSUE_REPORTER 1
#        endif

#        ifndef BLACKBOX_SDK_NO_EXTERNAL_USER_ID_SUPPORT
#            define BLACKBOX_SDK_NO_EXTERNAL_USER_ID_SUPPORT 1
#        endif

#        ifndef BLACKBOX_SDK_NO_ADT_SESSION_GROUPING
#            define BLACKBOX_SDK_NO_ADT_SESSION_GROUPING 1
#        endif

#        ifndef BLACKBOX_SDK_NO_LOG_STREAMING
#            define BLACKBOX_SDK_NO_LOG_STREAMING 1
#        endif

#        ifndef BLACKBOX_SDK_NO_DEVICE_INFORMATION_GATHERING
#            define BLACKBOX_SDK_NO_DEVICE_INFORMATION_GATHERING 1
#        endif
#    endif
#endif

// Disable the crash report
// Disabling crash report implies disabling crash video too
#ifdef BLACKBOX_SDK_NO_CRASH_REPORT
#    if BLACKBOX_SDK_NO_CRASH_REPORT
#        ifndef BLACKBOX_SDK_NO_CRASH_VIDEO
#            define BLACKBOX_SDK_NO_CRASH_VIDEO 1
#        else
#            undef BLACKBOX_SDK_NO_CRASH_VIDEO
#            define BLACKBOX_SDK_NO_CRASH_VIDEO 1
#        endif
#    endif
#endif

// Disable additional key value data gathering in the crash report
// Calling UpdateAdditionalInfo() and GetAdditionalInfoValue() will not have any effect if this active
#ifndef BLACKBOX_SDK_NO_ADDITIONAL_KEY_VALUE_DATA_GATHERING
#    define BLACKBOX_SDK_NO_ADDITIONAL_KEY_VALUE_DATA_GATHERING 0
#endif

// Disable crash video in the crash report
#ifndef BLACKBOX_SDK_NO_CRASH_VIDEO
#    define BLACKBOX_SDK_NO_CRASH_VIDEO 0
#endif

// Disable the crash report
#ifndef BLACKBOX_SDK_NO_CRASH_REPORT
#    define BLACKBOX_SDK_NO_CRASH_REPORT 0
#endif

// Disable Blackbox session creation, any functionality that rely on this won't work if this is active
// such as basic profiling, playtest, and session grouping
#ifndef BLACKBOX_SDK_NO_SESSION_CREATION
#    define BLACKBOX_SDK_NO_SESSION_CREATION 0
#endif

// Disable basic profiling functionality
#ifndef BLACKBOX_SDK_NO_PROFILING
#    define BLACKBOX_SDK_NO_PROFILING 0
#endif

// Disable issue reporter. If this is active, trying to activate issue reporter will not have any effect.
#ifndef BLACKBOX_SDK_NO_ISSUE_REPORTER
#    define BLACKBOX_SDK_NO_ISSUE_REPORTER 0
#endif

// Disable session grouping. If this is active, calling CreateMatch(), BeginMatchSession(), and EndMatchSession()
// will not have any effect.
#ifndef BLACKBOX_SDK_NO_ADT_SESSION_GROUPING
#    define BLACKBOX_SDK_NO_ADT_SESSION_GROUPING 0
#endif

// Disable cross referencing with external user id. If this is active, calling UpdateSessionWithExternalUserID()
// and UpdateSessionWithExternalSessionID() will not have any effect.
#ifndef BLACKBOX_SDK_NO_EXTERNAL_USER_ID_SUPPORT
#    define BLACKBOX_SDK_NO_EXTERNAL_USER_ID_SUPPORT 0
#endif

// Disable log streaming functionality
#ifndef BLACKBOX_SDK_NO_LOG_STREAMING
#    define BLACKBOX_SDK_NO_LOG_STREAMING 0
#endif

// Disable hardware information gathering
#ifndef BLACKBOX_SDK_NO_DEVICE_INFORMATION_GATHERING
#    define BLACKBOX_SDK_NO_DEVICE_INFORMATION_GATHERING 0
#endif

#ifndef BLACKBOX_SDK_CRASH_REPORT_ONLY
#    define BLACKBOX_SDK_CRASH_REPORT_ONLY 0
#endif

#ifndef BLACKBOX_SDK_MINIMAL_CRASH_REPORT_ONLY
#    define BLACKBOX_SDK_MINIMAL_CRASH_REPORT_ONLY 0
#endif

#define BLACKBOX_SDK_USE_CRASH_VIDEO !BLACKBOX_SDK_NO_CRASH_VIDEO

#define BLACKBOX_SDK_USE_ADDITIONAL_KEY_VALUE_DATA_GATHERING !BLACKBOX_SDK_NO_ADDITIONAL_KEY_VALUE_DATA_GATHERING

#define BLACKBOX_SDK_USE_CRASH_REPORT !BLACKBOX_SDK_NO_CRASH_REPORT

#define BLACKBOX_SDK_USE_SESSION_CREATION !BLACKBOX_SDK_NO_SESSION_CREATION

#define BLACKBOX_SDK_USE_PROFILING !BLACKBOX_SDK_NO_PROFILING

#define BLACKBOX_SDK_USE_ISSUE_REPORTER !BLACKBOX_SDK_NO_ISSUE_REPORTER

#define BLACKBOX_SDK_USE_ADT_SESSION_GROUPING !BLACKBOX_SDK_NO_ADT_SESSION_GROUPING

#define BLACKBOX_SDK_USE_EXTERNAL_USER_ID_SUPPORT !BLACKBOX_SDK_NO_EXTERNAL_USER_ID_SUPPORT

#define BLACKBOX_SDK_USE_LOG_STREAMING !BLACKBOX_SDK_NO_LOG_STREAMING

#define BLACKBOX_SDK_USE_DEVICE_INFORMATION_GATHERING !BLACKBOX_SDK_NO_DEVICE_INFORMATION_GATHERING
