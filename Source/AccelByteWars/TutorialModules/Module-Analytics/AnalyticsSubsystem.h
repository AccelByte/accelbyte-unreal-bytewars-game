// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "AnalyticsSubsystem.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAnalyticsEssentials, Log, All);

#define UE_LOG_ANALYTICS(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAnalyticsEssentials, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}

UCLASS()
class ACCELBYTEWARS_API UAnalyticsSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	/**
	 * @brief Send telemetry. The telemetry will automatically be send as server telemetry or client telemetry.
	 * @param EventName The telemetry event name.
	 * @param Payload The telemetry data to send.
	 * @param bIsImmediateEvent Whether the telemetry should be send immediately or not.
	 */
	void SendTelemetry(const FString& EventName, const TSharedPtr<FJsonObject>& Payload, const bool bIsImmediateEvent = false);

	/**
	 * @brief Send telemetry (Game Client).
	 * @param EventName The telemetry event name.
	 * @param Payload The telemetry data to send.
	 * @param bIsImmediateEvent Whether the telemetry should be send immediately or not.
	 */
	void SendTelemetryClient(const FString& EventName, const TSharedPtr<FJsonObject>& Payload, const bool bIsImmediateEvent = false);

	/**
	 * @brief Send telemetry (Server).
	 * @param EventName The telemetry event name.
	 * @param Payload The telemetry data to send.
	 * @param bIsImmediateEvent Whether the telemetry should be send immediately or not.
	 */
	void SendTelemetryServer(const FString& EventName, const TSharedPtr<FJsonObject>& Payload, const bool bIsImmediateEvent = false);

protected:
	const FString TelemetryGameNamespace = FString("bytewars");
	TArray<FString> ClientImmediateTelemetryEventList;
	TArray<FString> ServerImmediateTelemetryEventList;
};
