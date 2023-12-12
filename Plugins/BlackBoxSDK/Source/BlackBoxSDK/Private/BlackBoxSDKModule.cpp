// Copyright (c) 2019 - 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "BlackBoxSDKModule.h"

#include "BlackBoxLog.h"
#include "BlackBoxSettings.h"
#if WITH_EDITOR
#    include "BlackBoxSettingsCustomization.h"
#endif
#include "BlackBoxBackbufferManager.h"
#include "BlackBoxCommon.h"
#include "BlackBoxIssueReporter.h"
#include "BlackBoxTraceWriter.h"
#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "CrashHandler/BlackBoxCrashHandler.h"
#include "Endpoints.h"
#include "Engine.h"
#include "Engine/GameEngine.h"
#include "Engine/World.h"
#include "EngineGlobals.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformOutputDevices.h"
#include "HAL/PlatformProcess.h"
#include "InformationManager.h"
#include "Misc/Compression.h"
#include "Misc/CoreDelegates.h"
#include "Misc/EngineVersion.h"
#include "Misc/Paths.h"
#if WITH_EDITOR
#    include "PropertyEditorModule.h"
#endif

#if BLACKBOX_UE_WINDOWS
#    include "Windows/WindowsPlatformCrashContext.h"
#    include "Windows/WindowsPlatformProcess.h"
#elif BLACKBOX_UE_XBOXONE
#    if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION < 26)
#        include "XboxOne/XboxOnePlatformCrashContext.h"
#        include "XboxOne/XboxOnePlatformProcess.h"
#    else
#        include "XboxOnePlatformCrashContext.h"
#        include "XboxOnePlatformProcess.h"
#    endif
#elif BLACKBOX_UE_XBOXONEGDK
#    include "XboxCommonPlatformCrashContext.h"
#    if ENGINE_MAJOR_VERSION == 4
#        include "XboxOneGDKPlatformProcess.h"
#    else
#        include "XboxCommonPlatformProcess.h"
#    endif
#elif BLACKBOX_UE_XSX
#    include "XSXPlatformProcess.h"
#    include "XboxCommonPlatformCrashContext.h"
#elif BLACKBOX_UE_PS4
#    include "PS4PlatformProcess.h"
#elif BLACKBOX_UE_PS5
#    include "PS5PlatformProcess.h"
#elif BLACKBOX_UE_LINUX
#    include "Unix/UnixPlatformCrashContext.h"
#    include "Unix/UnixPlatformProcess.h"
#elif BLACKBOX_UE_MAC
#    include "Mac/MacPlatformProcess.h"
#endif

#include "BlackBoxUnrealHttp.h"
#include "Core/accelbyte/cpp/blackbox.h"
#include "Core/accelbyte/cpp/blackbox_http.h"
#include "Core/accelbyte/cpp/utils/error_codes.h"

#if WITH_EDITOR
#    include "ISettingsModule.h"
#    include "ISettingsSection.h"
#endif

#if BLACKBOX_UE_WINDOWS
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include "Windows/WindowsHWrapper.h"
// Disable macro redefinition warning for compatibility with Windows SDK 8+
#    pragma warning(push)
#    pragma warning(disable : 4005) // macro redefinition

#    if PLATFORM_64BITS
#        pragma pack(push, 16)
#    else
#        pragma pack(push, 8)
#    endif

#    undef DrawText
#    pragma pack(pop)
#    pragma warning(pop)
#    undef GetEnvironmentVariable
#endif

#include "BlackBoxFeatureCompileSwitches.h"

// STL
#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>

/** How many cycles the renderthread used (excluding idle time). It's set once per frame in FViewport::Draw. */
extern RENDERCORE_API uint32 GRenderThreadTime;
/** How many cycles the gamethread used (excluding idle time). It's set once per frame in FViewport::Draw. */
extern RENDERCORE_API uint32 GGameThreadTime;

namespace BlackboxSDK {
extern bool IsAPIKeyOverriden;
extern bool IsGameVersionIDOverriden;
extern bool IsNamespaceOverriden;

static bool
ZlibCompress(void* CompressedBuffer, int32& CompressedSize, const void* UncompressedBuffer, int32 UncompressedSize)
{
    return FCompression::CompressMemory(
        NAME_Zlib, CompressedBuffer, CompressedSize, UncompressedBuffer, UncompressedSize);
}
static void SessionCreatedCallback(const bbx_callback_http_response&, const char*);
static void MachineInfoGatherCallback();
static void OnPlaytestIdRetrieved(const char*);
static void MatchCreatedCallback(bool /*IsSuccessful*/, const char* /*ErrorMessage*/, const char* /*MatchID*/);
static void MatchSessionStartedCallback(bool /*IsSuccessful*/, const char* /*ErrorMessage*/);
static void MatchSessionEndedCallback(bool /*IsSuccessful*/, const char* /*ErrorMessage*/);
} // namespace BlackboxSDK

class MissionOutputDevice : public FOutputDevice {
public:
    MissionOutputDevice()
    {
        check(GLog);
        GLog->AddOutputDevice(this);
        if (GLog->IsRedirectingTo(this))
            return; // Never gets hit

        return;
    };

    ~MissionOutputDevice()
    {
        if (GLog != nullptr) {
            GLog->RemoveOutputDevice(this);
        }
    };

    void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
    {
        FString Format = FOutputDeviceHelper::FormatLogLine(Verbosity, Category, V, GPrintLogTimes);
        auto Conv = StringCast<ANSICHAR>(*Format);
        const char* UE_log_char = Conv.Get();
        bbx_collect_log_streaming_data(UE_log_char);
    }

    void SerializeGlobalLogBacklog()
    {
        if (GLog != nullptr) {
            GLog->SerializeBacklog(this);
        }
    }
};

#define LOCTEXT_NAMESPACE "FAccelByteBlackBoxSDK"

class FAccelByteBlackBoxSDKModule : public IAccelByteBlackBoxSDKModuleInterface {
public:
    FAccelByteBlackBoxSDKModule();
    ~FAccelByteBlackBoxSDKModule();
    void StartupModule() override;
    void ShutdownModule() override;
    void Start() override;
    void Stop() override;
    void Tick(float DeltaTime) override;
    void FeedKeyboardInput(APlayerController* PlayerController) override;
    void DeleteAdditionalInfoField(FString FieldName) override;
    void EmptyAdditionalInfo() override;
    bool UpdateAdditionalInfo(FString FieldName, FString Value) override;
    FString GetAdditionalInfoValue(FString FieldName) override;
    void EnableLog(bool Enable) override;
    void SetLogCallbackSeverity(uint8 MaxLogSeverity) override;
    void UpdateSessionWithExternalUserID(FString ExternalUserID) override;
    void UpdateSessionWithExternalSessionID(FString ExternalSessionID) override;
    void CreateMatch(
        const FString& PlatformMatchID,
        const FString& PlatformMatchIDType,
        const FOnBlackBoxMatchIDRetrievedDelegate& Callback) override;
    void
    BeginMatchSession(const FString& PlatformMatchID, const FOnBlackBoxMatchSessionStartedDelegate& Callback) override;
    void
    EndMatchSession(const FString& PlatformMatchID, const FOnBlackBoxMatchSessionStartedDelegate& Callback) override;

