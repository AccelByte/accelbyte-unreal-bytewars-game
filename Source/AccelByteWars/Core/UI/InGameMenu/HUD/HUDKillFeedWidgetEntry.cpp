// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "HUDKillFeedWidgetEntry.h"
#include "Components/TextBlock.h"

void UHUDKillFeedWidgetEntry::NativeConstruct()
{
	Super::NativeConstruct();
}

void UHUDKillFeedWidgetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	TWeakObjectPtr<UHUDKillFeedData> KillFeed = Cast<UHUDKillFeedData>(ListItemObject);
	if (!KillFeed.IsValid()) 
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot display kill feed entry. Kill feed data is null."));
		return;
	}

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot display kill feed entry. Game instance is null."));
		return;
	}

	Tb_Killer->SetText(FText::FromString(KillFeed->Killer.PlayerName));
	Tb_Killer->SetColorAndOpacity(GameInstance->GetTeamColor(KillFeed->Killer.TeamId));

	Tb_Victim->SetText(FText::FromString(KillFeed->Victim.PlayerName));
	Tb_Victim->SetColorAndOpacity(GameInstance->GetTeamColor(KillFeed->Victim.TeamId));

	OnListItemObjectSet.Broadcast();
}