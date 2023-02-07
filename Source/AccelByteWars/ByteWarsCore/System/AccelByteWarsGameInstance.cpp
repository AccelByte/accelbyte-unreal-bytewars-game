// Fill out your copyright notice in the Description page of Project Settings.


#include "ByteWarsCore/System/AccelByteWarsGameInstance.h"
#include "ByteWarsCore/UI/GameUIManagerSubsystem.h"
#include "ByteWarsCore/Player/CommonLocalPlayer.h"

#define GAMEINSTANCE_LOG(FormatString, ...) UE_LOG(LogByteWarsGameInstance, Log, TEXT(FormatString), __VA_ARGS__);

int32 UAccelByteWarsGameInstance::AddLocalPlayer(ULocalPlayer* NewPlayer, int32 ControllerId)
{
	const int32 ReturnVal = Super::AddLocalPlayer(NewPlayer, ControllerId);
	if (ReturnVal != INDEX_NONE)
	{
		if (!PrimaryPlayer.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("AddLocalPlayer: Set %s to Primary Player"), *NewPlayer->GetName());
			PrimaryPlayer = NewPlayer;
		}
		
		UE_LOG(LogTemp, Log, TEXT("AddLocalPlayer: New player %s is set to ControllerId: %i"), *NewPlayer->GetName(), ControllerId);
		GetSubsystem<UGameUIManagerSubsystem>()->NotifyPlayerAdded(Cast<UCommonLocalPlayer>(NewPlayer));

		if(OnLocalPlayerAdded.IsBound())
		{
			OnLocalPlayerAdded.Broadcast(NewPlayer);
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
		UE_LOG(LogTemp, Log, TEXT("RemoveLocalPlayer: Unsetting Primary Player from %s"), *ExistingPlayer->GetName());
	}
	GetSubsystem<UGameUIManagerSubsystem>()->NotifyPlayerDestroyed(Cast<UCommonLocalPlayer>(ExistingPlayer));

	if(OnLocalPlayerRemoved.IsBound())
	{
		OnLocalPlayerRemoved.Broadcast(ExistingPlayer);
	}
	
	return Super::RemoveLocalPlayer(ExistingPlayer);
}

EGameModeType UAccelByteWarsGameInstance::GetCurrentGameModeType() const
{
	return GameSetup.GameModeType;
}

void UAccelByteWarsGameInstance::AssignGameMode(FString CodeName)
{
	GameSetup = GetGameModeDataByCodeName(CodeName);

	GAMEINSTANCE_LOG("Game mode set to GameState: %s", *GameSetup.CodeName);
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
			// fallback
			Data = *GameModeDatas[0];

			for (const FGameModeData* GameModeData : GameModeDatas)
			{
				if (GameModeData->CodeName.Equals(CodeName))
				{
					Data = *GameModeData;
					break;
				}
			}
		}
	}
	return Data;
}