    static FAccelByteBlackBoxSDKModule* Instance();

private:
    void StartSDK();
    void StopSDK();
    void RegisterSettings();
    void UnregisterSettings();
    bool LoadSettings();
    void OnPostEngineInit();
    void OnEnginePreExit();
    void OnPropertyChanged(UObject* ModifiedObject, FPropertyChangedEvent& PropertyChangedEvent);
    void GatherProcessInformation();
    void GatherGPUInformation();
    void GatherUserInformation();
    FString GetDeviceId();
    void SetSDKInfoCrashContextXML();
    void SetOSInfoCrashContextXML();

    void RegisterViewportResizedCallback();
    void UnregisterViewportResizedCallback();
    void ViewportResized(FViewport*, uint32);
    void OnGameStart(const UWorld::FActorsInitializedParams& InitializationValues);
    void OnGameStop(UWorld* World, bool BoolArg1, bool BoolArg2);
    bool OnEngineTick(float TickTime);

    void OnIssueReporterTriggered();
    bool IsModuleLibraryLoaded();
    FString GetCrashGUID();

#if BLACKBOX_UE_WINDOWS
private:
    void OnGameCrash();

private:
    FDelegateHandle OnCrashHandle;
#endif

private:
    static FAccelByteBlackBoxSDKModule* Self;
    void* BlackBoxDllHandle = nullptr;
    FString SessionId;
    FString PlaytestId;
    FString SDKVersion;
    TUniquePtr<FInfoManager> InfoManager;
    TUniquePtr<FBackbufferManager> BackbufferManager;
    TUniquePtr<FBlackBoxCrashHandler> CrashHandler;
    FDelegateHandle OnPropertyChangedDelegateHandle;
    FDelegateHandle OnBackBufferReadyDelegate;
    FDelegateHandle OnViewportResizedHandle;
    FString IssueFolder;
    FString IssueReportDirectory;
    bool ConfigValidated = false;
    bool InGameSession = false;
    TUniquePtr<MissionOutputDevice> BlackBoxLogger;
    FOnBlackBoxMatchIDRetrievedDelegate OnBlackBoxMatchIDRetrieved;
    FOnBlackBoxMatchSessionStartedDelegate OnBlackBoxMatchSessionStarted;
    FOnBlackBoxMatchSessionEndedDelegate OnBlackBoxMatchSessionEnded;

private:
    friend void BlackboxSDK::SessionCreatedCallback(const bbx_callback_http_response&, const char*);
    friend void BlackboxSDK::MachineInfoGatherCallback();
    friend void BlackboxSDK::OnPlaytestIdRetrieved(const char*);
    friend void BlackboxSDK::MatchCreatedCallback(bool, const char*, const char*);
    friend void BlackboxSDK::MatchSessionStartedCallback(bool, const char*);
    friend void BlackboxSDK::MatchSessionEndedCallback(bool, const char*);
};
FAccelByteBlackBoxSDKModule* FAccelByteBlackBoxSDKModule::Self = nullptr;

void BlackboxSDK::SessionCreatedCallback(const bbx_callback_http_response& resp, const char* session_id)
{
    auto ModuleInstance = FAccelByteBlackBoxSDKModule::Instance();
    if (ModuleInstance) {
        ModuleInstance->SessionId = FString::Printf(TEXT("%s"), UTF8_TO_TCHAR(session_id));
        if (!resp.is_success) {
            UE_LOG(
                LogBlackBox,
                Warning,
                TEXT("Cannot create a new session [%d], Trace ID: %s"),
                resp.http_status,
                UTF8_TO_TCHAR(resp.trace_id));
            return;
        }
        blackbox::unreal::WriteSessionID(UTF8_TO_TCHAR(session_id));
        UE_LOG(LogBlackBox, Log, TEXT("Got Session ID: %s"), *ModuleInstance->SessionId);
        auto ConfigInfo = ModuleInstance->InfoManager->GetConfigInformation();
#if BLACKBOX_SDK_USE_PROFILING
        if (ConfigInfo.EnableBasicProfiling) {
            bbx_start_profiling();
        }
#endif
#if !BLACKBOX_UE_SONY
        FPlatformCrashContext::SetGameData(TEXT("BlackBox.SessionID"), ModuleInstance->SessionId);
#endif
    }
}

void BlackboxSDK::OnPlaytestIdRetrieved(const char* playtest_id)
{
    auto ModuleInstance = FAccelByteBlackBoxSDKModule::Instance();
    if (ModuleInstance) {
        ModuleInstance->PlaytestId = FString::Printf(TEXT("%s"), UTF8_TO_TCHAR(playtest_id));
#if !BLACKBOX_UE_SONY
        FPlatformCrashContext::SetGameData(TEXT("BlackBox.PlayTestID"), ModuleInstance->PlaytestId);
#endif
    }
}

void BlackboxSDK::MatchCreatedCallback(bool IsSuccessful, const char* ErrorMessage, const char* MatchID)
{
    auto ModuleInstance = FAccelByteBlackBoxSDKModule::Instance();
    if (!ModuleInstance) {
        return;
    }
    ModuleInstance->OnBlackBoxMatchIDRetrieved.ExecuteIfBound(
        IsSuccessful, UTF8_TO_TCHAR(ErrorMessage), UTF8_TO_TCHAR(MatchID));
}

void BlackboxSDK::MatchSessionStartedCallback(bool IsSuccessful, const char* ErrorMessage)
{
    auto ModuleInstance = FAccelByteBlackBoxSDKModule::Instance();
    if (!ModuleInstance) {
        return;
    }
    ModuleInstance->OnBlackBoxMatchSessionStarted.ExecuteIfBound(IsSuccessful, UTF8_TO_TCHAR(ErrorMessage));
}

void BlackboxSDK::MatchSessionEndedCallback(bool IsSuccessful, const char* ErrorMessage)
{
    auto ModuleInstance = FAccelByteBlackBoxSDKModule::Instance();
    if (!ModuleInstance) {
        return;
    }
    ModuleInstance->OnBlackBoxMatchSessionEnded.ExecuteIfBound(IsSuccessful, UTF8_TO_TCHAR(ErrorMessage));
}

void BlackboxSDK::MachineInfoGatherCallback()
{
    auto ModuleInstance = FAccelByteBlackBoxSDKModule::Instance();
    if (ModuleInstance) {
        ModuleInstance->SetOSInfoCrashContextXML();
    }
}

#if BLACKBOX_UE_WINDOWS
void FAccelByteBlackBoxSDKModule::OnGameCrash()
{
    bbx_handle_crash();
}
#endif

void FAccelByteBlackBoxSDKModule::OnGameStart(const UWorld::FActorsInitializedParams& InitializationValues)
{
    if (IsValid(InitializationValues.World) && InitializationValues.World->GetGameInstance() != nullptr) {
        InGameSession = true;
    }
}

void FAccelByteBlackBoxSDKModule::OnGameStop(UWorld* World, bool BoolArg1, bool BoolArg2)
{
    if (IsValid(World) && World->GetGameInstance() != nullptr) {
        InGameSession = false;
    }
}

bool FAccelByteBlackBoxSDKModule::IsModuleLibraryLoaded()
{
#if BLACKBOX_UE_WINDOWS || BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX || BLACKBOX_UE_LINUX || BLACKBOX_UE_MAC
    return BlackBoxDllHandle != nullptr;
#else
    return true;
#endif
}

