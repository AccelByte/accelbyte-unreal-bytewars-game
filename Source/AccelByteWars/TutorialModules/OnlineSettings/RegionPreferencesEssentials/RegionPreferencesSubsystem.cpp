// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "RegionPreferencesSubsystem.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemUtils.h"

#define RETRY_LIMIT 3

void URegionPreferencesSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return;
	}

	GetIngameLatencyRefreshConfiguration();

	OnGetLatenciesSuccessDelegate = THandler<TArray<TPair<FString, float>>>::CreateUObject(this, &ThisClass::OnGetLatenciesSuccess);
	if (FOnlineIdentityAccelBytePtr IdentityInterface = GetIdentityInterface())
	{
		IdentityInterface->AddOnConnectLobbyCompleteDelegate_Handle(0, FOnConnectLobbyCompleteDelegate::CreateUObject(this, &ThisClass::OnLobbyConnected));
	}
#endif
}

void URegionPreferencesSubsystem::Deinitialize()
{
	Super::Deinitialize();

#if UE_EDITOR || !UE_SERVER
	if (IsRunningDedicatedServer())
	{
		return;
	}

	OnGetLatenciesSuccessDelegate.Unbind();
	if (FOnlineIdentityAccelBytePtr IdentityInterface = GetIdentityInterface())
	{
		IdentityInterface->ClearOnConnectLobbyCompleteDelegates(0, this);
	}
#endif
}

void URegionPreferencesSubsystem::RefreshRegionLatency(bool bRetryOnFailed)
{
#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return;
	}
	
	AccelByte::FApiClientPtr ApiClient = UTutorialModuleOnlineUtility::GetApiClient(this);
	if (!ApiClient)
	{
		UE_LOG_REGION_PREFERENCES_ESSENTIALS(Warning, TEXT("Cannot get region latencies. AccelByte API Client is invalid."));
		return;
	}

	ApiClient->Qos.GetServerLatencies(OnGetLatenciesSuccessDelegate, FErrorHandler::CreateWeakLambda(this, [this, bRetryOnFailed](int32 ErrorCode, const FString& ErrorMessage)
	{
		UE_LOG_REGION_PREFERENCES_ESSENTIALS(Warning, TEXT("Error getting regions latency!! Error code: %d Error Message: %s"), ErrorCode, *ErrorMessage);
		OnGetLatenciesError(ErrorCode, ErrorMessage);
		if(bRetryOnFailed && NumRetry < RETRY_LIMIT)
		{
			UE_LOG_REGION_PREFERENCES_ESSENTIALS(Log, TEXT("Retry getting regions latency, retry num = %d"), NumRetry);
			NumRetry += 1;
			RefreshRegionLatency(bRetryOnFailed);
		}
		else
		{
			NumRetry = 0;
		}
	}));
#endif
}

TArray<FString> URegionPreferencesSubsystem::GetEnabledRegion()
{
	TArray<FString> EnabledRegion = {};
	
#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return EnabledRegion;
	}
	
	for(const URegionPreferenceInfo* Info : RegionInfos)
	{
		if(Info->bEnabled)
		{
			EnabledRegion.Add(Info->RegionCode);
		}
	}
#endif
	
	return EnabledRegion;
}

int32 URegionPreferencesSubsystem::GetEnabledRegionCount()
{
	int32 Result = 0;

#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return Result;
	}
	
	for(const URegionPreferenceInfo* Info : RegionInfos)
	{
		if(Info->bEnabled)
		{
			Result+=1;
		}
	}
#endif
	
	return Result;
}

bool URegionPreferencesSubsystem::TryToggleRegion(const FString& RegionCode)
{
	URegionPreferenceInfo* RegionInfo = FindRegionInfo(RegionCode);

#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return false;
	}
	
	if(RegionInfo != nullptr)
	{
		// toggle region will fail if user try to disable the last remaining enabled in region
		if(RegionInfo->bEnabled && GetEnabledRegionCount() <= 1)
		{
			OnWarningMinimumRegionCount.Broadcast();
		}
		else
		{
			RegionInfo->bEnabled = !RegionInfo->bEnabled;
			return true;
		}
	}
#endif
	
	return false;
}

URegionPreferenceInfo* URegionPreferencesSubsystem::FindRegionInfo(const FString& RegionCode)
{
#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return nullptr;
	}
	
	URegionPreferenceInfo** RegionInfo = RegionInfos.FindByPredicate([&RegionCode](const URegionPreferenceInfo* Info)
	{
		return Info->RegionCode.Equals(RegionCode);
	});

	if(RegionInfo != nullptr)
	{
		return *RegionInfo;
	}
#endif
	
	return nullptr;
}

FString URegionPreferencesSubsystem::GetCurrentGameSessionRegion()
{
	FString CurrentGameSessionRegion = TEXT("");

#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return CurrentGameSessionRegion;
	}
	
	FAccelByteModelsV2GameSessionDSInformation DSInformation = UTutorialModuleOnlineUtility::GetDedicatedServer(this);
	CurrentGameSessionRegion = DSInformation.Server.Region;
