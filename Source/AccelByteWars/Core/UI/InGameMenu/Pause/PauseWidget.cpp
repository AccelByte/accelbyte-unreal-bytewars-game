// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "CommonButtonBase.h"
#include "Core/GameModes/AccelByteWarsGameStateBase.h"

void UPauseWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// if on server, disable restart button
	Btn_Restart->SetVisibility(
		GetOwningPlayer()->HasAuthority() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	Btn_Resume->OnClicked().AddUObject(this, &UPauseWidget::ResumeGame);
	Btn_Restart->OnClicked().AddUObject(this, &UPauseWidget::RestartGame);
	Btn_Quit->OnClicked().AddUObject(this, &UPauseWidget::QuitGame);

	SetInputModeToUIOnly();
}

void UPauseWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// Unbind buttons click event.
	Btn_Resume->OnClicked().RemoveAll(this);
	Btn_Restart->OnClicked().RemoveAll(this);
	Btn_Quit->OnClicked().RemoveAll(this);

	SetInputModeToGameOnly();
}

void UPauseWidget::ResumeGame()
{
	DeactivateWidget();
}

void UPauseWidget::RestartGame()
{
	if (GetOwningPlayer()->HasAuthority())
	{
		AAccelByteWarsGameStateBase* GameState = Cast<AAccelByteWarsGameStateBase>(GetWorld()->GetGameState());
		ensure(GameState);

		GameState->bIsServerTravelling = true;
		GameState->OnNotify_IsServerTravelling();

		// Waits for replication before travelling
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			GetWorld()->ServerTravel("/Game/ByteWars/Maps/GalaxyWorld/GalaxyWorld");
		}, 1.0f, false);
	}
}

void UPauseWidget::QuitGame()
{
	if (OnQuitGameDelegate.IsBound()) 
	{
		OnQuitGameDelegate.Broadcast(GetOwningPlayer());
	}
	
	OnExitLevel();

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenu"));
}