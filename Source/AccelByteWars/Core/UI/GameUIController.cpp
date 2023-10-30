// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/GameUIController.h"
#include "Engine/LocalPlayer.h"
#include "Core/Player/CommonLocalPlayer.h"
#include "GameUIManagerSubsystem.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

// Static
UGameUIController* UGameUIController::GetGameUIController(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UGameUIManagerSubsystem* UIManager = UGameInstance::GetSubsystem<UGameUIManagerSubsystem>(GameInstance))
			{
				return UIManager->GetCurrentUIController();
			}
		}
	}

	return nullptr;
}

UGameUIManagerSubsystem* UGameUIController::GetOwningUIManager() const
{
	return CastChecked<UGameUIManagerSubsystem>(GetOuter());
}

UWorld* UGameUIController::GetWorld() const
{
	return GetOwningUIManager()->GetGameInstance()->GetWorld();
}

UAccelByteWarsBaseUI* UGameUIController::GetRootLayout(const UCommonLocalPlayer* LocalPlayer) const
{
	const FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer);
	return LayoutInfo ? LayoutInfo->RootLayout : nullptr;
}

void UGameUIController::NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer)
{
	// TODO @afif on player added then add BaseUI / Layout for this local player from viewport
	/*LocalPlayer->OnPlayerControllerSet.AddWeakLambda(this, [this](UCommonLocalPlayer* LocalPlayer, APlayerController* PlayerController)
	{
		NotifyPlayerRemoved(LocalPlayer);

		if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
		{
			AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
			LayoutInfo->bAddedToViewport = true;
		}
		else
		{
			CreateLayoutWidget(LocalPlayer);
		}
	});

	if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = true;
	}
	else
	{
		CreateLayoutWidget(LocalPlayer);
	}*/
}

void UGameUIController::NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer)
{
	// TODO @afif on player removed then remove BaseUI / Layout for this local player from viewport
	/*if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		RemoveLayoutFromViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = false;

		if (LocalMultiplayerInteractionMode == ELocalMultiplayerInteractionMode::SingleToggle && !LocalPlayer->IsPrimaryPlayer())
		{
			UAccelByteWarsBaseUI* RootLayout = LayoutInfo->RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				// We're removing a secondary player's root while it's in control - transfer control back to the primary player's root
				RootLayout->SetIsDormant(true);
				for (const FRootViewportLayoutInfo& RootLayoutInfo : RootViewportLayouts)
				{
					if (RootLayoutInfo.LocalPlayer->IsPrimaryPlayer())
					{
						if (UAccelByteWarsBaseUI* PrimaryRootLayout = RootLayoutInfo.RootLayout)
						{
							PrimaryRootLayout->SetIsDormant(false);
						}
					}
				}
			}
		}
	}*/
}

void UGameUIController::NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer)
{
	/*NotifyPlayerRemoved(LocalPlayer);
	LocalPlayer->OnPlayerControllerSet.RemoveAll(this);
	const int32 LayoutInfoIdx = RootViewportLayouts.IndexOfByKey(LocalPlayer);
	if (LayoutInfoIdx != INDEX_NONE)
	{
		UAccelByteWarsBaseUI* Layout = RootViewportLayouts[LayoutInfoIdx].RootLayout;
		RootViewportLayouts.RemoveAt(LayoutInfoIdx);

		RemoveLayoutFromViewport(LocalPlayer, Layout);

		OnRootLayoutReleased(LocalPlayer, Layout);
	}*/
}

void UGameUIController::AddLayoutToViewport(UCommonLocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout)
{
	UE_LOG(LogTemp, Log, TEXT("[%s] is adding player [%s]'s root layout [%s] to the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

	Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
	Layout->AddToPlayerScreen(1000);

	OnRootLayoutAddedToViewport(LocalPlayer, Layout);
}

void UGameUIController::RemoveLayoutFromViewport(UCommonLocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout)
{
	TWeakPtr<SWidget> LayoutSlateWidget = Layout->GetCachedWidget();
	if (LayoutSlateWidget.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] is removing player [%s]'s root layout [%s] from the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

		Layout->RemoveFromParent();
		if (LayoutSlateWidget.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("Player [%s]'s root layout [%s] has been removed from the viewport, but other references to its underlying Slate widget still exist. Noting in case we leak it."), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));
		}

		OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);
	}
}

void UGameUIController::OnRootLayoutAddedToViewport(UCommonLocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout)
{
#if WITH_EDITOR
	if (GIsEditor && LocalPlayer->IsPrimaryPlayer())
	{
		// So our controller will work in PIE without needing to click in the viewport
		FSlateApplication::Get().SetUserFocusToGameViewport(0);
	}
#endif
}

void UGameUIController::OnRootLayoutRemovedFromViewport(UCommonLocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout)
{
	
}

void UGameUIController::OnRootLayoutReleased(UCommonLocalPlayer* LocalPlayer, UAccelByteWarsBaseUI* Layout)
{
	
}

void UGameUIController::RequestPrimaryControl(UAccelByteWarsBaseUI* Layout)
{
	/*if (LocalMultiplayerInteractionMode == ELocalMultiplayerInteractionMode::SingleToggle && Layout->IsDormant())
	{
		for (const FRootViewportLayoutInfo& LayoutInfo : RootViewportLayouts)
		{
			UAccelByteWarsBaseUI* RootLayout = LayoutInfo.RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				RootLayout->SetIsDormant(true);
				break;
			}
		}
		Layout->SetIsDormant(false);
	}*/
}

void UGameUIController::CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer)
{
	if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
	{
		TSubclassOf<UAccelByteWarsBaseUI> LayoutWidgetClass = GetLayoutWidgetClass(LocalPlayer);
		if (ensure(LayoutWidgetClass && !LayoutWidgetClass->HasAnyClassFlags(CLASS_Abstract)))
		{
			UAccelByteWarsBaseUI* NewLayoutObject = CreateWidget<UAccelByteWarsBaseUI>(PlayerController, LayoutWidgetClass);
			RootViewportLayouts.Emplace(LocalPlayer, NewLayoutObject, true);
			
			AddLayoutToViewport(LocalPlayer, NewLayoutObject);
		}
	}
}

TSubclassOf<UAccelByteWarsBaseUI> UGameUIController::GetLayoutWidgetClass(UCommonLocalPlayer* LocalPlayer)
{
	return LayoutClass.LoadSynchronous();
}