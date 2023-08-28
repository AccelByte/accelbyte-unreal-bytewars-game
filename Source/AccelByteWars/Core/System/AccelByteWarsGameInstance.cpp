// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/System/AccelByteWarsGameInstance.h"

#include "Core/AssetManager/AccelByteWarsAssetManager.h"
#include "Core/UI/GameUIManagerSubsystem.h"
#include "Core/Player/CommonLocalPlayer.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "GameFramework/OnlineSession.h"

#define GAMEINSTANCE_LOG(FormatString, ...) UE_LOG(LogAccelByteWarsGameInstance, Log, TEXT(FormatString), __VA_ARGS__);

void UAccelByteWarsGameInstance::Shutdown()
{
	OnGameInstanceShutdownDelegate.Broadcast();

	Super::Shutdown();
}

UAccelByteWarsBaseUI* UAccelByteWarsGameInstance::GetBaseUIWidget()
{
	if (IsRunningDedicatedServer())
	{
		return nullptr;
	}

	if (!BaseUIWidget)
	{
		BaseUIWidget = Cast<UAccelByteWarsBaseUI>(CreateWidget(this, BaseUIMenuWidgetClass));
	}

	if (!BaseUIWidget->IsActivated())
	{
		BaseUIWidget->ActivateWidget();
	}

	if (!BaseUIWidget->IsInViewport() && !bHasAddToViewportCalled)
	{
		/**
		 * somwhow, when this is called multiple times before the BaseUIWidget finish constructing, 
		 * IsInViewport will always return false, but the AddToViewport will be still be called that many times.
		 * Resulting in the widget to be added multiple times to viewport despite using the IsInViewport flag.
		 * Hence why there's this custom flag.
		 */
		bHasAddToViewportCalled = true;
		BaseUIWidget->OnDeactivated().AddWeakLambda(this, [this]()
		{
			bHasAddToViewportCalled = false;
		});

		BaseUIWidget->AddToViewport(10);
	}

	return BaseUIWidget;
}

int32 UAccelByteWarsGameInstance::AddLocalPlayer(ULocalPlayer* NewLocalPlayer, FPlatformUserId UserId)
{
	const int32 ReturnVal = Super::AddLocalPlayer(NewLocalPlayer, UserId);
	if (ReturnVal != INDEX_NONE)
	{
		if (!PrimaryPlayer.IsValid())
		{
			GAMEINSTANCE_LOG("AddLocalPlayer: Set %s to Primary Player", *NewLocalPlayer->GetName());
			PrimaryPlayer = NewLocalPlayer;
		}

		GAMEINSTANCE_LOG("AddLocalPlayer: New player %s is set to ControllerId: %i", *NewLocalPlayer->GetName(), UserId.GetInternalId());
		GetSubsystem<UGameUIManagerSubsystem>()->NotifyPlayerAdded(Cast<UCommonLocalPlayer>(NewLocalPlayer));

		if(OnLocalPlayerAdded.IsBound())
		{
			OnLocalPlayerAdded.Broadcast(NewLocalPlayer);
		}
	}
	
	return ReturnVal;
}

bool UAccelByteWarsGameInstance::RemoveLocalPlayer(ULocalPlayer* ExistingPlayer)
{
	if (PrimaryPlayer == ExistingPlayer)
	{
		//TODO: do we want to fall back to another player?
		PrimaryPlayer.Reset();
		GAMEINSTANCE_LOG("RemoveLocalPlayer: Unsetting Primary Player from %s", *ExistingPlayer->GetName());
	}
	GetSubsystem<UGameUIManagerSubsystem>()->NotifyPlayerDestroyed(Cast<UCommonLocalPlayer>(ExistingPlayer));

	if(OnLocalPlayerRemoved.IsBound())
	{
		OnLocalPlayerRemoved.Broadcast(ExistingPlayer);
	}
	
	return Super::RemoveLocalPlayer(ExistingPlayer);
}

FLinearColor UAccelByteWarsGameInstance::GetTeamColor(uint8 TeamId) const
{
	if (const uint8 GlobalTeamSetupNum = GlobalSettingsDataAsset->GlobalTeamSetup.Num(); TeamId >= GlobalTeamSetupNum)
	{
		// fallback: use modulo
		TeamId %= GlobalTeamSetupNum;
	}
	return GlobalSettingsDataAsset->GlobalTeamSetup[TeamId].itemColor;
}

TSubclassOf<UOnlineSession> UAccelByteWarsGameInstance::GetOnlineSessionClass()
{
	TSubclassOf<UOnlineSession> OnlineSessionClass = nullptr;

	bool bUseCompletedOnlineSession = false;
	const FString OnlineSessionCompleteCodeName = "ONLINESESSIONCOMPLETE";
	TArray<FTutorialModuleData> TutorialModules = UAccelByteWarsAssetManager::GetAllTutorialModules();

	for (const FTutorialModuleData& TutorialModule : TutorialModules)
	{
		if (TutorialModule.CodeName.Equals(OnlineSessionCompleteCodeName))
		{
			continue;
		}

		if (TutorialModule.bIsActive && TutorialModule.bOnlineSessionModule)
		{
			/**
			 * Online Session module behaviour:
			 *
			 * if one or more of the Online Session module has starter module activated:
			 * Use the Online Session from the first module found.
			 * If more than one, show notification that the game will only use the first one.
			 * 
			 * else if none of the Online Session module has starter mode activated:
			 * if 1 Online Session module found, use its Online Session.
			 * If more than one Online Session module active, use the complete OnlineSession.
			 */
			if (TutorialModule.bIsStarterModeActive)
			{
				if (TutorialModule.OnlineSessionClass)
				{
					OnlineSessionClass = TutorialModule.OnlineSessionClass;
					break;
				}
			}
			else
			{
				if (TutorialModule.OnlineSessionClass)
				{
					if (OnlineSessionClass == nullptr)
					{
						OnlineSessionClass = TutorialModule.OnlineSessionClass;
					}
					else
					{
						bUseCompletedOnlineSession = true;
						break;
					}
				}
			}
		}
	}

	if (bUseCompletedOnlineSession)
	{
		// re iterate in case the completed module is at the earlier index
		for (const FTutorialModuleData& TutorialModule : TutorialModules)
		{
			if (TutorialModule.CodeName.Equals(OnlineSessionCompleteCodeName))
			{
				OnlineSessionClass = TutorialModule.OnlineSessionClass;
			}
		}
	}

	OnlineSessionClass = OnlineSessionClass == nullptr ? Super::GetOnlineSessionClass() : OnlineSessionClass;

	GAMEINSTANCE_LOG("OnlineSession used: %s", *OnlineSessionClass->GetPathName());
	return OnlineSessionClass;
}

FGameModeData UAccelByteWarsGameInstance::GetGameModeDataByCodeName(const FString CodeName) const
{
	FGameModeData Data;
	if (ensure(GameModeDataTable))
	{
		TArray<FGameModeData*> GameModeDatas;
		GameModeDataTable->GetAllRows("GetGameModeDataByCodeName", GameModeDatas);

		if (GameModeDatas.Num() > 0)
		{
			FGameModeData** DataPtr = GameModeDatas.FindByPredicate([CodeName](const FGameModeData* GameModeData)
			{
				return GameModeData->CodeName.Equals(CodeName);
			});

			// if not found, use the first entry
			Data = DataPtr ? **DataPtr : *GameModeDatas[0];
		}
	}
	return Data;
}
