// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/MultiplayerEntries/PlayerEntryWidget.h"

#include "Components/HorizontalBox.h"
#include "Core/UI/Components/AccelByteWarsAsyncImageWidget.h"
#include "Components/TextBlock.h"

void UPlayerEntryWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Hb_PlatformOuter->SetVisibility(Hb_PlatformOuter->HasAnyChildren() ?
		ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UPlayerEntryWidget::SetUsername(const FText& Username)
{
	Tb_Username->SetText(Username);
}

void UPlayerEntryWidget::SetAvatar(const FString& AvatarURL)
{
	Img_Avatar->LoadImage(AvatarURL);
}

void UPlayerEntryWidget::SetTextColor(const FLinearColor& Color)
{
	Tb_Username->SetColorAndOpacity(FSlateColor(Color));
}

void UPlayerEntryWidget::SetAvatarTint(const FLinearColor& Color)
{
	Img_Avatar->SetImageTint(Color);
}

void UPlayerEntryWidget::SetNetId(FUniqueNetIdPtr Id)
{
	NetId = Id;
}

FUniqueNetIdPtr UPlayerEntryWidget::GetNetId() const
{
	return NetId;
}