#endif

	return CurrentGameSessionRegion;
}

bool URegionPreferencesSubsystem::ShouldShowLatencyInGame()
{
	bool bResult = false;

#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return bResult;
	}
	
	if(!bShowLatency)
	{
		return false;
	}
	
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem)
	{
		return false;
	}

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (!SessionInterface)
	{
		return false;
	}
	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if(Session == nullptr)
	{
		return false;
	}
	FString ServerType = TEXT("");
	Session->SessionSettings.Get(SETTING_SESSION_SERVER_TYPE, ServerType);
	
	bResult = ServerType.ToUpper().Equals("DS");
#endif

	return bResult;
}

void URegionPreferencesSubsystem::StartLatencyRefreshTimer()
{
#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return;
	}
	
	GetWorld()->GetTimerManager().SetTimer(LatencyRefreshTimerHandle, this, &ThisClass::RefreshGameLatency, LatencyRefreshInterval, true, 1);
#endif
}

void URegionPreferencesSubsystem::StopLatencyRefreshTimer()
{
#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return;
	}
	
	GetWorld()->GetTimerManager().ClearTimer(LatencyRefreshTimerHandle);
#endif
}

void URegionPreferencesSubsystem::RefreshGameLatency() const
{
#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return;
	}
	
	const APlayerController* LocalPlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (!LocalPlayerController)
	{
		UE_LOG_REGION_PREFERENCES_ESSENTIALS(Warning, TEXT("Unable to get current logged in player's User Id. PlayerController is invalid."));
		return;
	}

	AAccelByteWarsPlayerState* PlayerState = LocalPlayerController->GetPlayerState<AAccelByteWarsPlayerState>();

	if(PlayerState)
	{
		const float Latency = PlayerState->GetPingInMilliseconds();
		UE_LOG_REGION_PREFERENCES_ESSENTIALS(Log, TEXT("Current latency is %f"),Latency);
		OnLatencyRefreshComplete.Broadcast(Latency);
	}
	else
	{
		UE_LOG_REGION_PREFERENCES_ESSENTIALS(Warning, TEXT("Unable to get latency, player state is empty!!"));
	}
#endif
}

void URegionPreferencesSubsystem::FilterSessionSearch(TSharedRef<FOnlineSessionSearch> SessionSearch)
{
#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return;
	}
	
	SessionSearch->SearchResults.RemoveAll([this](const FOnlineSessionSearchResult& Element)
	{
		FString ServerType = TEXT("");
		Element.Session.SessionSettings.Get(SETTING_SESSION_SERVER_TYPE, ServerType);
		if(ServerType.ToUpper().Equals("DS"))
		{
			TSharedPtr<FOnlineSessionInfoAccelByteV2> ABSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Element.Session.SessionInfo);
			if(ABSessionInfo->IsValid())
			{
				TSharedPtr<FAccelByteModelsV2GameSession> GameBackendSessionData = ABSessionInfo->GetBackendSessionDataAsGameSession();
				if(GameBackendSessionData.IsValid())
				{
					if(RegionInfos.IsEmpty())
					{
						return false;
					}
					return RegionInfos.ContainsByPredicate([GameBackendSessionData](URegionPreferenceInfo* Info)
					{
						return Info->RegionCode.Equals(GameBackendSessionData->DSInformation.Server.Region) && !Info->bEnabled;
					});
				}
				else
				{
					UE_LOG_REGION_PREFERENCES_ESSENTIALS(Warning, TEXT("Failed to check session's ds region. Game backend session data is empty!!"));
				}
			}
			else
			{
				UE_LOG_REGION_PREFERENCES_ESSENTIALS(Warning, TEXT("Failed to check session's ds region. Not a valid session!!"));
			}
		}
		return false;
	});
#endif
}

#if UE_EDITOR || !UE_SERVER
TArray<UTutorialModuleSubsystem::FCheatCommandEntry> URegionPreferencesSubsystem::GetCheatCommandEntries()
{
	TArray<FCheatCommandEntry> OutArray = {};

	// Get self user info
	OutArray.Add(FCheatCommandEntry(
		*CommandMyUserInfo,
		TEXT("Show logged in user info. Optional param: local user index"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &ThisClass::CheatRefreshGameLatency)));

	return OutArray;
}

FOnlineIdentityAccelBytePtr URegionPreferencesSubsystem::GetIdentityInterface() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_REGION_PREFERENCES_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
}