FAccelByteBlackBoxSDKModule::FAccelByteBlackBoxSDKModule()
{
    Self = this;
#if BLACKBOX_UE_WINDOWS
    FString DllPath =
        FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/DLLs/x64/Win/relwithdebinfo") / TEXT("blackbox-core.dll");
#elif BLACKBOX_UE_LINUX
    FString DllPath =
        FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/Libs/x64/Linux/relwithdebinfo") / TEXT("libblackbox-core.so");
#elif BLACKBOX_UE_MAC
    FString DllPath =
        FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/Libs/x64/Mac/relwithdebinfo") / TEXT("libblackbox-core.dylib");
#elif BLACKBOX_UE_XBOXONEGDK
    FString DllPath = FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/DLLs/x64/XBCommon/relwithdebinfo") /
                      TEXT("BlackBoxSDK-XboxSeriesX.dll");
#elif BLACKBOX_UE_XSX
    FString DllPath = FPaths::ProjectPluginsDir() / TEXT("BlackBoxSDK/DLLs/x64/XSX/relwithdebinfo") /
                      TEXT("BlackBoxSDK-XboxSeriesX.dll");
#endif
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2 && (BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX)
    size_t AbsoluteDllPathLenBuffer = 512;
    TArray<char> AbsoluteDllPathBuffer;
    AbsoluteDllPathBuffer.SetNumZeroed(AbsoluteDllPathLenBuffer);
    size_t required_len;

    error_code err_code = bbx_convert_relative_path_to_absolute(
        TCHAR_TO_UTF8(*DllPath), AbsoluteDllPathBuffer.GetData(), AbsoluteDllPathLenBuffer, &required_len);

    if (err_code == static_cast<error_code>(blackbox::error::code::NO_ERROR_)) {
        DllPath = UTF8_TO_TCHAR(AbsoluteDllPathBuffer.GetData());
    }
    else if (err_code == static_cast<error_code>(blackbox::error::code::BUFFER_TOO_SMALL)) {
        AbsoluteDllPathBuffer.SetNumZeroed(required_len);
        err_code = bbx_convert_relative_path_to_absolute(
            TCHAR_TO_UTF8(*DllPath), AbsoluteDllPathBuffer.GetData(), required_len, &required_len);

        if (err_code == static_cast<error_code>(blackbox::error::code::NO_ERROR_)) {
            DllPath = UTF8_TO_TCHAR(AbsoluteDllPathBuffer.GetData());
        }
    }
#endif
#if BLACKBOX_UE_WINDOWS || BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX || BLACKBOX_UE_LINUX || BLACKBOX_UE_MAC
    BlackBoxDllHandle = FPlatformProcess::GetDllHandle(*DllPath);
    if (!BlackBoxDllHandle) {
        UE_LOG(
            LogBlackBox,
            Error,
            TEXT("Can't find BlackBox DLL, please make sure that the file exist. BlackBox functionality will not be "
                 "available."));
        return;
    }
#endif
    bbx_initiate_base_modules();

#if BLACKBOX_SDK_USE_LOG_STREAMING
    BlackBoxLogger = MakeUnique<MissionOutputDevice>();
    BlackBoxLogger->SerializeGlobalLogBacklog();
#endif

#if BLACKBOX_UE_WINDOWS && BLACKBOX_SDK_USE_CRASH_REPORT
    OnCrashHandle = FCoreDelegates::OnHandleSystemError.AddRaw(this, &FAccelByteBlackBoxSDKModule::OnGameCrash);
#endif
}

FAccelByteBlackBoxSDKModule::~FAccelByteBlackBoxSDKModule()
{
    Self = nullptr;
#if BLACKBOX_UE_WINDOWS && BLACKBOX_SDK_USE_CRASH_REPORT
    FCoreDelegates::OnHandleSystemError.Remove(OnCrashHandle);
#endif

    bbx_shutdown_base_modules();

#if BLACKBOX_UE_WINDOWS || BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX || BLACKBOX_UE_LINUX || BLACKBOX_UE_MAC
    if (BlackBoxDllHandle) {
        FPlatformProcess::FreeDllHandle(BlackBoxDllHandle);
    }
#endif
}

FAccelByteBlackBoxSDKModule* FAccelByteBlackBoxSDKModule::Instance()
{
    return Self;
}

void FAccelByteBlackBoxSDKModule::StartupModule()
{
    if (!IsModuleLibraryLoaded()) {
        return;
    }
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Startup Module"));

    std::string sdk_ver = bbx_info_get_version();
    SDKVersion = FString(UTF8_TO_TCHAR(sdk_ver.c_str()));
    UE_LOG(LogBlackBox, Log, TEXT("SDK Version: %s"), *SDKVersion);

    // Instantiate Setting Storage Class
    InfoManager = MakeUnique<FInfoManager>();

    RegisterSettings();
    if (!LoadSettings()) {
        ShutdownModule();
        return;
    }

#if BLACKBOX_UE_PS5 || BLACKBOX_UE_PS4
    // set up the http caller before initializing the module (module is not needed to set this instance)
    blackbox::unreal::set_unreal_singleton(new blackbox::unreal::sdk_http_impl());
#endif
    // Gather Settings Value
    bbx_config_set_gpu_device_id(GRHIDeviceId);
#if BLACKBOX_SDK_USE_CRASH_VIDEO
    BackbufferManager = MakeUnique<FBackbufferManager>();
    bbx_initiate_main_modules(static_cast<uint8_t>(BackbufferManager->GetActiveRenderingAPI()));
#else
    // initiate with null renderer
    bbx_initiate_main_modules(2);
#endif

#if BLACKBOX_SDK_USE_DEVICE_INFORMATION_GATHERING
    bbx_start_gather_device_info(&BlackboxSDK::MachineInfoGatherCallback);
#endif

#if BLACKBOX_UE_PS5 || BLACKBOX_UE_PS4
    bbx_config_set_engine_version(TCHAR_TO_UTF8(*FEngineVersion::Current().ToString()));
#endif

    GatherProcessInformation();
    bbx_set_file_compression_function(&BlackboxSDK::ZlibCompress);
#if BLACKBOX_SDK_USE_CRASH_REPORT
    SetSDKInfoCrashContextXML();
    // Start up Crash Handler
    CrashHandler = MakeUnique<FBlackBoxCrashHandler>();
#endif
#if BLACKBOX_SDK_USE_CRASH_VIDEO
    RegisterViewportResizedCallback();
#endif
    FWorldDelegates::OnWorldInitializedActors.AddRaw(this, &FAccelByteBlackBoxSDKModule::OnGameStart);
    FWorldDelegates::OnWorldCleanup.AddRaw(this, &FAccelByteBlackBoxSDKModule::OnGameStop);
#if ENGINE_MAJOR_VERSION == 4
    using Ticker = FTicker;
#else
    using Ticker = FTSTicker;
#endif
    Ticker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FAccelByteBlackBoxSDKModule::OnEngineTick));

    IssueFolder = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("Issues"));

    FCoreDelegates::OnPostEngineInit.AddRaw(this, &FAccelByteBlackBoxSDKModule::OnPostEngineInit);
    FCoreDelegates::OnEnginePreExit.AddRaw(this, &FAccelByteBlackBoxSDKModule::OnEnginePreExit);

    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Startup Module - END"));
}

void FAccelByteBlackBoxSDKModule::ShutdownModule()
{
    if (!IsModuleLibraryLoaded()) {
        return;
    }
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Shutdown Module"));
    bbx_stop_recording();
    bbx_suspend_shared_mem();
    if (BackbufferManager) {
        BackbufferManager->UnregisterBackbufferCallback();
    }
    UnregisterViewportResizedCallback();
    UnregisterSettings();
    bbx_shutdown_main_modules();
#if BLACKBOX_UE_PS5 || BLACKBOX_UE_PS4
    blackbox::unreal::destroy_unreal_singleton();
#endif
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Shutdown Module - END"));
}

