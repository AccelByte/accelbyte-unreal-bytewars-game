// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "BlackBoxCommon.h"
#include "BlackBoxSettings.h"
#include "CoreMinimal.h"

class BlackBoxIssueReporter {
    BlackBoxIssueReporter();

private:
#if BLACKBOX_UE_WINDOWS
    static const inline TMap<UsableKeysE, FKey> WindowsKeyMapping = {
        {UsableKeysE::F1, EKeys::F1},
        {UsableKeysE::F2, EKeys::F2},
        {UsableKeysE::F3, EKeys::F3},
        {UsableKeysE::F4, EKeys::F4},
        {UsableKeysE::F5, EKeys::F5},
        {UsableKeysE::F6, EKeys::F6},
        {UsableKeysE::F7, EKeys::F7},
        {UsableKeysE::F8, EKeys::F8},
        {UsableKeysE::F9, EKeys::F9},
        {UsableKeysE::F10, EKeys::F10},
        {UsableKeysE::F11, EKeys::F11},
        {UsableKeysE::F12, EKeys::F12},
        {UsableKeysE::Unknown, EKeys::Invalid}};
#endif

public:
    static int GetIssueReporterHotkey();

    static FKey ConvertStringToFKey(FString KeyString);
    static UsableKeysE ConvertStringToEnumKey(FString KeyString);
    static FString ConvertEnumKeyToString(UsableKeysE EnumKey);

    static FKey ConvertEnumKeyToFKey(UsableKeysE EnumKey);

    static int TakeStandardScreenshotWithUE(FString IssueFolder);
};