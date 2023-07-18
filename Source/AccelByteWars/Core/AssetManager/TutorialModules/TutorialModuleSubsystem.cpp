// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"

void UTutorialModuleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Assign associate Tutorial Module based on default object.
	AssociateTutorialModule = GetClass()->GetDefaultObject<UTutorialModuleSubsystem>()->AssociateTutorialModule;
}

bool UTutorialModuleSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return AssociateTutorialModule ? AssociateTutorialModule->IsActiveAndDependenciesChecked() : false;
}