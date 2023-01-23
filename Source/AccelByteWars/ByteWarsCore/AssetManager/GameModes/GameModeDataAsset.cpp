// Fill out your copyright notice in the Description page of Project Settings.


#include "ByteWarsCore/AssetManager/GameModes/GameModeDataAsset.h"

const FPrimaryAssetType	UGameModeDataAsset::GameModeAssetType = TEXT("GameMode");

FGameModeData UGameModeDataAsset::GetGameModeDataByCodeName(const FString& InCodeName)
{
	FPrimaryAssetId GameModeAssetId = GenerateAssetIdFromCodeName(InCodeName);

	FGameModeData GameModeData;
	GameModeData.CodeName = InCodeName;

	GameModeData.DisplayName = UAccelByteWarsDataAsset::GetDisplayNameForAsset(GameModeAssetId);

	FString DefaultClassString = UAccelByteWarsDataAsset::GetMetadataForAsset<FString>(GameModeAssetId, GET_MEMBER_NAME_CHECKED(UGameModeDataAsset, DefaultClass));
	GameModeData.DefaultClass = TSoftClassPtr<AAccelByteWarsGameModeBase>(DefaultClassString);
	
	GameModeData.GameModeType = static_cast<EGameModeType>(UAccelByteWarsDataAsset::GetMetadataForAsset<uint8>(GenerateAssetIdFromCodeName(InCodeName), GET_MEMBER_NAME_CHECKED(UGameModeDataAsset, GameModeType)));
	GameModeData.GameModeTypeString = GetGameModeTypeForCodeName(InCodeName).ToString();

	GameModeData.bIsLocalGame = UAccelByteWarsDataAsset::GetMetadataForAsset<bool>(GenerateAssetIdFromCodeName(InCodeName), GET_MEMBER_NAME_CHECKED(UGameModeDataAsset, bIsLocalGame));
	GameModeData.bIsTeamGame = UAccelByteWarsDataAsset::GetMetadataForAsset<bool>(GenerateAssetIdFromCodeName(InCodeName), GET_MEMBER_NAME_CHECKED(UGameModeDataAsset, bIsTeamGame));

	GameModeData.TeamNum = UAccelByteWarsDataAsset::GetMetadataForAsset<int32>(GenerateAssetIdFromCodeName(InCodeName), GET_MEMBER_NAME_CHECKED(UGameModeDataAsset, TeamNum));

	GameModeData.MatchTime = UAccelByteWarsDataAsset::GetMetadataForAsset<int32>(GenerateAssetIdFromCodeName(InCodeName), GET_MEMBER_NAME_CHECKED(UGameModeDataAsset, MatchTime));
	GameModeData.MaxPlayers = UAccelByteWarsDataAsset::GetMetadataForAsset<int32>(GenerateAssetIdFromCodeName(InCodeName), GET_MEMBER_NAME_CHECKED(UGameModeDataAsset, MaxPlayers));

	GameModeData.ScoreLimit = UAccelByteWarsDataAsset::GetMetadataForAsset<int32>(GenerateAssetIdFromCodeName(InCodeName), GET_MEMBER_NAME_CHECKED(UGameModeDataAsset, ScoreLimit));

	return GameModeData;
}

FPrimaryAssetId UGameModeDataAsset::GenerateAssetIdFromCodeName(const FString& InCodeName)
{
	return FPrimaryAssetId(UGameModeDataAsset::GameModeAssetType, FName(*InCodeName));
}

FString UGameModeDataAsset::GetCodeNameFromAssetId(const FPrimaryAssetId& AssetId)
{
	check(AssetId.PrimaryAssetType == UGameModeDataAsset::GameModeAssetType);
	return AssetId.PrimaryAssetName.ToString();
}

FText UGameModeDataAsset::GetDisplayNameByCodeName(const FString& InCodeName)
{
	return UAccelByteWarsDataAsset::GetDisplayNameForAsset(GenerateAssetIdFromCodeName(InCodeName));
}

FPrimaryAssetId UGameModeDataAsset::GetGameModeTypeForCodeName(const FString& InCodeName)
{
	const FString GameModeType = UAccelByteWarsDataAsset::GetMetadataForAsset<FString>(GenerateAssetIdFromCodeName(InCodeName), GET_MEMBER_NAME_CHECKED(UGameModeDataAsset, GameModeTypeString));
	return FPrimaryAssetId(GameModeType);
}