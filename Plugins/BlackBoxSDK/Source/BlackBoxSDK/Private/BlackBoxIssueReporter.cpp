// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "BlackBoxIssueReporter.h"

#include "BlackBoxLog.h"
#include "CoreMinimal.h"
#include "UnrealClient.h"
#include "Misc/DateTime.h"
#include "Core/accelbyte/cpp/blackbox.h"

int BlackBoxIssueReporter::GetIssueReporterHotkey()
{
#if BLACKBOX_UE_WINDOWS
    FString StringVal = UTF8_TO_TCHAR(bbx_config_get_issue_reporter_hotkey());
    FKey Hotkey = ConvertStringToFKey(StringVal);
    const uint32* KeyCodePtr;
    const uint32* CharCodePtr;
    FInputKeyManager::Get().GetCodesFromKey(Hotkey, KeyCodePtr, CharCodePtr);
    if (KeyCodePtr == nullptr) {
        return 0;
    }
    return *KeyCodePtr;
#endif
    return 0;
}

FKey BlackBoxIssueReporter::ConvertStringToFKey(FString KeyString)
{
#if BLACKBOX_UE_WINDOWS
    FName HotkeyName = FName(*KeyString, EFindName::FNAME_Find);
    return FKey(HotkeyName);
#endif
    return EKeys::Invalid;
}

UsableKeysE BlackBoxIssueReporter::ConvertStringToEnumKey(FString KeyString)
{
#if BLACKBOX_UE_WINDOWS
    auto Hotkey = ConvertStringToFKey(KeyString);
    return *WindowsKeyMapping.FindKey(Hotkey);
#endif
    return UsableKeysE::Unknown;
}

FString BlackBoxIssueReporter::ConvertEnumKeyToString(UsableKeysE EnumKey)
{
#if BLACKBOX_UE_WINDOWS
    auto Hotkey = WindowsKeyMapping.FindRef(EnumKey);
    return Hotkey.GetFName().ToString();
#endif
    return EKeys::Invalid.GetFName().ToString();
}

FKey BlackBoxIssueReporter::ConvertEnumKeyToFKey(UsableKeysE EnumKey)
{
#if BLACKBOX_UE_WINDOWS
    return WindowsKeyMapping.FindRef(EnumKey);
#endif
    return EKeys::Invalid;
}

FString GetTimeNowIso8601UtcAlphanumOnly()
{
    FDateTime DateTimeNow = FDateTime::UtcNow();
    FString TimeString = DateTimeNow.ToString(TEXT("%Y%m%dT%H%M%SZ"));
    return TimeString;
}

int BlackBoxIssueReporter::TakeStandardScreenshotWithUE(FString IssueFolder)
{
    try {
        FString ImageFilename = "screenshot_" + GetTimeNowIso8601UtcAlphanumOnly() + ".png";
        FString ImageFilepath = FPaths::Combine(IssueFolder, TEXT("/"), ImageFilename);
        FScreenshotRequest::RequestScreenshot(ImageFilepath, true, false);
    }
    catch (const std::exception& e) {
        UE_LOG(LogBlackBox, Error, TEXT("An exception occurred: %s"), UTF8_TO_TCHAR(e.what()));
        return 1;
    }
    return 0;
}
