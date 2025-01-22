// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "CoreSubsystem.h"

#include "Core/Player/AccelByteWarsPlayerController.h"
#include "GameFramework/GameStateBase.h"

void UCoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#pragma region "Core game cheats"
	if (UGUICheatWidgetEntry* GUICheatEntryKillSelf = FTutorialModuleGeneratedWidget::GetGUICheatMetadataById(GUI_CHEAT_ENTRY_KILL_SELF))
	{
		GUICheatEntryKillSelf->OnClicked.AddDynamic(this, &ThisClass::KillSelf);
	}
	if (UGUICheatWidgetEntry* GUICheatEntryKillOthers = FTutorialModuleGeneratedWidget::GetGUICheatMetadataById(GUI_CHEAT_ENTRY_KILL_OTHERS))
	{
		GUICheatEntryKillOthers->OnClicked.AddDynamic(this, &ThisClass::KillOthers);
	}
#pragma endregion 
}

#pragma region "Core game cheats"
void UCoreSubsystem::KillSelf(const TArray<FString>& Args)
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	AAccelByteWarsPlayerController* AccelByteWarsPlayerController = Cast<AAccelByteWarsPlayerController>(PlayerController);
	if (!AccelByteWarsPlayerController)
	{
		return;
	}

	AccelByteWarsPlayerController->ClientInstructInstaKillPlayer({PlayerController->PlayerState});
}

void UCoreSubsystem::KillOthers(const TArray<FString>& Args)
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	AAccelByteWarsPlayerController* AccelByteWarsPlayerController = Cast<AAccelByteWarsPlayerController>(PlayerController);
	if (!AccelByteWarsPlayerController)
	{
		return;
	}

	TArray<APlayerState*> PlayerStates = GetWorld()->GetGameState()->PlayerArray;

	// Exclude local user
	const APlayerState* LocalPlayer = GetWorld()->GetFirstPlayerController()->PlayerState;
	PlayerStates.RemoveAll([LocalPlayer](const APlayerState* PlayerState)
	{
		return PlayerState == LocalPlayer;
	});

	AccelByteWarsPlayerController->ClientInstructInstaKillPlayer(PlayerStates);
}
#pragma endregion 
