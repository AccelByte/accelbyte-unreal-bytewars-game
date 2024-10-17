// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"

#include "Kismet/GameplayStatics.h"
#include "CommonButtonBase.h"
#include "Components/VerticalBox.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/GameStates/AccelByteWarsGameState.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"

void UPauseWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// if on server, disable restart button
	Btn_Restart->SetVisibility(
		GetOwningPlayer()->HasAuthority() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	Btn_Resume->OnClicked().AddUObject(this, &UPauseWidget::ResumeGame);
	Btn_Restart->OnClicked().AddUObject(this, &UPauseWidget::RestartGame);
	Btn_HelpOptions->OnClicked().AddUObject(this, &UPauseWidget::OpenHelpOptions);
	Btn_Quit->OnClicked().AddUObject(this, &UPauseWidget::QuitGame);

	const AAccelByteWarsInGameGameState* GameState = Cast<AAccelByteWarsInGameGameState>(GetWorld()->GetGameState());
	if(GameState)
	{
		// hide generated buttons that meant for playing with online session
		Vb_OnlinePlayButtons->SetVisibility(GameState->GameSetup.NetworkType == EGameModeNetworkType::LOCAL ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	SetInputModeToUIOnly();
}

void UPauseWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// Unbind buttons click event.
	Btn_Resume->OnClicked().RemoveAll(this);
	Btn_Restart->OnClicked().RemoveAll(this);
	Btn_HelpOptions->OnClicked().RemoveAll(this);
	Btn_Quit->OnClicked().RemoveAll(this);

	SetInputModeToGameOnly();
}

// @@@SNIPSTART PauseWidget.cpp-ResumeGame
void UPauseWidget::ResumeGame()
{
	DeactivateWidget();
}
// @@@SNIPEND

// @@@SNIPSTART PauseWidget.cpp-RestartGame
void UPauseWidget::RestartGame()
{
	if (const AAccelByteWarsGameMode* GameMode = Cast<AAccelByteWarsGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->DelayedServerTravel("/Game/ByteWars/Maps/GalaxyWorld/GalaxyWorld");
	}
}
// @@@SNIPEND

void UPauseWidget::OpenHelpOptions()
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if(!ensure(GameInstance)) 
	{
		return;
	}

	UAccelByteWarsBaseUI* BaseUIWidget = Cast<UAccelByteWarsBaseUI>(GameInstance->GetBaseUIWidget());
	if (!ensure(BaseUIWidget)) 
	{
		return;
	}

	BaseUIWidget->PushWidgetToStack(GetWidgetStackType(), HelpOptionsWidgetClass);
}

// @@@SNIPSTART PauseWidget.cpp-QuitGame
void UPauseWidget::QuitGame()
{
	if (OnQuitGameDelegate.IsBound()) 
	{
		OnQuitGameDelegate.Broadcast(GetOwningPlayer());
	}
	
	OnExitLevel();

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenu"));
}
// @@@SNIPEND
