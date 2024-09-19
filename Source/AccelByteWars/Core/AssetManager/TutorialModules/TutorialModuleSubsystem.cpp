// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"

#if UE_BUILD_DEVELOPMENT
void UTutorialModuleSubsystem::RegisterCommands()
{
	for (const FCheatCommandEntry& Entry : GetCheatCommandEntries())
	{
		IConsoleManager::Get().RegisterConsoleCommand(Entry.Command, Entry.Description, Entry.Delegate);
	}
}
#endif

void UTutorialModuleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Assign associate Tutorial Module based on default object.
	AssociateTutorialModule = GetClass()->GetDefaultObject<UTutorialModuleSubsystem>()->AssociateTutorialModule;

#if UE_BUILD_DEVELOPMENT
	RegisterCommands();
#endif
}

bool UTutorialModuleSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return AssociateTutorialModule ? AssociateTutorialModule->IsActiveAndDependenciesChecked() : false;
}