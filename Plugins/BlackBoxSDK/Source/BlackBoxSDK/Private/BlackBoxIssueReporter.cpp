// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "BlackBoxIssueReporter.h"

#include "BlackBoxLog.h"
#include "CoreMinimal.h"
#include "UnrealClient.h"
#include "Misc/DateTime.h"
#include "Framework/Commands/InputChord.h"
#include "GameFramework/InputSettings.h"
#include "Core/accelbyte/cpp/blackbox.h"

FInputChord BlackBoxIssueReporter::GetIssueReporterHotkey()
{
    FInputChord Hotkey;

#if BLACKBOX_UE_WINDOWS
    FString SerializedHotkey = UTF8_TO_TCHAR(bbx_config_get_issue_reporter_hotkey());
    Hotkey = DeserializeInputChord(SerializedHotkey);
#endif

    return Hotkey;
}

const uint32 BlackBoxIssueReporter::GetKeyCodes(const FKey& InputKey)
{
    const uint32* KeyCodePtr;
    const uint32* CharCodePtr;
    FInputKeyManager::Get().GetCodesFromKey(InputKey, KeyCodePtr, CharCodePtr);

    if (KeyCodePtr != nullptr) {
        return *KeyCodePtr;
    }
    else if (CharCodePtr != nullptr) {
        return *CharCodePtr;
    }
    UE_LOG(
        LogBlackBox,
        Warning,
        TEXT("Failed to get Issue Reporter Hotkey Codes."));
    return {};
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

FString BlackBoxIssueReporter::SerializeInputChord(const FInputChord& InputChord)
{
    if (!InputChord.IsValidChord()) {
        UE_LOG(LogBlackBox, Warning, TEXT("Invalid key in FInputChord during serialization."));
        return FString();
    }

    FString SerializedString = FString::Printf(
        TEXT("Key=%s,bShift=%d,bCtrl=%d,bAlt=%d,bCmd=%d"),
        *InputChord.Key.ToString(),
        InputChord.bShift ? 1 : 0,
        InputChord.bCtrl ? 1 : 0,
        InputChord.bAlt ? 1 : 0,
        InputChord.bCmd ? 1 : 0);

    return SerializedString;
}

FInputChord BlackBoxIssueReporter::DeserializeInputChord(const FString& SerializedString)
{
    FInputChord InputChord;

    TArray<FString> Tokens;
    SerializedString.ParseIntoArray(Tokens, TEXT(","), true);

    for (const FString& Token : Tokens) {
        FString Key, Value;
        if (Token.Split(TEXT("="), &Key, &Value)) {
            if (Key == "Key") {
                InputChord.Key = FKey(*Value);
            }
            else if (Key == "bCtrl") {
                InputChord.bCtrl = FCString::Atoi(*Value) != 0;
            }
            else if (Key == "bShift") {
                InputChord.bShift = FCString::Atoi(*Value) != 0;
            }
            else if (Key == "bAlt") {
                InputChord.bAlt = FCString::Atoi(*Value) != 0;
            }
            else if (Key == "bCmd") {
                InputChord.bCmd = FCString::Atoi(*Value) != 0;
            }
        }
    }

    if (InputChord.IsValidChord()) {
        return InputChord;
    }

    UE_LOG(LogBlackBox, Warning, TEXT("Failed to deserialize FInputChord: Invalid format."));
    return FInputChord();
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
        FString ImageFilename = "image-" + GetTimeNowIso8601UtcAlphanumOnly() + ".png";
        FString ImageFilepath = FPaths::Combine(IssueFolder, TEXT("/"), ImageFilename);
        FScreenshotRequest::RequestScreenshot(ImageFilepath, true, false);
    }
    catch (const std::exception& e) {
        UE_LOG(LogBlackBox, Error, TEXT("An exception occurred: %s"), UTF8_TO_TCHAR(e.what()));
        return 1;
    }
    return 0;
}

bool BlackBoxIssueReporter::IsChordUsedAsMappings(const FInputChord& ChordToCheck)
{
    UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();

    if (InputSettings) {
        for (const FInputActionKeyMapping& ActionMapping : InputSettings->GetActionMappings()) {
            if (ActionMapping.Key == ChordToCheck.Key && ActionMapping.bShift == ChordToCheck.bShift &&
                ActionMapping.bCtrl == ChordToCheck.bCtrl && ActionMapping.bAlt == ChordToCheck.bAlt &&
                ActionMapping.bCmd == ChordToCheck.bCmd) {
                return true;
            }
        }

        for (const FInputAxisKeyMapping& AxisMapping : InputSettings->GetAxisMappings()) {
            if (AxisMapping.Key == ChordToCheck.Key && 0 == ChordToCheck.bShift && 0 == ChordToCheck.bCtrl &&
                0 == ChordToCheck.bAlt && 0 == ChordToCheck.bCmd) {
                return true;
            }
        }
    }

    return false;
}