void FAccelByteBlackBoxSDKModule::OnPostEngineInit()
{
    StartSDK();
}

void FAccelByteBlackBoxSDKModule::OnEnginePreExit()
{
    StopSDK();
}

void FAccelByteBlackBoxSDKModule::StartSDK()
{
    if (!IsModuleLibraryLoaded()) {
        return;
    }
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Start SDK"));
#if WITH_EDITOR
    bbx_clear_pending_tasks();
#endif
    ConfigValidated = bbx_validate_config();
    if (ConfigValidated) {
#if BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX
        UE_LOG(LogBlackBox, Log, TEXT("Sending previous crashes..."));
        bbx_send_crash(TCHAR_TO_ANSI(TEXT("D:\\")), nullptr);
        UE_LOG(LogBlackBox, Log, TEXT("Fetching default user gamertag"));
        bbx_set_gamertag(FInfoManager::GetGamertag().c_str());
#endif
        GatherGPUInformation();
        if (bbx_get_is_feature_enabled_from_env("BLACKBOXSDK_FLAG_CREATE_SESSION_V2")) {
            GatherUserInformation();
        }
        UE_LOG(LogBlackBox, Log, TEXT("Creating Session"));
        SDKInformation Info = InfoManager->GetSDKInformation();
#if BLACKBOX_SDK_USE_SESSION_CREATION
        if (Info.BuildId.IsEmpty()) {
            bbx_start_new_session_on_editor(&BlackboxSDK::SessionCreatedCallback, &BlackboxSDK::OnPlaytestIdRetrieved);
        }
        else {
            bbx_start_new_session(&BlackboxSDK::SessionCreatedCallback, &BlackboxSDK::OnPlaytestIdRetrieved);
        }
#endif
    }
    else {
        UE_LOG(LogBlackBox, Warning, TEXT("BlackBox SDK configuration is invalid"));
    }
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Start SDK - END"));
}

void FAccelByteBlackBoxSDKModule::StopSDK()
{
    if (!IsModuleLibraryLoaded()) {
        return;
    }
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Stop SDK"));
    if (ConfigValidated) {
        bbx_stop_profiling();
        InfoManager->ResetKeyInformation();
        bbx_clear_pending_tasks();
        bbx_stop_log_streaming();
    }
    UE_LOG(LogBlackBox, Log, TEXT("SDK MODULE Stop SDK - END"));
}

[[deprecated]] void FAccelByteBlackBoxSDKModule::Start()
{
}
[[deprecated]] void FAccelByteBlackBoxSDKModule::Stop()
{
}

void FAccelByteBlackBoxSDKModule::RegisterSettings()
{
    UBlackBoxSettings* BlackBoxSettings = GetMutableDefault<UBlackBoxSettings>();
    BlackBoxSettings->InitializeLocalConfigProperties();
    FString BlackBoxConfig = FPaths::ProjectConfigDir() / TEXT("BlackBox.ini");
    bool isSDKEnabled;
    if (GConfig->GetBool(TEXT("BlackBoxSettings"), TEXT("Enable"), isSDKEnabled, BlackBoxConfig)) {
        BlackBoxSettings->HideIssueReporterSettings = !isSDKEnabled;
    }
#if WITH_EDITOR
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
        SettingsModule->RegisterSettings(
            TEXT("Project"),
            TEXT("Plugins"),
            TEXT("AccelByte BlackBox SDK"),
            FText::FromName(TEXT("AccelByte BlackBox SDK")),
            FText::FromName(TEXT("Setup your plugin.")),
            BlackBoxSettings);
    }
    OnPropertyChangedDelegateHandle =
        FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FAccelByteBlackBoxSDKModule::OnPropertyChanged);
#endif
}

void FAccelByteBlackBoxSDKModule::UnregisterSettings()
{
#if WITH_EDITOR
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
        SettingsModule->UnregisterSettings(TEXT("Project"), TEXT("Plugins"), TEXT("AccelByte BlackBox SDK"));
    }
    FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(OnPropertyChangedDelegateHandle);
#endif
}

bool FAccelByteBlackBoxSDKModule::LoadSettings()
{
    SDKInformation SDKInfo;
    SDKInfo.BaseUrl = TEXT(__BLACKBOX_BASE_URL__);
    SDKInfo.IamUrl = FString(TEXT(__BLACKBOX_BASE_URL__)) + TEXT("/iam");
    SDKInfo.DownloadURL = TEXT(__BLACKBOX_DOWNLOAD_URL__);
    SDKInfo.LatestReleaseURL = TEXT(__BLACKBOX_RELEASE_INFO_URL__);
    bool ExperimentalServerBuildIdFeature = GetDefault<UBlackBoxSettings>()->ExperimentalServerBuildIdFeature;
    bool IsExperimentalServerBuildIdFeatureActive = IsRunningDedicatedServer() && ExperimentalServerBuildIdFeature;
    SDKInfo.IsRunningAsServer = IsRunningDedicatedServer();
    if (SDKInfo.IsRunningAsServer) {
        UE_LOG(LogBlackBox, Log, TEXT("BlackBox SDK is running under a server entity."));
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("BlackBox SDK is running under a client entity."));
    }
#if WITH_EDITOR
    SDKInfo.CoreSDKConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultBlackBox.ini");
#elif BLACKBOX_UE_WINDOWS
    SDKInfo.CoreSDKConfigPath = FPaths::GeneratedConfigDir() / TEXT("Blackbox") / TEXT("DefaultBlackBox.ini");
#else
    SDKInfo.CoreSDKConfigPath = TEXT("");
