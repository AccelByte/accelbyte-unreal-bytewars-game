// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/MultiplayerEntries/PlayerEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UPlayerEntryWidget::SetUsername(const FText& Username)
{
	Tb_Username->SetText(Username);
}

void UPlayerEntryWidget::SetAvatar(const FSlateBrush& Avatar)
{
	Img_Avatar->SetBrush(Avatar);
}

void UPlayerEntryWidget::SetTextColor(const FLinearColor Color)
{
	Tb_Username->SetColorAndOpacity(FSlateColor(Color));
}

void UPlayerEntryWidget::SetAvatarTint(const FLinearColor Color)
{
	Img_Avatar->SetBrushColor(Color);
}