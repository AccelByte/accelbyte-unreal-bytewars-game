// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/System/AccelByteWarsGlobals.h"


TArray<FGameModeData*> UAccelByteWarsGlobals::GetAllGameModes() const
{
	TArray<FGameModeData*> GameModeDataArray;

	if (ensure(GameModes))
	{
		GameModes->GetAllRows<FGameModeData>("UAccelByteWarsGlobals::GetAllGameModes", GameModeDataArray);
	}

	return GameModeDataArray;
}

TArray<FString> UAccelByteWarsGlobals::GetAllGameModeCodeNames() const
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

//FGameModeData* UAccelByteWarsGlobals::GetGameModeDataById(int32 GameModeId) const
bool UAccelByteWarsGlobals::GetGameModeDataById(int32 GameModeId, UPARAM(ref) FGameModeData& OutGameModeData) const
{
	if (ensure(GameModes))
	{
		GameModes->FindRow<FGameModeData>(*FString::FromInt(GameModeId), "UAccelByteWarsGlobals::GetGameModeDataById", false);
		return true;
	}

	return false;
}

TArray<FString> UAccelByteWarsGlobals::GetGameModeDataByType(EGameModeType GameModeType) const
{
	TArray<FString> CodeNames;

	TArray<FGameModeData*> GameModeDataArray = GetAllGameModes();
	for (FGameModeData* GameModeData : GameModeDataArray)
	{
		if (!GameModeData) continue;
		if(GameModeData->GameModeType == GameModeType)
		{
			CodeNames.Add(GameModeData->CodeName);
		}
	}
	
	return CodeNames;
}

//FGameModeData* UAccelByteWarsGlobals::GetGameModeDataByCodeName(const FString& CodeName) const
bool UAccelByteWarsGlobals::GetGameModeDataByCodeName(const FString& CodeName, UPARAM(ref) FGameModeData& OutGameModeData) const
{
	TArray<FGameModeData*> ModeDataArray = GetAllGameModes();
	for (FGameModeData* ModeData : ModeDataArray)
	{
		if (ModeData && ModeData->CodeName == CodeName)
		{
			//return ModeData;
			OutGameModeData = *ModeData;
			return true;
		}
	}

	return false;
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

//FGameModeTypeData* UAccelByteWarsGlobals::GetGameModeTypeData(const EGameModeType Type) const
bool UAccelByteWarsGlobals::GetGameModeTypeData(const EGameModeType Type, UPARAM(ref) FGameModeTypeData& OutGameModeTypeData) const
{
	TArray<FGameModeTypeData*> TypeDataArray = GetAllGameModeTypes();
	for (FGameModeTypeData* TypeData : TypeDataArray)
	{
		if (TypeData && TypeData->Type == Type)
		{
			OutGameModeTypeData = *TypeData;
			return true;
		}
	}

	return false;
}

TArray<EGameModeType> UAccelByteWarsGlobals::GetAllGameModeTypesEnum() const
{
	TArray<EGameModeType> GameModeEnums;

	TArray<FGameModeTypeData*> TypeDataArray = GetAllGameModeTypes();
	for (FGameModeTypeData* TypeData : TypeDataArray)
	{
		if (!TypeData) continue;
		GameModeEnums.Add(TypeData->Type);
	}

	return GameModeEnums;
}

