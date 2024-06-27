// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/System/AccelByteWarsGameInstance.h"

#include "Core/AssetManager/AccelByteWarsAssetManager.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/UI/GameUIManagerSubsystem.h"
#include "Core/Player/CommonLocalPlayer.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "GameFramework/OnlineSession.h"

#include "Core/UI/Components/Prompt/FTUE/FTUEDialogueWidget.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"

#define GAMEINSTANCE_LOG(FormatString, ...) UE_LOG(LogAccelByteWarsGameInstance, Log, TEXT(FormatString), __VA_ARGS__);

void UAccelByteWarsGameInstance::Init()
{
	Super::Init();

	GEngine->NetworkFailureEvent.AddUObject(this, &ThisClass::OnNetworkFailure);

	// Command to crash the game. Used to test ADT crash report
	IConsoleManager::Get().RegisterConsoleCommand(TEXT("TriggerCrash"), TEXT("Crash current game client"), FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
	{
		UE_LOG(LogAccelByteWarsGameInstance, Warning, TEXT("Intended crash"))

		// Simulating crash by accessing a null pointer.
		const APlayerController* CrashPC = nullptr;
		CrashPC->GetLocalPlayer()->GetPreferredUniqueNetId();
	}));
}

void UAccelByteWarsGameInstance::Shutdown()
{
	OnGameInstanceShutdownDelegate.Broadcast();

	Super::Shutdown();
}

UAccelByteWarsBaseUI* UAccelByteWarsGameInstance::GetBaseUIWidget(bool bAutoActivate)
{
	if (IsRunningDedicatedServer())
	{
		return nullptr;
	}

	if (!BaseUIWidget)
	{
		BaseUIWidget = Cast<UAccelByteWarsBaseUI>(CreateWidget(this, BaseUIMenuWidgetClass));
	}

	if (bAutoActivate && !BaseUIWidget->IsInViewport())
	{
		BaseUIWidget->AddToViewport(10);
		BaseUIWidget->ResetWidget();
	}

	return BaseUIWidget;
}

void UAccelByteWarsGameInstance::OnDelayedClientTravelStarted()
{
	OnDelayedClientTravelStartedDelegates.Broadcast();

	BaseUIWidget->ClearWidgets();
}

bool UAccelByteWarsGameInstance::GetIsPendingFailureNotification(ENetworkFailure::Type& OutFailureType)
{
	if (bPendingFailureNotification)
	{
		OutFailureType = LastFailureType;
		bPendingFailureNotification = false;
		return true;
	}

	return false;
}

void UAccelByteWarsGameInstance::OnNetworkFailure(
	UWorld* World,
	UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType,
	const FString& Message)
{
	// Only change the FailureType if the Message is different
	if (!LastFailureMessage.Equals(Message))
	{
		const AAccelByteWarsInGameGameState* GameState = Cast<AAccelByteWarsInGameGameState>(GetWorld()->GetGameState());
		if(GameState && GameState->GameStatus == EGameStatus::GAME_ENDS)
		{
			// ignore network error if already at GAME_ENDS state, as it only wait to back to main menu
			return;
		}
		LastFailureMessage = Message;
		bPendingFailureNotification = true;
		LastFailureType = FailureType;
	}
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
	const TSubclassOf<UOnlineSession> OnlineSessionClass =
		UAccelByteWarsAssetManager::Get().GetPreferredOnlineSessionClassFromDataAsset();

	return OnlineSessionClass ? OnlineSessionClass : Super::GetOnlineSessionClass();
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

void UAccelByteWarsGameInstance::OpenSDKConfigMenu()
{
	// Abort if the SDK config menu is already opened.
	if (SDKConfigWidgetInstance && SDKConfigWidgetInstance->IsActivated()) 
	{
		return;
	}

	// Open the SDK config menu.
	if (SDKConfigWidgetClass && SDKConfigWidgetClass.Get())
	{
		SDKConfigWidgetInstance = GetBaseUIWidget()->PushWidgetToStack(EBaseUIStackType::Prompt, SDKConfigWidgetClass.Get());
	}
}

void UAccelByteWarsGameInstance::OpenFTUEMenu()
{
	UFTUEDialogueWidget* FTUEWidget = BaseUIWidget->GetFTUEDialogueWidget();

	if (FTUEWidget && FTUEWidget->IsActivated())
	{
		return;
	}

	if (FTUEWidgetClass && FTUEWidgetClass.Get())
	{
		GetBaseUIWidget()->SetFTUEDialogueWidget(nullptr);
		UCommonActivatableWidget* HUDWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::InGameHUD, this);
		UAccelByteWarsActivatableWidget* HUDWidgetAB = Cast<UAccelByteWarsActivatableWidget>(HUDWidget);
		HUDWidgetAB->OpenFTUEWidget();
	}
}