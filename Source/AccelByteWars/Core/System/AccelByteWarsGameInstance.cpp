// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/GameUIManagerSubsystem.h"
#include "Core/Player/CommonLocalPlayer.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/Loading/LoadingWidget.h"

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

	if (!BaseUIWidget->IsInViewport())
	{
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