void URegionPreferencesSubsystem::GetIngameLatencyRefreshConfiguration()
{
	const FString CmdArgs = FCommandLine::Get();
	bool bValidCmdValue = false;

	// Check show latency configuration
	const FString ShowLatencyCmdStr = FString("-ShowLatency=");
	if (CmdArgs.Contains(ShowLatencyCmdStr, ESearchCase::IgnoreCase))
	{
		FString CmdValue;
		FParse::Value(*CmdArgs, *ShowLatencyCmdStr, CmdValue);
		if (!CmdValue.IsEmpty())
		{
			bShowLatency = CmdValue.Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);
			bValidCmdValue = true;
			UE_LOG_REGION_PREFERENCES_ESSENTIALS(Log, TEXT("Launch param sets the show latency to %s."), bShowLatency ? TEXT("TRUE") : TEXT("FALSE"));
		}
	}

	if (!bValidCmdValue)
	{
		GConfig->GetBool(TEXT("AccelByteTutorialModules"), TEXT("bShowLatency"), bShowLatency, GEngineIni);
		UE_LOG_REGION_PREFERENCES_ESSENTIALS(Log, TEXT("DefaultEngine.ini sets the show latency to %s."), bShowLatency ? TEXT("TRUE") : TEXT("FALSE"));
	}
	
	// Check latency refresh interval configuration
	bValidCmdValue = false;
	const FString LatencyRefreshIntervalCmdStr = FString("-LatencyRefreshInterval=");
	if (CmdArgs.Contains(LatencyRefreshIntervalCmdStr, ESearchCase::IgnoreCase))
	{
		FString CmdValue;
		FParse::Value(*CmdArgs, *LatencyRefreshIntervalCmdStr, CmdValue);
		if (!CmdValue.IsEmpty() && CmdValue.IsNumeric())
		{
			LatencyRefreshInterval = FCString::Atoi(*CmdValue);
			bValidCmdValue = true;
			UE_LOG_REGION_PREFERENCES_ESSENTIALS(Log, TEXT("Launch param sets the latency refresh interval to %d."), LatencyRefreshInterval);
		}
	}

	if (!bValidCmdValue)
	{
		GConfig->GetBool(TEXT("AccelByteTutorialModules"), TEXT("bShowLatency"), bShowLatency, GEngineIni);
		UE_LOG_REGION_PREFERENCES_ESSENTIALS(Log, TEXT("DefaultEngine.ini sets the latency refresh interval to %d."), LatencyRefreshInterval);
	}
}

void URegionPreferencesSubsystem::OnLobbyConnected(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	AccelByte::FApiClientPtr ApiClient = UTutorialModuleOnlineUtility::GetApiClient(this);
	if (!ApiClient)
	{
		UE_LOG_REGION_PREFERENCES_ESSENTIALS(Warning, TEXT("Cannot initialze region latencies. AccelByte API Client is invalid."));
		return;
	}

	// Initialize region latencies from cache if any. Otherwise, query the region latencies.
	const TArray<TPair<FString, float>> Latencies = ApiClient->Qos.GetCachedLatencies();
	if (Latencies.Num() > 0)
	{
		OnGetLatenciesSuccess(Latencies);
	}
	else
	{
		RefreshRegionLatency(true);
	}
}

void URegionPreferencesSubsystem::OnGetLatenciesSuccess(const TArray<TPair<FString, float>>& InLatencies)
{
	UE_LOG_REGION_PREFERENCES_ESSENTIALS(Log, TEXT("Success getting regions latency"));;
	// remove regions that no longer exist
	RegionInfos.RemoveAll([&InLatencies](const URegionPreferenceInfo* Info)
	{
		const TPair<FString, float>* Latency = InLatencies.FindByPredicate([&Info](const TPair<FString, float>& LatencyItem)
		{
			return LatencyItem.Key.Equals(Info->RegionCode);
		});

		return Latency == nullptr;
	});

	// update existing region info and add new one (if exist)
	for(const TPair<FString, float>& Latency : InLatencies)
	{
		URegionPreferenceInfo* RegionInfo = FindRegionInfo(Latency.Key);
		
		if(RegionInfo != nullptr)
		{
			RegionInfo->Latency = Latency.Value;
		}
		else
		{
			URegionPreferenceInfo* RegionPreferenceInfo = NewObject<URegionPreferenceInfo>();
			RegionPreferenceInfo->RegionCode = Latency.Key;
			RegionPreferenceInfo->Latency = Latency.Value;
			RegionInfos.Add(RegionPreferenceInfo);
		}
	}
	
	NumRetry = 0;
	OnRefreshRegionComplete.Broadcast(true);
}

void URegionPreferencesSubsystem::OnGetLatenciesError(int32 ErrorCode, const FString& ErrorMessage)
{
	RegionInfos.Empty();
	OnRefreshRegionComplete.Broadcast(false);
}
#endif

void URegionPreferencesSubsystem::CheatRefreshGameLatency(const TArray<FString>& Args) const
{
#if UE_EDITOR || !UE_SERVER
	if(IsRunningDedicatedServer())
	{
		return;
	}
	
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!Subsystem)
	{
		UE_LOG_REGION_PREFERENCES_ESSENTIALS(Warning, TEXT("Cheat refresh latency failed, online subsystem is empty"));
		return;
	}

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
	if (!SessionInterface)
	{
		UE_LOG_REGION_PREFERENCES_ESSENTIALS(Warning, TEXT("Cheat refresh latency failed, session interface is empty!!"));
		return;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if(Session == nullptr)
	{
		UE_LOG_REGION_PREFERENCES_ESSENTIALS(Warning, TEXT("Cheat refresh latency failed, player is not in a game session!!"));
		return;
	}

	RefreshGameLatency();
#endif
}
