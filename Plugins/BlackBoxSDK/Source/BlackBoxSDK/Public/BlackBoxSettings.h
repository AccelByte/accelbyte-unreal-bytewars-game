// Copyright (c) 2019 - 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Misc/MessageDialog.h"
#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "BlackBoxLogSeverity.h"
#include "BlackBoxSettings.generated.h"

UENUM()
enum class BlackBoxConfigValE : uint8 {
    Off UMETA(DisplayName = "Off"),
    On UMETA(DisplayName = "On"),
    WebConfig UMETA(DisplayName = "Web Config")
};

struct BlackBoxConsoleCommand {
    static FString ConvertConfigEnumToString(const BlackBoxConfigValE& value);
    static BlackBoxConfigValE ConvertConfigStringToEnum(const FString& value);

    FString Name{};
    FString Help{};
    uint8 SettingVar;
    bool NeedRestartOnChange{};
};

UENUM()
enum class UsableKeysE : uint8 {
    F1 UMETA(DisplayName = "F1"),
    F2 UMETA(DisplayName = "F2"),
    F3 UMETA(DisplayName = "F3"),
    F4 UMETA(DisplayName = "F4"),
    F5 UMETA(DisplayName = "F5"),
    F6 UMETA(DisplayName = "F6"),
    F7 UMETA(DisplayName = "F7"),
    F8 UMETA(DisplayName = "F8"),
    F9 UMETA(DisplayName = "F9"),
    F10 UMETA(DisplayName = "F10"),
    F11 UMETA(DisplayName = "F11"),
    F12 UMETA(DisplayName = "F12"),
    Unknown UMETA(Hidden)
};

/**
 * @brief UObject for storing settings into configuration file.
 */
UCLASS(Config = Engine)
class BLACKBOXSDK_API UBlackBoxSettings : public UObject {
    GENERATED_BODY()
public:
    UBlackBoxSettings();

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    FString APIKey{};

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    FString GameVersionID{};

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    FString Namespace{};

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    BlackBoxLogSeverity LogSeverity = BlackBoxLogSeverity::INFO;

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Settings")
    bool EnableLog{};

    UPROPERTY(EditAnywhere, Category = "Local Overrides")
    BlackBoxConfigValE EnableBasicProfiling = BlackBoxConfigValE::Off;

    UPROPERTY(EditAnywhere, Category = "Local Overrides")
    BlackBoxConfigValE EnableCrashReporter = BlackBoxConfigValE::On;

    UPROPERTY(EditAnywhere, Category = "Local Overrides")
    BlackBoxConfigValE EnableHardwareInformationGathering = BlackBoxConfigValE::On;

    UPROPERTY(EditAnywhere, Category = "Local Overrides")
    BlackBoxConfigValE EnableStoreCrashVideo = BlackBoxConfigValE::On;

    UPROPERTY(GlobalConfig)
    bool HideIssueReporterSettings = false;

    UPROPERTY(
        EditAnywhere,
        GlobalConfig,
        Category = "Issue Reporter",
        Meta = (EditCondition = "!HideIssueReporterSettings", EditConditionHides, HideEditConditionToggle))
    bool EnableIssueReporter = false;

    UPROPERTY(
        EditAnywhere,
        GlobalConfig,
        Category = "Issue Reporter",
        Meta = (EditCondition = "!HideIssueReporterSettings", EditConditionHides, HideEditConditionToggle))
    UsableKeysE IssueReporterHotkey = UsableKeysE::F8;

    bool UseUnrealToCaptureScreenshot = false;

    UPROPERTY(EditAnywhere, GlobalConfig, Category = "Experimental")
    bool ExperimentalServerBuildIdFeature = false;

    // <Property Name, Initial Value>
    TMap<FString, int> NeedToRestartOnChangedProperties{};

    TArray<BlackBoxConsoleCommand> ConsoleCommands{};

    void InitializeLocalConfigProperties();

    void InitializeNeedToRestartOnChangeProperties();

    bool CheckNeedToRestart(const FString& ChangedPropertiesName);

    void ApplyLocalConfigProperties();

    void ShowMustRestartDialog();

#if defined(WITH_EDITOR) && WITH_EDITOR
#    if ((ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION < 25))
    virtual bool CanEditChange(const UProperty* InProperty) const override;
#    else
    virtual bool CanEditChange(const FProperty* InProperty) const override;
#    endif
#endif
private:
    void CreateConsoleCommand(BlackBoxConsoleCommand& NewConsoleCommand);
};