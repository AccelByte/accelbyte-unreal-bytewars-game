// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/AccelByteEnvironment.h"
#include "Core/UI/Components/Prompt/FTUE/FTUEModels.h"
#include "Core/UI/Components/WidgetValidator/WidgetValidatorModels.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "TutorialModuleOnlineUtility.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteWarsTutorialModuleOnlineUtility, Log, All);
#define UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAccelByteWarsTutorialModuleOnlineUtility, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}

#define BYTEWARS_LOCTEXT_NAMESPACE "AccelByteWars"
#define DEFAULT_USER_DISPLAYNAME NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Player-{0}", "Player-{0}")
#define UNKNOWN_USER_DISPLAYNAME NSLOCTEXT(BYTEWARS_LOCTEXT_NAMESPACE, "Unknown Player", "Unknown")

UCLASS()
class ACCELBYTEWARS_API UTutorialModuleOnlineUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UTutorialModuleOnlineUtility();

	UFUNCTION(BlueprintCallable, Category = "Tutorial Module Online Utility", meta = (WorldContext = "WorldContextObject"))
	static bool IsAccelByteSDKInitialized(const UObject* Target);

	/* Get user default display name. 
	 * Return first 5 character of the AccelByte Id with "Player" prefix. Example: Player-a523f
	 * When fails to get the AccelByte Id, returns "Unknown" instead. */
	static const FString GetUserDefaultDisplayName(const FUniqueNetId& UserId)
	{
		const FUniqueNetIdAccelByteUserRef UserABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UserId.AsShared());
		if (!UserABId->IsValid())
		{
			return UNKNOWN_USER_DISPLAYNAME.ToString();
		}

		return FText::Format(DEFAULT_USER_DISPLAYNAME, FText::FromString(UserABId->GetAccelByteId().Left(5))).ToString();
	}

	static const FString GetUserDefaultDisplayName(const FString& UserId)
	{
		return FText::Format(DEFAULT_USER_DISPLAYNAME, FText::FromString(UserId.Left(5))).ToString();
	}

	/* @brief Get dedicated server version override. Return empty if not overridden. */
	static FString GetDedicatedServerVersionOverride();

	/* @brief Get if the server use AccelByte Multiplayer Server. */
	static bool GetIsServerUseAMS();

	static void OverrideSDKConfig(const TMap<FString, FString>& ClientConfigs, const TMap<FString, FString>& ServerConfigs);

	static bool IsUseAGSStarter();

	static FString GetPrimaryLanguageSubtag();

private:
	static void CheckForDedicatedServerVersionOverride();

	static TMap<FString, FString> CheckForSDKConfigOverride(const bool bIsServer);

	static void CheckForEnvironmentConfigOverride();
	static ESettingsEnvironment ConvertStringEnvToAccelByteEnv(const FString& EnvironmentStr);
	static FString ConvertAccelByteEnvToStringEnv(const ESettingsEnvironment& Environment);
	static ESettingsEnvironment ConvertOSSEnvToAccelByteEnv(const EOnlineEnvironment::Type& Environment);

	void ExecutePredefinedServiceValidator(const EServicePredefinedValidator ValidatorType, const TDelegate<void(const bool /*bIsValid*/)>& OnComplete, const UObject* Context);
	
	void ExecutePredefinedServiceForFTUE(FFTUEDialogueModel* Dialogue, const FOnFTUEDialogueValidationComplete& OnComplete, const UObject* Context);
	void ExecutePredefinedServiceForWidgetValidator(FWidgetValidator* WidgetValidator, const UObject* Context);

	FString GetServicePredefinedArgument(const EServicePredifinedArgument Keyword);

	static FNamedOnlineSession* GetOnlineSession(const FName SessionName, const UObject* Context);
	static FAccelByteModelsV2GameSessionDSInformation GetDedicatedServer(const UObject* Context);

	static void CheckUseAGSStarter();
	
	void CacheGeneralInformation(const APlayerController* PC);

	inline static FString CurrentPlayerUserIdStr = TEXT("");
	inline static FString CurrentPlayerDisplayName = TEXT("");
	inline static FString CurrentPublishedStoreId = TEXT("");
	inline static FString DedicatedServerVersionOverride = TEXT("");

	inline static bool bUseAGSStarter = false;
};