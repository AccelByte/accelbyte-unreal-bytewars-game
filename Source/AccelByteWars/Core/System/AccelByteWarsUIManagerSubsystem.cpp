// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/System/AccelByteWarsUIManagerSubsystem.h"
#include "Core/UI/GameUIController.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "GameFramework/HUD.h"
#include "Core/Player/CommonLocalPlayer.h"

UAccelByteWarsUIManagerSubsystem::UAccelByteWarsUIManagerSubsystem()
{
	
}

void UAccelByteWarsUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UAccelByteWarsUIManagerSubsystem::Tick), 0.0f);
}

void UAccelByteWarsUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
}

bool UAccelByteWarsUIManagerSubsystem::Tick(float DeltaTime)
{
	SyncRootLayoutVisibilityToShowHUD();
	
	return true;
}

void UAccelByteWarsUIManagerSubsystem::SyncRootLayoutVisibilityToShowHUD()
{
	if (const UGameUIController* Policy = GetCurrentUIController())
	{
		for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers())
		{
			bool bShouldShowUI = true;
			
			if (const APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld()))
			{
				const AHUD* HUD = PC->GetHUD();

				if (HUD && !HUD->bShowHUD)
				{
					bShouldShowUI = false;
				}
			}

			if (UAccelByteWarsBaseUI* RootLayout = Policy->GetRootLayout(CastChecked<UCommonLocalPlayer>(LocalPlayer)))
			{
				const ESlateVisibility DesiredVisibility = bShouldShowUI ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
				if (DesiredVisibility != RootLayout->GetVisibility())
				{
					RootLayout->SetVisibility(DesiredVisibility);	
				}
			}
		}
	}
}