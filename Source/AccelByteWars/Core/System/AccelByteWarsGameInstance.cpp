// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/System/AccelByteWarsGameInstance.h"

#include "Core/AssetManager/AccelByteWarsAssetManager.h"
#include "Core/AssetManager/InGameItems/InGameItemDataAsset.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/UI/GameUIManagerSubsystem.h"
#include "Core/Player/CommonLocalPlayer.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "GameFramework/OnlineSession.h"

#include "Core/UI/Components/Prompt/FTUE/FTUEDialogueWidget.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"

#pragma region "Lobby Connect/Disconnect using PS Controller"
#ifdef AGS_LOBBY_CHEAT_ENABLED
#include "TutorialModuleUtilities/StartupSubsystem.h"
#endif // AGS_LOBBY_CHEAT_ENABLED
#pragma endregion 

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

void UAccelByteWarsGameInstance::UpdateEquippedItems(const int32 PlayerIndex, const TArray<FEquippedItem>& InItems)
{
	if (TArray<FEquippedItem>* PlayerItems = EquippedItems.Find(PlayerIndex))
	{
		for (const FEquippedItem& InItem : InItems)
		{
			// overwrite item with same type
			bool bFound = false;
			for (FEquippedItem& PlayerItem : *PlayerItems)
			{
				if (PlayerItem.ItemType == InItem.ItemType)
				{
					PlayerItem.ItemId = InItem.ItemId;
					PlayerItem.ItemType = InItem.ItemType;
					PlayerItem.Count = PlayerItem.Count == 1 ? InItem.Count : PlayerItem.Count;
					bFound = true;
					break;
				}
			}
			if (!bFound)
			{
				PlayerItems->Add(InItem);
			}
		}
	}
	else
	{
		EquippedItems.Add(PlayerIndex, InItems);
	}
}

TArray<FEquippedItem>* UAccelByteWarsGameInstance::GetEquippedItems(const int32 PlayerIndex)
{
	return EquippedItems.Find(PlayerIndex) ? &EquippedItems[PlayerIndex] : nullptr;
}

void UAccelByteWarsGameInstance::UpdateEquippedItemsByInGameItemId(const int32 PlayerIndex, const TMap<FString, int32>& InGameItemIds)
{
	// Get items
	TArray<FEquippedItem> InItems;
	for (const TTuple<FString, int>& InGameItemId : InGameItemIds)
	{
		if (const UInGameItemDataAsset* ItemDataAsset = UInGameItemUtility::GetItemDataAsset(InGameItemId.Key))
		{
			InItems.Add({ItemDataAsset->Type, ItemDataAsset->Id, InGameItemId.Value});
		}
	}

	UpdateEquippedItems(PlayerIndex, InItems);
}

void UAccelByteWarsGameInstance::ModifyEquippedItemCountByInGameItemId(
	const int32 PlayerIndex,
	const FString& ItemId,
	const int32 Modifier)
{
	if (EquippedItems.Find(PlayerIndex))
	{
		for (FEquippedItem& EquippedItem : EquippedItems[PlayerIndex])
		{
			if (EquippedItem.ItemId.Equals(ItemId))
			{
				EquippedItem.Count += Modifier;
				return;
			}
		}
	}
}

bool UAccelByteWarsGameInstance::UpdateEquippedItemsBySku(
	const int32 PlayerIndex,
	const EItemSkuPlatform Platform,
	const TMap<FString, int32> Skus)
{
	bool bSucceeded = false;

	// Get items
	TArray<FEquippedItem> InItems;
	for (const TTuple<FString, int>& Sku : Skus)
	{
		if (const UInGameItemDataAsset* ItemDataAsset = UInGameItemUtility::GetItemDataAssetBySku(Platform, Sku.Key))
		{
			InItems.Add({ItemDataAsset->Type, ItemDataAsset->Id, Sku.Value});
			bSucceeded = true;
		}
		else
		{
			GAMEINSTANCE_LOG("SKU (%s) doesn't match with any in-game item", *Sku.Key);
		}
	}

	UpdateEquippedItems(PlayerIndex, InItems);
	return bSucceeded;
}

