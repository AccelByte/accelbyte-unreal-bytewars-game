// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

DEFINE_LOG_CATEGORY(LogTutorialModuleSubsystem);

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
	// Assign associate Tutorial Module based on default object.
	AssociateTutorialModule = GetClass()->GetDefaultObject<UTutorialModuleSubsystem>()->AssociateTutorialModule;
	
	// Initialize utility subsystem dependencies.
	Collection.InitializeDependency(UPromptSubsystem::StaticClass());

	// Initialize Tutorial Module subsystem dependencies.
	if (AssociateTutorialModule) 
	{
		UE_LOG_TUTORIALMODULESUBSYSTEM(Log, TEXT("Initialize Tutorial Module Subsystem: %s"), *AssociateTutorialModule->CodeName);
		for (const UTutorialModuleDataAsset* Dependency : AssociateTutorialModule->TutorialModuleDependencies) 
		{
			if (!Dependency || !Dependency->IsActiveAndDependenciesChecked() || !Dependency->GetTutorialModuleSubsystemClass().Get())
			{
				UE_LOG_TUTORIALMODULESUBSYSTEM(Warning, TEXT("Failed to initialize Tutorial Module Subsystem dependency for %s. The dependency Tutorial Module %s is not active."), 
					*AssociateTutorialModule->CodeName,
					*Dependency->CodeName);
				continue;
			}

			const USubsystem* Subsystem = Collection.InitializeDependency(Dependency->GetTutorialModuleSubsystemClass());
			UE_LOG_TUTORIALMODULESUBSYSTEM(Log, TEXT("Initialize Tutorial Module Subsystem dependency %s for %s. Success: %s"),
				*Dependency->CodeName,
				*AssociateTutorialModule->CodeName,
				Subsystem == nullptr ? TEXT("FALSE") : TEXT("TRUE"));
		}
	}

	Super::Initialize(Collection);

#if UE_BUILD_DEVELOPMENT
	RegisterCommands();
#endif
}

bool UTutorialModuleSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return AssociateTutorialModule ? AssociateTutorialModule->IsActiveAndDependenciesChecked() : false;
}

void UTutorialModuleSubsystem::ExecuteNextTick(const FTimerDelegate& Delegate) const
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(Delegate);
}
