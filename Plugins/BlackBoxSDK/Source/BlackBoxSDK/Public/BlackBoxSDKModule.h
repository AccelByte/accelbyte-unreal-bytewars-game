// Copyright (c) 2019 - 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Modules/ModuleManager.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(
    FOnBlackBoxMatchIDRetrieved, bool /*IsSuccessful*/, const FString& /*ErrorMessage*/, const FString& /*MatchID*/);
using FOnBlackBoxMatchIDRetrievedDelegate = FOnBlackBoxMatchIDRetrieved::FDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(
    FOnBlackBoxMatchSessionStarted, bool /*IsSuccessful*/, const FString& /*ErrorMessage*/);
using FOnBlackBoxMatchSessionStartedDelegate = FOnBlackBoxMatchSessionStarted::FDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(
    FOnBlackBoxMatchSessionEnded, bool /*IsSuccessful*/, const FString& /*ErrorMessage*/);
using FOnBlackBoxMatchSessionEndedDelegate = FOnBlackBoxMatchSessionEnded::FDelegate;

class BLACKBOXSDK_API IAccelByteBlackBoxSDKModuleInterface : public IModuleInterface {
public:
    /**
     * @brief Get the BlackBox SDK, this is the entry point of the SDK
     *
     * @return IAccelByteBlackBoxSDKModuleInterface&
     */
    static IAccelByteBlackBoxSDKModuleInterface& Get()
    {
        return FModuleManager::LoadModuleChecked<IAccelByteBlackBoxSDKModuleInterface>("BlackBoxSDK");
    }

    /**
     * @brief Check if the SDK module has been correctly loaded
     *
     * @return true
     * @return false
     */
    static bool IsAvailable() { return FModuleManager::Get().IsModuleLoaded("BlackBoxSDK"); }

    /**
     * @brief Tick the SDK
     *
     * @param DeltaTime
     */
    virtual void Tick(float DeltaTime) = 0;

    /**
     * @brief Call this at the start of the game
     *
     */
    virtual void Start() = 0;

    /**
     * @brief Call this at the end of the game
     *
     */
    virtual void Stop() = 0;

    /**
     * @brief Call this on every tick of the game to record the keypresses
     *
     * @param PlayerController
     */
    virtual void FeedKeyboardInput(APlayerController* PlayerController) = 0;

    /**
     * @brief Delete a field in additional info
     *
     * @param FieldName
     */
    virtual void DeleteAdditionalInfoField(FString FieldName) = 0;

    /**
     * @brief Empty the additional info
     *
     */
    virtual void EmptyAdditionalInfo() = 0;

    /**
     * @brief Update / register a new additional info
     *
     * @param FieldName
     * @param Value
     */
    virtual bool UpdateAdditionalInfo(FString FieldName, FString Value) = 0;

    /**
     * @brief Get a registered additional info
     *
     * @param FieldName
     */
    virtual FString GetAdditionalInfoValue(FString FieldName) = 0;

    /**
     * @brief Set true to enable logs from the SDK
     *
     * @param Enable
     */
    virtual void EnableLog(bool Enable) = 0;

    /**
     * @brief Set level of severity of returned logs
     *
     * @param LogSeverity
     */
    virtual void SetLogCallbackSeverity(uint8 MaxLogSeverity) = 0;

    /**
     * @brief update existing session with external user id
     *
     * @param external user id
     */
    virtual void UpdateSessionWithExternalUserID(FString ExternalUserID) = 0;

    /**
     * @brief update existing session with external session id
     *
     * @param external session id
     */
    virtual void UpdateSessionWithExternalSessionID(FString ExternalSessionID) = 0;

    /**
     * @brief Create BlackBox Match for session grouping function
     * 
     * @param PlatformMatchID The match provider id, might be AccelByte Multiplayer Service Session Id, Steam lobby Id, etc
     * @param PlatformMatchIDType The match provider name, might be AccelByte Multiplayer Service (AMS), Steam, etc
     * @param Callback The callback to call when the operation is completed
     * 
     */
    virtual void CreateMatch(
        const FString& PlatformMatchID,
        const FString& PlatformMatchIDType,
        const FOnBlackBoxMatchIDRetrievedDelegate& Callback) = 0;

    /**
     * @brief Begin BlackBox Match Session for session grouping function
     * 
     * @param BlackBoxMatchID The BlackBox match id returned by the callback in the CreateMatch function
     * @param Callback The callback to call when the operation is completed
     */
    virtual void
    BeginMatchSession(const FString& BlackBoxMatchID, const FOnBlackBoxMatchSessionStartedDelegate& Callback) = 0;

    /**
     * @brief End BlackBox Match Session for session grouping function
     *
     * @param BlackBoxMatchID The BlackBox match id returned by the callback in the CreateMatch function
     * @param Callback The callback to call when the operation is completed
     */
    virtual void
    EndMatchSession(const FString& BlackBoxMatchID, const FOnBlackBoxMatchSessionEndedDelegate& Callback) = 0;
};
