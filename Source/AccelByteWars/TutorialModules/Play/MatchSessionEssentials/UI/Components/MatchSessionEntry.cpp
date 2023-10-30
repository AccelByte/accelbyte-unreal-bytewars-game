// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchSessionEntry.h"

#include "CommonButtonBase.h"
#include "Components/TextBlock.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UMatchSessionEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	const UMatchSessionData* SessionData = Cast<UMatchSessionData>(ListItemObject);
	ensure(SessionData);

	SessionEssentialInfo = SessionData->SessionEssentialInfo;
	OnJoinButtonClicked = SessionData->OnJoinButtonClicked;

	Btn_Join->OnClicked().AddWeakLambda(this, [this]()
	{
		OnJoinButtonClicked.ExecuteIfBound(SessionEssentialInfo.SessionSearchResult);
	});

	UiSetup();

	Super::NativeOnListItemObjectSet(ListItemObject);
}

void UMatchSessionEntry::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	Btn_Join->OnClicked().Clear();
}

void UMatchSessionEntry::UiSetup() const
{
	// owner avatar
	W_OwnerAvatar->LoadImage(SessionEssentialInfo.OwnerAvatarUrl);

	// owner display name
	const FText MatchLiteralText = LOCTEXT("match", "match");
	const FString FormattedOwnerNameString = FString::Printf(
		TEXT("%s's %s"),
		*SessionEssentialInfo.OwnerName,
		*MatchLiteralText.ToString());
	Tb_OwnerDisplayName->SetText(FText::FromString(FormattedOwnerNameString));

	// game mode type
	FText GameModeTypeText;
	switch (SessionEssentialInfo.GameModeType)
	{
	case EGameModeType::FFA:
		GameModeTypeText = LOCTEXT("FFA", "Elimination");
		break;
	case EGameModeType::TDM:
		GameModeTypeText = LOCTEXT("TDM", "Team Deathmatch");
		break;
	}
	Tb_GameModeType->SetText(GameModeTypeText);

	// network type
	FText NetworkTypeText;
	switch (SessionEssentialInfo.NetworkType)
	{
	case EGameModeNetworkType::DS:
		NetworkTypeText = LOCTEXT("DS", "DS");
		break;
	case EGameModeNetworkType::P2P:
		NetworkTypeText = LOCTEXT("P2P", "P2P");
		break;
	case EGameModeNetworkType::LOCAL:
		NetworkTypeText = LOCTEXT("LOCAL", "LOCAL");
		break;
	}
	Tb_NetworkType->SetText(NetworkTypeText);

	// registered player count
	const FText PlayerLiteralText = LOCTEXT("Player", "Player");
	const FString FormattedPlayerCountString = FString::Printf(
		TEXT("%d/%d %s"),
		SessionEssentialInfo.RegisteredPlayerCount,
		SessionEssentialInfo.MaxPlayerCount,
		*PlayerLiteralText.ToString());
	Tb_RegisteredPlayerCount->SetText(FText::FromString(FormattedPlayerCountString));

	// disable join button if registered players is full
	Btn_Join->SetIsEnabled(SessionEssentialInfo.RegisteredPlayerCount < SessionEssentialInfo.MaxPlayerCount);
}

#undef LOCTEXT_NAMESPACE
