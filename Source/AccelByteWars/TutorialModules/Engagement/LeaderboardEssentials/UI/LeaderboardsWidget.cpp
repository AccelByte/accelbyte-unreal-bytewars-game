// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Engagement/LeaderboardEssentials/UI/LeaderboardsWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"

FString ULeaderboardsWidget::LeaderboardGameMode = TEXT("");

void ULeaderboardsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_SinglePlayer->OnClicked().AddUObject(this, &ThisClass::OpenLeaderboardsPeriod, FString::Printf(TEXT("singleplayer")));
	Btn_Elimination->OnClicked().AddUObject(this, &ThisClass::OpenLeaderboardsPeriod, FString::Printf(TEXT("elimination")));
	Btn_TeamDeathmatch->OnClicked().AddUObject(this, &ThisClass::OpenLeaderboardsPeriod, FString::Printf(TEXT("teamdeathmatch")));
}

void ULeaderboardsWidget::NativeOnDeactivated()
{
	Btn_SinglePlayer->OnClicked().Clear();
	Btn_Elimination->OnClicked().Clear();
	Btn_TeamDeathmatch->OnClicked().Clear();

	Super::NativeOnDeactivated();
}

void ULeaderboardsWidget::OpenLeaderboardsPeriod(const FString InGameMode)
{
	LeaderboardGameMode = InGameMode;

	UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetWorld()->GetGameInstance());
	ensure(GameInstance);

	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);

	BaseUIWidget->PushWidgetToStack(EBaseUIStackType::Menu, LeaderboardsPeriodWidgetClass);
}
