// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "HUDShipLabelWidget.h"
#include "Components/TextBlock.h"

void UHUDShipLabelWidget::Setup(const FGameplayPlayerData& Data)
{
	Tb_PlayerName->SetText(FText::FromString(Data.PlayerName));
	Tb_LocalPlayerIndicator->SetVisibility(ESlateVisibility::Collapsed);

	FLinearColor Color = FLinearColor::White;
	if (const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()))
	{
		Color = GameInstance->GetTeamColor(Data.TeamId);
	}
	Tb_PlayerName->SetColorAndOpacity(Color);
	Tb_LocalPlayerIndicator->SetColorAndOpacity(Color);
	
	// Only apply indicator for online game.
	if (GetOwningPlayer() && GetOwningPlayer()->GetNetMode() != ENetMode::NM_Standalone)
	{
		if (const APlayerController* PC = GetOwningPlayer())
		{
			if (const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				bool bIsLocalPlayer = Data.UniqueNetId == LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
				Tb_LocalPlayerIndicator->SetVisibility(bIsLocalPlayer ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			}
		}
	}
}