#endif

    FString BlackBoxConfig = FPaths::ProjectConfigDir() / TEXT("BlackBox.ini");
    if (FPaths::FileExists(BlackBoxConfig)) {
        if (GConfig->GetBool(TEXT("BlackBoxSettings"), TEXT("Enable"), SDKInfo.IsEnable, BlackBoxConfig)) {
            UE_LOG(
                LogBlackBox,
                Log,
                TEXT("BlackBox Setting to override BlackBoxSDK is exist. Enable BlackBoxSDK = %s"),
                SDKInfo.IsEnable ? TEXT("TRUE") : TEXT("FALSE"));

            const FString ForceEnableSDKEnvironmentVariable =
                FGenericPlatformMisc::GetEnvironmentVariable(TEXT("FORCE_ENABLE_BLACKBOX_SDK"));
            if (!ForceEnableSDKEnvironmentVariable.IsEmpty()) {
                if (ForceEnableSDKEnvironmentVariable == TEXT("1")) {
                    SDKInfo.IsEnable = true;
                    UE_LOG(
                        LogBlackBox,
                        Log,
                        TEXT("BlackBox Environment Variable to force enable BlackBoxSDK exists. Enable BlackBoxSDK "
                             "= "
                             "True"));
                }
            }
        }
        if (!GConfig->GetString(TEXT("BlackBoxSettings"), TEXT("APIKey"), SDKInfo.APIKey, BlackBoxConfig)) {
            UE_LOG(
                LogBlackBox,
                Log,
                TEXT("Missing APIKey value from BlackBox.ini, fallback to the entry in DefaultEngine.ini"));
            SDKInfo.APIKey = TEXT("");
            BlackboxSDK::IsAPIKeyOverriden = false;
        }
        if (!GConfig->GetString(
                TEXT("BlackBoxSettings"), TEXT("GameVersionID"), SDKInfo.GameVersionId, BlackBoxConfig)) {
            UE_LOG(
                LogBlackBox,
                Log,
                TEXT("Missing GameVersionId value from BlackBox.ini, fallback to the entry in DefaultEngine.ini"));
            SDKInfo.GameVersionId = TEXT("");
            BlackboxSDK::IsGameVersionIDOverriden = false;
        }
        if (!GConfig->GetString(TEXT("BlackBoxSettings"), TEXT("Namespace"), SDKInfo.Namespace, BlackBoxConfig)) {
            UE_LOG(
                LogBlackBox,
                Log,
                TEXT("Missing Namespace value from BlackBox.ini, fallback to the entry in DefaultEngine.ini"));
            SDKInfo.Namespace = TEXT("");
            BlackboxSDK::IsNamespaceOverriden = false;
        }
        if (SDKInfo.BuildId.IsEmpty()) {
            FString BuildId{};

            if (!IsExperimentalServerBuildIdFeatureActive) {
                if (!GConfig->GetString(TEXT("BlackBoxSettings"), TEXT("BuildID"), BuildId, BlackBoxConfig)) {
                    BuildId = TEXT("");
                }
            }
            else {
                if (!GConfig->GetString(TEXT("BlackBoxSettings"), TEXT("ServerBuildID"), BuildId, BlackBoxConfig)) {
                    BuildId = TEXT("");
                }
            }

            SDKInfo.BuildId = BuildId;
        }
        if (SDKInfo.IsEnable) {
            if (!GConfig->GetBool(
                    TEXT("BlackBoxSettings"),
                    TEXT("EnableIssueReporter"),
                    SDKInfo.EnableIssueReporter,
                    BlackBoxConfig)) {
                SDKInfo.EnableIssueReporter = GetDefault<UBlackBoxSettings>()->EnableIssueReporter;
            }

            FString IssueHotkeyStr{};
            if (GConfig->GetString(
                    TEXT("BlackBoxSettings"), TEXT("IssueReporterHotkey"), IssueHotkeyStr, BlackBoxConfig)) {
                FName IssueHotkeyName = FName(*IssueHotkeyStr, EFindName::FNAME_Find);
                if (IssueHotkeyName != NAME_None) {
                    SDKInfo.IssueReporterHotkey = FKey(IssueHotkeyName);
                }
                else {
                    SDKInfo.IssueReporterHotkey = BlackBoxIssueReporter::ConvertEnumKeyToFKey(
                        GetDefault<UBlackBoxSettings>()->IssueReporterHotkey);
                }
            }
            else {
                SDKInfo.IssueReporterHotkey =
                    BlackBoxIssueReporter::ConvertEnumKeyToFKey(GetDefault<UBlackBoxSettings>()->IssueReporterHotkey);
            }

            if (!GConfig->GetBool(
                    TEXT("BlackBoxSettings"),
                    TEXT("UseUnrealToCaptureScreenshot"),
                    SDKInfo.UseUnrealToCaptureScreenshot,
                    BlackBoxConfig)) {
                SDKInfo.UseUnrealToCaptureScreenshot = GetDefault<UBlackBoxSettings>()->UseUnrealToCaptureScreenshot;
            }
        }
        else {
            SDKInfo.EnableIssueReporter = false;
        }

#if WITH_EDITOR
        // register settings detail panel customization
        FPropertyEditorModule& PropertyModule =
            FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.RegisterCustomClassLayout(
            UBlackBoxSettings::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FBlackBoxSettingsCustomization::MakeInstance));
#endif
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("BlackBox.ini file isn't present, fallback to the entries in DefaultEngine.ini"));
        BlackboxSDK::IsNamespaceOverriden = false;
        BlackboxSDK::IsGameVersionIDOverriden = false;
        BlackboxSDK::IsAPIKeyOverriden = false;
    }
    if (SDKInfo.APIKey.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Get APIKey entry from DefaultEngine.ini"));
        SDKInfo.APIKey = GetDefault<UBlackBoxSettings>()->APIKey;
    }
    if (SDKInfo.Namespace.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Get Namespace entry from DefaultEngine.ini"));
        SDKInfo.Namespace = GetDefault<UBlackBoxSettings>()->Namespace;
    }
    if (SDKInfo.GameVersionId.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Get GameVersionId entry from DefaultEngine.ini"));
        SDKInfo.GameVersionId = GetDefault<UBlackBoxSettings>()->GameVersionID;
    }

    if (SDKInfo.Namespace.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Namespace not set"));
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("Namespace    : %s"), *SDKInfo.Namespace);
    }

    if (SDKInfo.GameVersionId.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Version ID not set"));
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("Version ID   : %s"), *SDKInfo.GameVersionId);
    }

    if (SDKInfo.IsEnable) {
        if (!SDKInfo.IssueReporterHotkey.IsValid()) {
            UE_LOG(LogBlackBox, Log, TEXT("Issue Reporter hotkey not set, using default value..."));
            SDKInfo.IssueReporterHotkey =
                BlackBoxIssueReporter::ConvertEnumKeyToFKey(GetDefault<UBlackBoxSettings>()->IssueReporterHotkey);
        }
    }

    GetMutableDefault<UBlackBoxSettings>()->InitializeNeedToRestartOnChangeProperties();

    FString DetectedBuildIdText = IsExperimentalServerBuildIdFeatureActive ? "Build ID (Server)" : "Build ID";
    if (SDKInfo.BuildId.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("%s not set"), *DetectedBuildIdText);
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("%s     : %s"), *DetectedBuildIdText, *SDKInfo.BuildId);
    }

    if (!GConfig->GetString(
            TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectID"), SDKInfo.ProjectId, GGameIni)) {
        UE_LOG(
            LogBlackBox,
            Warning,
            TEXT("Cannot run SDK, Missing ProjectID in [/Script/EngineSettings.GeneralProjectSettings] of "
                 "DefaultGame.ini"));
        return false;
    }
    if (GetDefault<UBlackBoxSettings>()->EnableLog &&
        static_cast<uint8>(GetDefault<UBlackBoxSettings>()->LogSeverity) > 0) {
        EnableLog(true);
        SetLogCallbackSeverity(static_cast<uint8>(GetDefault<UBlackBoxSettings>()->LogSeverity));
    }
    else if (static_cast<uint8>(GetDefault<UBlackBoxSettings>()->LogSeverity) == 0) {
        GetMutableDefault<UBlackBoxSettings>()->EnableLog = true;
        EnableLog(true);
        GetMutableDefault<UBlackBoxSettings>()->LogSeverity = BlackBoxLogSeverity::WARNING;
        SetLogCallbackSeverity(static_cast<uint8>(BlackBoxLogSeverity::WARNING));
    }

    InfoManager->SetSDKInformation(SDKInfo);
    return true;
}

