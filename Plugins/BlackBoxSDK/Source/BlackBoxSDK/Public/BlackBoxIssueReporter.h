// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "BlackBoxCommon.h"
#include "BlackBoxSettings.h"
#include "CoreMinimal.h"
#include "Framework/Commands/InputChord.h"

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
    static FInputChord GetIssueReporterHotkey();

    static FKey ConvertStringToFKey(FString KeyString);
    static UsableKeysE ConvertStringToEnumKey(FString KeyString);
    static FString ConvertEnumKeyToString(UsableKeysE EnumKey);

    static FKey ConvertEnumKeyToFKey(UsableKeysE EnumKey);

    /**
     * Serialize an FInputChord to a string.
     *
     * @param InputChord The FInputChord to be serialized.
     * @return A serialized string representation of the FInputChord.
     */
    static FString SerializeInputChord(const FInputChord& InputChord);

    /**
     * Deserialize a string to an FInputChord.
     *
     * @param SerializedString The serialized string representation of an FInputChord.
     * @return The deserialized FInputChord.
     */
    static FInputChord DeserializeInputChord(const FString& SerializedString);

    /**
     * Check if the provided FInputChord combination is already used as a mappings in Unreal Engine.
     *
     * @param ChordToCheck The FInputChord combination to check.
     * @return true if the ChordToCheck is already in use, false otherwise.
     */
    static bool IsChordUsedAsMappings(const FInputChord& ChordToCheck);

    static const uint32 GetKeyCodes(const FKey& InputKey);

    static int TakeStandardScreenshotWithUE(FString IssueFolder);
};