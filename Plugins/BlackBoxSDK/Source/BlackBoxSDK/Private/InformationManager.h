// Copyright (c) 2020-2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "BlackBoxCommon.h"
#include "CoreMinimal.h"
#include "InputCoreTypes.h"

#include <string>

class APlayerController;

struct CPUInformation {
    FString Model;
    uint64 Frequency;
};

struct GPUInformation {
    FString Model;
    uint64 Frequency;
    uint64 MemoryAmount;
    uint64 MemoryFrequency;
    FString DriverVer;
};

struct OSInformation {
    FString Name;
    FString Version;
    FString Architecture;
    FString RendererApi;
    FString Locale;
    FString Country;
    FString UserName;
    FString ComputerName;
};

struct SDKInformation {
    FString BaseUrl;
    FString IamUrl;
    FString DownloadURL;
    FString LatestReleaseURL;
    FString BuildId;
    FString GameVersionId;
    FString ProjectId;
    FString Namespace;
    FString APIKey;
    FString CoreSDKConfigPath;
    FKey IssueReporterHotkey;
    bool IsEnable;
    bool EnableIssueReporter;
    bool IsRunningAsServer;
    bool UseUnrealToCaptureScreenshot;
};

struct ConfigInformation {
    uint32 FPS;
    uint32 KPS;
    uint32 TotalRecordingSecond;
    FString SubtitleType;
    bool EnableCrashReporter;
    bool StoreDXDiag;
    bool StoreCrashVideo;
    bool EnableBasicProfiling;
    bool EnableCPUProfiling;
    bool EnableGPUProfiling;
    bool EnableMemoryProfiling;
};

struct ProcessInformation {
    uint32 PID;
    FString BlackBoxHelperPath;
    FString BlackBoxHelperLogAlternativePath;
    FString BlackBoxIssueReporterPath;
    FString CrashFolder;
    FString CrashGUID;
    FString LogSourceFilePath;
};

struct UserInformation {
    FString IAMUserId;
    FString PlaytestId;
    FString DeviceId;
};

struct InputInformation {
    TArray<TPair<FString, FKey>> ActionKeyPair;
};

class FInfoManager {
public:
    FInfoManager();
    ~FInfoManager();

    InputInformation& GetKeyInformation();
    void SetupKeyInformation(APlayerController* PlayerCtrl);
    bool IsKeyInformationPresent();
    void ResetKeyInformation();

    CPUInformation& GetCPUInformation();
    void SetCPUInformation(const CPUInformation& CPUInfo_);

    GPUInformation& GetGPUInformation();
    void SetGPUInformation(const GPUInformation& GPUInfo_);

    OSInformation& GetOSInformation();
    void SetOSInformation(const OSInformation& OSInfo_);

    ConfigInformation GetConfigInformation();

    SDKInformation GetSDKInformation();
    void SetSDKInformation(const SDKInformation& SDKInfo);

    ProcessInformation& GetProcessInformation();
    void SetProcessInformation(const ProcessInformation& ProcInfo_);

    UserInformation& GetUserInformation();
    void SetUserInformation(const UserInformation& UserInfo_);

#if BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX
    static std::string GetGamertag();
#endif

private:
    void UpdateBlackBoxConfiguration(const SDKInformation& SDKInfo_);
    void UpdateBlackBoxClientInformation(const ProcessInformation& ProcInfo_);
    bool IsOSInfoEmpty() const;

private:
    CPUInformation CPUInfo;
    GPUInformation GPUInfo;
    OSInformation OSInfo;
    ProcessInformation ProcInfo;
    InputInformation InputInfo;
    UserInformation UserInfo;
};