void FAccelByteBlackBoxSDKModule::OnPropertyChanged(
    UObject* ModifiedObject, FPropertyChangedEvent& PropertyChangedEvent)
{
    if (UBlackBoxSettings* settings = Cast<UBlackBoxSettings>(ModifiedObject)) {
        SDKInformation Info = InfoManager->GetSDKInformation();
        FString BlackBoxConfig = FPaths::ProjectConfigDir() / TEXT("BlackBox.ini");
        if (!BlackboxSDK::IsAPIKeyOverriden) {
            Info.APIKey = settings->APIKey;
        }
        if (!BlackboxSDK::IsNamespaceOverriden) {
            Info.Namespace = settings->Namespace;
        }
        if (!BlackboxSDK::IsGameVersionIDOverriden) {
            Info.GameVersionId = settings->GameVersionID;
        }
        if (Info.EnableIssueReporter != settings->EnableIssueReporter) {
            GConfig->SetBool(
                TEXT("BlackBoxSettings"), TEXT("EnableIssueReporter"), settings->EnableIssueReporter, BlackBoxConfig);
            Info.EnableIssueReporter = settings->EnableIssueReporter;
        }
        if (Info.IssueReporterHotkey != BlackBoxIssueReporter::ConvertEnumKeyToFKey(settings->IssueReporterHotkey)) {
            GConfig->SetString(
                TEXT("BlackBoxSettings"),
                TEXT("IssueReporterHotkey"),
                *BlackBoxIssueReporter::ConvertEnumKeyToString(settings->IssueReporterHotkey),
                BlackBoxConfig);
            Info.IssueReporterHotkey = BlackBoxIssueReporter::ConvertEnumKeyToFKey(settings->IssueReporterHotkey);
        }
        if (settings->EnableLog && static_cast<uint8>(settings->LogSeverity) > 0) {
            EnableLog(true);
            SetLogCallbackSeverity(static_cast<uint8>(settings->LogSeverity));
        }
        else {
            EnableLog(false);
        }

        if (settings->CheckNeedToRestart(PropertyChangedEvent.GetPropertyName().ToString())) {
            settings->ShowMustRestartDialog();
        }

        settings->ApplyLocalConfigProperties();
        bbx_save_local_config(TCHAR_TO_UTF8(FApp::GetProjectName()));

        InfoManager->SetSDKInformation(Info);
    }
}

static short previous_trigger_state = 0;
bool FAccelByteBlackBoxSDKModule::OnEngineTick(float TickTime)
{
#if BLACKBOX_SDK_USE_CRASH_VIDEO
    if ((bbx_config_get_store_crash_video() ||
         (bbx_config_get_enable_issue_reporter() && !bbx_config_get_use_engine_to_capture_screenshot())) &&
        BackbufferManager != nullptr && !BackbufferManager->GetIsActive()) {
        BackbufferManager->RegisterBackbufferCallback();
    }

    bbx_engine_tick(TickTime);
#endif
    if (InGameSession) {
        bbx_tick(TickTime);
#if BLACKBOX_SDK_USE_PROFILING
        float game_thread_time_ms = FPlatformTime::ToMilliseconds(GGameThreadTime);
        float render_thread_time_ms = FPlatformTime::ToMilliseconds(GRenderThreadTime);
        bbx_update_profiling_basic_data(TickTime, render_thread_time_ms, game_thread_time_ms);
#endif
    }
#if BLACKBOX_UE_WINDOWS && BLACKBOX_SDK_USE_ISSUE_REPORTER
    if (bbx_config_get_enable_issue_reporter()) {
        auto keycode = BlackBoxIssueReporter::GetIssueReporterHotkey();
        if (keycode != 0) {
            short trigger_state_now = GetKeyState(keycode) & 0x8000;

            if ((trigger_state_now & 0x8000) == 0x8000 && (previous_trigger_state & 0x8000) == 0) {
                if (FSlateApplication::IsInitialized() && FSlateApplication::Get().IsActive()) {
                    OnIssueReporterTriggered();
                }
            }
            previous_trigger_state = trigger_state_now;
        }
        else {
            UE_LOG(
                LogBlackBox,
                Warning,
                TEXT("Failed to get Issue Reporter Hotkey, please set a hotkey and re-enable Issue Reporter "
                     "afterwards."));
            bbx_config_set_enable_issue_reporter(false);
        }
    }
#endif
    return true;
}

void FAccelByteBlackBoxSDKModule::GatherProcessInformation()
{
    ProcessInformation ProcInfo;
    ProcInfo.PID = FPlatformProcess::GetCurrentProcessId();

    // Windows Specific helper and issue reporter applications
#if BLACKBOX_UE_WINDOWS
    FString arch{};
#    if defined _M_X64
    arch = "x64";
#    else
    arch = "x86";
#    endif

    // Helper
    FString HelperExePath{};
    HelperExePath = FPaths::ProjectPluginsDir() / +"BlackBoxSDK/helper/" + arch + "/blackbox_helper.exe";
    ProcInfo.BlackBoxHelperPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*HelperExePath);
    ProcInfo.BlackBoxHelperLogAlternativePath = FPaths::ProjectSavedDir();

    // Issue Reporter
    FString IssueReporterExePath{};
    IssueReporterExePath =
        FPaths::ProjectPluginsDir() / +"BlackBoxSDK/issue_reporter/" + arch + "/blackbox_issue_reporter.exe";
    ProcInfo.BlackBoxIssueReporterPath =
        IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*IssueReporterExePath);
#elif BLACKBOX_UE_LINUX
    // Helper
    FString HelperExePath{};
    HelperExePath = FPaths::ProjectPluginsDir() / +"BlackBoxSDK/helper/linux/blackbox_helper";
    ProcInfo.BlackBoxHelperPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*HelperExePath);
    ProcInfo.BlackBoxHelperLogAlternativePath = FPaths::ProjectSavedDir();
#endif

    // Gather Crash directory information
#if BLACKBOX_UE_WINDOWS
    FString CrashFolder = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("Crashes"));
    FString CrashGUIDString = GetCrashGUID();

#    if UE_BUILD_DEVELOPMENT
    FString CrashFolderAbsolute = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*CrashFolder);
#    else
    FString CrashFolderAbsolute = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*CrashFolder);
#    endif

    ProcInfo.CrashFolder = CrashFolderAbsolute;
    ProcInfo.CrashGUID = CrashGUIDString;
#elif BLACKBOX_UE_LINUX || BLACKBOX_UE_MAC
    FString CrashGUIDString;
    if (!FParse::Value(FCommandLine::Get(), TEXT("CrashGUID="), CrashGUIDString) || CrashGUIDString.Len() <= 0) {
        CrashGUIDString = FGuid::NewGuid().ToString();
        FCommandLine::Append(*FString::Printf(TEXT(" -CrashGUID=%s"), *CrashGUIDString));
    }
    FString CrashFolder = FPaths::Combine(
        *FPaths::ProjectSavedDir(),
        TEXT("Crashes"),
        *FString::Printf(
            TEXT("%sinfo-%s-pid-%d-%s"), TEXT("crash"), FApp::GetProjectName(), getpid(), *CrashGUIDString));
    FString CrashFolderAbsolute = FPaths::ConvertRelativePathToFull(CrashFolder);
    UE_LOG(LogBlackBox, Log, TEXT("Crashfolder: %s"), *CrashFolderAbsolute);
    ProcInfo.CrashFolder = CrashFolderAbsolute;
    ProcInfo.CrashGUID = CrashGUIDString;
#endif

    ProcInfo.LogSourceFilePath = FPlatformOutputDevices::GetAbsoluteLogFilename();
    InfoManager->SetProcessInformation(ProcInfo);
}

