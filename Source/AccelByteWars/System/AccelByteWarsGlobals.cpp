// Fill out your copyright notice in the Description page of Project Settings.


#include "System/AccelByteWarsGlobals.h"


TArray<FGameModeData*> UAccelByteWarsGlobals::GetAllGameModes() const
{
	TArray<FGameModeData*> GameModeDataArray;

	if (ensure(GameModes))
	{
		GameModes->GetAllRows<FGameModeData>("UAccelByteWarsGlobals::GetAllGameModes", GameModeDataArray);
	}

	return GameModeDataArray;
}

TArray<FString> UAccelByteWarsGlobals::GetAllGameModeCodeName() const
{
	TArray<FString> CodeNames;

	TArray<FGameModeData*> GameModeDataArray = GetAllGameModes();
	for (FGameModeData* GameModeData : GameModeDataArray)
	{
		if (!GameModeData) continue;
		CodeNames.Add(GameModeData->CodeName);
	}

	return CodeNames;
}

FGameModeData* UAccelByteWarsGlobals::GetGameModeDataById(int32 GameModeId) const
{
	if (ensure(GameModes))
	{
		return GameModes->FindRow<FGameModeData>(*FString::FromInt(GameModeId), "UAccelByteWarsGlobals::GetGameModeDataById", false);
	}

	return nullptr;
}

FGameModeData* UAccelByteWarsGlobals::GetGameModeDataByCodeName(const FString& CodeName) const
{
	TArray<FGameModeData*> ModeDataArray = GetAllGameModes();
	for (FGameModeData* ModeData : ModeDataArray)
	{
		if (ModeData && ModeData->CodeName == CodeName)
		{
			return ModeData;
		}
	}

	return nullptr;
}

TArray<FGameModeTypeData*> UAccelByteWarsGlobals::GetAllGameModeTypes() const
{
	TArray<FGameModeTypeData*> GameModeTypeDatas;

	if (ensure(GameModeTypes))
	{
		GameModeTypes->GetAllRows<FGameModeTypeData>("UAccelByteWarsGlobals::GetAllGameModeTypes", GameModeTypeDatas);
	}

	return GameModeTypeDatas;
}

FGameModeTypeData* UAccelByteWarsGlobals::GetGameModeTypeData(const EGameModeType Type) const
{
	TArray<FGameModeTypeData*> TypeDataArray = GetAllGameModeTypes();
	for (FGameModeTypeData* TypeData : TypeDataArray)
	{
		if (TypeData && TypeData->Type == Type)
		{
			return TypeData;
		}
	}

	return nullptr;
}