void UAccelByteWarsGameInstance::UnEquipItemBySku(
	const int32 PlayerIndex,
	const EItemSkuPlatform Platform,
	const FString& Sku)
{
	if (TArray<FEquippedItem>* PlayerItems = EquippedItems.Find(PlayerIndex))
	{
		const UInGameItemDataAsset* ItemDataAsset = UInGameItemUtility::GetItemDataAssetBySku(Platform, Sku);
		if (!ItemDataAsset)
		{
			return;
		}

		PlayerItems->Remove(FEquippedItem{ItemDataAsset->Type, ItemDataAsset->Id});
	}
}

void UAccelByteWarsGameInstance::UnEquipAll(const int32 PlayerIndex)
{
	EquippedItems.Remove(PlayerIndex);
}

bool UAccelByteWarsGameInstance::IsItemEquippedBySku(
	const int32 PlayerIndex,
	const EItemSkuPlatform Platform,
	const FString& Sku)
{
	if (TArray<FEquippedItem>* PlayerItems = EquippedItems.Find(PlayerIndex))
	{
		FString ItemId;
		if (const UInGameItemDataAsset* ItemDataAsset = UInGameItemUtility::GetItemDataAssetBySku(Platform, Sku))
		{
			ItemId = ItemDataAsset->Id;
		}

		for (FEquippedItem& PlayerItem : *PlayerItems)
		{	
			if (PlayerItem.ItemId == ItemId)
			{
				return true;
			}
		}
	}

	return false;
}

void UAccelByteWarsGameInstance::SaveGameSettings(const APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	ensure(LocalPlayer != nullptr);
	const int32 LocalUserNum = LocalPlayer->GetControllerId();

	SaveGameSettings(LocalUserNum);
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
	// Abort if the network error message is the same.
	if (LastFailureMessage.Equals(Message)) 
	{
		return;
	}

	// Ignore network error message if the game is over (GAME_ENDS) or server is shutdown gracefully (INVALID)
	const AAccelByteWarsInGameGameState* GameState = Cast<AAccelByteWarsInGameGameState>(GetWorld()->GetGameState());
	if (GameState && (GameState->GameStatus == EGameStatus::GAME_ENDS || GameState->GameStatus == EGameStatus::INVALID)) 
	{
		return;
	}

	LastFailureMessage = Message;
	bPendingFailureNotification = true;
	LastFailureType = FailureType;
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

bool UAccelByteWarsGameInstance::GetGameStatsDataById(const FName& Id, FGameStatsData& OutGameStatsData) const
{
	if (ensure(GameStatsDataTable))
	{
		FGameStatsData* FoundData = GameStatsDataTable->FindRow<FGameStatsData>(Id, TEXT("GetGameStatsDataById"));
		if (FoundData) 
		{
			OutGameStatsData = *FoundData;
			return true;
		}
		else 
		{
			GAMEINSTANCE_LOG("Game stats data of id %s is not found.", *Id.ToString());
		}
	}
	return false;
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
		UAccelByteWarsActivatableWidget* HUDWidgetAB = static_cast<UAccelByteWarsActivatableWidget*>(HUDWidget);
		HUDWidgetAB->OpenFTUEWidget();
	}
}

#pragma region "Lobby Connect/Disconnect using PS Controller"
#ifdef AGS_LOBBY_CHEAT_ENABLED
void UAccelByteWarsGameInstance::LobbyConnect()
{
	UStartupSubsystem* StartupSubsystem = GetSubsystem<UStartupSubsystem>();
	StartupSubsystem->LobbyConnect({});

	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Lobby Connect executed"));
}

void UAccelByteWarsGameInstance::LobbyDisconnect()
{
	UStartupSubsystem* StartupSubsystem = GetSubsystem<UStartupSubsystem>();
	StartupSubsystem->LobbyDisconnect({});

	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Lobby Disconnect executed"));
}
#endif // AGS_LOBBY_CHEAT_ENABLED
#pragma endregion