void FAccelByteBlackBoxSDKModule::SetSDKInfoCrashContextXML()
{
#if BLACKBOX_UE_WINDOWS || BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX || BLACKBOX_UE_LINUX || BLACKBOX_UE_MAC
    SDKInformation SDKInfo = InfoManager->GetSDKInformation();
    FPlatformCrashContext::SetGameData(TEXT("BlackBox.GameVersionID"), SDKInfo.GameVersionId);
    FPlatformCrashContext::SetGameData(TEXT("BlackBox.BuildId"), SDKInfo.BuildId);
    FPlatformCrashContext::SetGameData(TEXT("BlackBox.Namespace"), SDKInfo.Namespace);
    FPlatformCrashContext::SetGameData(TEXT("BlackBox.SDKVersion"), SDKVersion);
#endif
}

void FAccelByteBlackBoxSDKModule::SetOSInfoCrashContextXML()
{
#if BLACKBOX_UE_WINDOWS || BLACKBOX_UE_XBOXONEGDK || BLACKBOX_UE_XSX || BLACKBOX_UE_LINUX || BLACKBOX_UE_MAC
    OSInformation& OSInfo = InfoManager->GetOSInformation();
    FPlatformCrashContext::SetGameData(TEXT("BlackBox.ComputerName"), OSInfo.ComputerName);
    FPlatformCrashContext::SetGameData(TEXT("BlackBox.UserName"), OSInfo.UserName);
#endif
}

void FAccelByteBlackBoxSDKModule::GatherGPUInformation()
{
    GPUInformation GpuInfo;
    GpuInfo.Model = GRHIAdapterName;
    GpuInfo.DriverVer = FPlatformMisc::GetGPUDriverInfo(GRHIAdapterName).UserDriverVersion;
    InfoManager->SetGPUInformation(GpuInfo);
}

void FAccelByteBlackBoxSDKModule::GatherUserInformation()
{
    UserInformation UserInfo;
#if BLACKBOX_UE_WINDOWS
    UserInfo.IAMUserId = FPlatformMisc::GetEnvironmentVariable(TEXT("BLACKBOX_VAR_IAM_USER_ID"));
    if (!UserInfo.IAMUserId.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Got IAM User ID: %s"), *UserInfo.IAMUserId);
    }
    UserInfo.PlaytestId = FPlatformMisc::GetEnvironmentVariable(TEXT("BLACKBOX_VAR_PLAYTEST_ID"));
    if (!UserInfo.PlaytestId.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Got Playtest ID: %s"), *UserInfo.PlaytestId);
    }
#endif
    UserInfo.DeviceId = GetDeviceId();
    if (!UserInfo.DeviceId.IsEmpty()) {
        UE_LOG(LogBlackBox, Log, TEXT("Got Device ID: %s"), *UserInfo.DeviceId);
    }

    InfoManager->SetUserInformation(UserInfo);
}

FString FAccelByteBlackBoxSDKModule::GetDeviceId()
{
    FString PlatformDeviceId = FPlatformMisc::GetDeviceId();
    if (!PlatformDeviceId.IsEmpty()) {
        return PlatformDeviceId;
    }
    else {
        FString MacAddress;
        PRAGMA_DISABLE_DEPRECATION_WARNINGS
        const TArray<uint8> MacAddr = FPlatformMisc::GetMacAddress();
        PRAGMA_ENABLE_DEPRECATION_WARNINGS
        for (TArray<uint8>::TConstIterator it(MacAddr); it; ++it) { MacAddress += FString::Printf(TEXT("%02x"), *it); }
        if (MacAddress.IsEmpty()) {
            return FGuid::NewGuid().ToString();
        }
        else {
            return MacAddress;
        }
    }
}

void FAccelByteBlackBoxSDKModule::EnableLog(bool Enable)
{
    if (!IsModuleLibraryLoaded()) {
        return;
    }
    if (Enable) {
        auto log_cb = [](bbx_log_severity sev, const char* msg) {
            switch (sev) {
            case BBX_LOG_VERBOSE:
                UE_LOG(LogBlackBox, Log, TEXT("%s"), UTF8_TO_TCHAR(msg));
                break;
            case BBX_LOG_INFO:
                UE_LOG(LogBlackBox, Log, TEXT("%s"), UTF8_TO_TCHAR(msg));
                break;
            case BBX_LOG_WARNING:
                UE_LOG(LogBlackBox, Warning, TEXT("%s"), UTF8_TO_TCHAR(msg));
                break;
            case BBX_LOG_ERROR:
                UE_LOG(LogBlackBox, Warning, TEXT("%s"), UTF8_TO_TCHAR(msg));
                break;
            default:
                break;
            }
        };
        bbx_set_log_callback(log_cb);
    }
    else {
        bbx_set_log_callback(nullptr);
    }
}

void FAccelByteBlackBoxSDKModule::SetLogCallbackSeverity(uint8 MaxLogSeverity)
{
    if (!IsModuleLibraryLoaded()) {
        return;
    }

    bbx_log_severity LogSeverity{};
    switch (static_cast<BlackBoxLogSeverity>(MaxLogSeverity)) {
    case BlackBoxLogSeverity::VERBOSE:
        LogSeverity = BBX_LOG_DEBUG;
        break;
    case BlackBoxLogSeverity::WARNING:
        LogSeverity = BBX_LOG_WARNING;
        break;
    case BlackBoxLogSeverity::ERROR_:
        LogSeverity = BBX_LOG_ERROR;
        break;
    case BlackBoxLogSeverity::INFO:
        LogSeverity = BBX_LOG_INFO;
        break;
    default:
        // no log
        break;
    }
    bbx_set_log_severity(LogSeverity);
}

void FAccelByteBlackBoxSDKModule::UpdateSessionWithExternalUserID(FString ExternalUserID)
{
#if BLACKBOX_SDK_USE_EXTERNAL_USER_ID_SUPPORT
    if (!IsModuleLibraryLoaded()) {
        return;
    }
    bbx_update_existing_session_with_external_user_id(TCHAR_TO_UTF8(*ExternalUserID));
#endif
}
void FAccelByteBlackBoxSDKModule::UpdateSessionWithExternalSessionID(FString ExternalSessionID)
{
#if BLACKBOX_SDK_USE_EXTERNAL_USER_ID_SUPPORT
    if (!IsModuleLibraryLoaded()) {
        return;
    }
    bbx_update_existing_session_with_external_session_id(TCHAR_TO_UTF8(*ExternalSessionID));
#endif
}

[[deprecated]] void FAccelByteBlackBoxSDKModule::Tick(float DeltaTime)
{
}

