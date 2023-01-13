// Copyright Epic Games, Inc. All Rights Reserved.


#include "AccelByteWarsGameModeBase.h"

#include "ByteWarsCore/System/AccelByteWarsGameInstance.h"
#include "ByteWarsCore/System/AccelByteWarsGlobals.h"
#include "ByteWarsCore/System/AccelByteWarsGlobalSubsystem.h"

void AAccelByteWarsGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	// get value from launch argument, i.e -GameMode=LOCAL_TDM
	FString CodeName;
	FParse::Value(FCommandLine::Get(), TEXT("-GameMode="), CodeName);

	// set game mode if not yet selected
	AssignGameMode(CodeName);
}

void AAccelByteWarsGameModeBase::AssignGameMode(FString CodeName) const
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	if (!ensure(GameInstance)) return;

	// if game mode have been selected, then do nothing
	if (!GameInstance->SelectedGameMode.SelectedGameMode.CodeName.IsEmpty()) return;

	UAccelByteWarsGlobalSubsystem* GlobalSubsystem = GetGameInstance()->GetSubsystem<UAccelByteWarsGlobalSubsystem>();
	if (!ensure(GlobalSubsystem)) return;

	const UAccelByteWarsGlobals* Globals = GlobalSubsystem->GetCurrentGlobals();
	if (!ensure(Globals)) return;

	FGameModeData GameModeData;
	if (!Globals->GetGameModeDataByCodeName(CodeName, GameModeData))
	{
		// fallback
		Globals->GetGameModeDataByCodeName("LOCAL_FFA4", GameModeData);
	}

	// set game mode
	GameInstance->SelectedGameMode = FSelectedGameMode{GameModeData, GameModeData.MaxPlayers};

	UE_LOG(LogByteWarsGameMode, Log, TEXT("Game mode set: %s"), *GameModeData.CodeName);
}