void FAccelByteBlackBoxSDKModule::FeedKeyboardInput(APlayerController* PlayerController)
{
#if BLACKBOX_SDK_USE_CRASH_VIDEO
    if (!IsModuleLibraryLoaded()) {
        return;
    }
    if (!IsValid(PlayerController)) {
        return;
    }
    if (!InfoManager->IsKeyInformationPresent()) {
        InfoManager->SetupKeyInformation(PlayerController);
    }
    InputInformation& InputInfo = InfoManager->GetKeyInformation();
    for (uint64 i = 0; i < InputInfo.ActionKeyPair.Num(); i++) {
#    if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION <= 25)
        if (InputInfo.ActionKeyPair[i].Value.IsFloatAxis()) {
#    else
        if (InputInfo.ActionKeyPair[i].Value.IsAxis1D()) {
#    endif
            continue;
        }
        bbx_update_key_input(i, PlayerController->IsInputKeyDown(InputInfo.ActionKeyPair[i].Value));
    }
#endif
}

void FAccelByteBlackBoxSDKModule::OnIssueReporterTriggered()
{
#if BLACKBOX_UE_WINDOWS
    UE_LOG(LogBlackBox, Warning, TEXT("CAPTURING SCREENSHOT"));
    if (IssueFolder == "" || IssueFolder.IsEmpty()) {
        UE_LOG(LogBlackBox, Warning, TEXT("Issue Folder does not exist"));
    }
    bool is_screenshot_succeeded{};
    if (!bbx_config_get_use_engine_to_capture_screenshot()) {
        is_screenshot_succeeded = (bbx_capture_screenshot(TCHAR_TO_UTF8(*IssueFolder)) == 0);
    }
    if (!is_screenshot_succeeded || bbx_config_get_use_engine_to_capture_screenshot()) {
        // take_screenshoot_here
        UE_LOG(LogBlackBox, Warning, TEXT("Taking screenshot with UE Screenshot"));
#    if (ENGINE_MAJOR_VERSION < 5 && ENGINE_MINOR_VERSION < 1) || WITH_EDITOR
        UE_LOG(
            LogBlackBox,
            Warning,
            TEXT("Using Unreal Engine to capture screenshot is not recommended when using Unreal Engine 5.1 and below, "
                 "or when is in the editor window"));
#    endif
        size_t IssueReportDirLenBuffer = 512;
        std::vector<char> IssueReportDirBuffer;
        IssueReportDirBuffer.resize(IssueReportDirLenBuffer);
        if (bbx_get_issue_report_directory(
                TCHAR_TO_UTF8(*IssueFolder), IssueReportDirBuffer.data(), IssueReportDirLenBuffer)) {
            IssueReportDirectory = UTF8_TO_TCHAR(IssueReportDirBuffer.data());
        }

        if (!IssueReportDirectory.IsEmpty()) {
            is_screenshot_succeeded = (BlackBoxIssueReporter::TakeStandardScreenshotWithUE(IssueReportDirectory) == 0);
            if (is_screenshot_succeeded) {
                bbx_launch_issue_reporter(TCHAR_TO_UTF8(*IssueFolder));
            }
            else {
                UE_LOG(
                    LogBlackBox,
                    Warning,
                    TEXT("Capture screenshot failed - Unable to take screenshot with UE screenshot"));
            }
        }
        else {
            UE_LOG(LogBlackBox, Warning, TEXT("Capture screenshot failed - Issue Report Directory does not exist"));
        }
    };
#else
    UE_LOG(LogBlackBox, Warning, TEXT("Cannot capture screenshot - Platform not supported"))
#endif
}

void FAccelByteBlackBoxSDKModule::DeleteAdditionalInfoField(FString FieldName)
{
#if BLACKBOX_SDK_USE_ADDITIONAL_KEY_VALUE_DATA_GATHERING
    if (!IsModuleLibraryLoaded()) {
        return;
    }
    bbx_delete_additional_info_field(TCHAR_TO_UTF8(*FieldName));
#endif
}

void FAccelByteBlackBoxSDKModule::EmptyAdditionalInfo()
{
#if BLACKBOX_SDK_USE_ADDITIONAL_KEY_VALUE_DATA_GATHERING
    if (!IsModuleLibraryLoaded()) {
        return;
    }
    bbx_empty_additional_info();
#endif
}

bool FAccelByteBlackBoxSDKModule::UpdateAdditionalInfo(FString FieldName, FString Value)
{
#if BLACKBOX_SDK_USE_ADDITIONAL_KEY_VALUE_DATA_GATHERING
    if (!IsModuleLibraryLoaded()) {
        return false;
    }
    bbx_update_additional_info(TCHAR_TO_UTF8(*FieldName), TCHAR_TO_UTF8(*Value));
    return true;
#else
    return false;
#endif
}

FString FAccelByteBlackBoxSDKModule::GetAdditionalInfoValue(FString FieldName)
{
#if BLACKBOX_SDK_USE_ADDITIONAL_KEY_VALUE_DATA_GATHERING
    if (!IsModuleLibraryLoaded()) {
        return FString();
    }
    std::string Value = bbx_get_additional_info_value(TCHAR_TO_UTF8(*FieldName));
    FString Out = UTF8_TO_TCHAR(Value.c_str());
    return Out;
#else
    return FString();
#endif
}

void FAccelByteBlackBoxSDKModule::RegisterViewportResizedCallback()
{
#if BLACKBOX_UE_WINDOWS
    if (GEngine) {
        OnViewportResizedHandle =
            FViewport::ViewportResizedEvent.AddRaw(this, &FAccelByteBlackBoxSDKModule::ViewportResized);
    }
#endif
}

void FAccelByteBlackBoxSDKModule::UnregisterViewportResizedCallback()
{
#if BLACKBOX_UE_WINDOWS
    if (OnViewportResizedHandle.IsValid()) {
        FViewport::ViewportResizedEvent.Remove(OnViewportResizedHandle);
    }
#endif
}

void FAccelByteBlackBoxSDKModule::ViewportResized(FViewport*, uint32)
{
#if BLACKBOX_UE_WINDOWS
    bbx_notify_change_game_resolution();
#endif
}

FString FAccelByteBlackBoxSDKModule::GetCrashGUID()
{
#if BLACKBOX_UE_WINDOWS
    auto CrashConfigPath = FPlatformCrashContext::GetCrashConfigFilePath();
    auto ParentPath = FPaths::GetPath(CrashConfigPath);
    return FPaths::GetPathLeaf(ParentPath);
#else
    return FString();
#endif
}

void FAccelByteBlackBoxSDKModule::CreateMatch(
    const FString& PlatformMatchID,
    const FString& PlatformMatchIDType,
    const FOnBlackBoxMatchIDRetrievedDelegate& Callback)
{
#if BLACKBOX_SDK_USE_ADT_SESSION_GROUPING
    if (!IsModuleLibraryLoaded()) {
        Callback.ExecuteIfBound(false, "DLL Library is not loaded.", "");
        return;
    }

    OnBlackBoxMatchIDRetrieved = Callback;

    bbx_create_match(
        TCHAR_TO_UTF8(*PlatformMatchID), TCHAR_TO_UTF8(*PlatformMatchIDType), &BlackboxSDK::MatchCreatedCallback);
#endif
}

void FAccelByteBlackBoxSDKModule::BeginMatchSession(
    const FString& BlackBoxMatchID, const FOnBlackBoxMatchSessionStartedDelegate& Callback)
{
#if BLACKBOX_SDK_USE_ADT_SESSION_GROUPING
    if (!IsModuleLibraryLoaded()) {
        Callback.ExecuteIfBound(false, "DLL Library is not loaded.");
        return;
    }

    OnBlackBoxMatchSessionStarted = Callback;

    bbx_begin_match_session(TCHAR_TO_UTF8(*BlackBoxMatchID), &BlackboxSDK::MatchSessionStartedCallback);
#endif
}

void FAccelByteBlackBoxSDKModule::EndMatchSession(
    const FString& BlackBoxMatchID, const FOnBlackBoxMatchSessionStartedDelegate& Callback)
{
#if BLACKBOX_SDK_USE_ADT_SESSION_GROUPING
    if (!IsModuleLibraryLoaded()) {
        Callback.ExecuteIfBound(false, "DLL Library is not loaded.");
        return;
    }

    OnBlackBoxMatchSessionEnded = Callback;

    bbx_end_match_session(TCHAR_TO_UTF8(*BlackBoxMatchID), &BlackboxSDK::MatchSessionEndedCallback);
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAccelByteBlackBoxSDKModule, BlackBoxSDK)