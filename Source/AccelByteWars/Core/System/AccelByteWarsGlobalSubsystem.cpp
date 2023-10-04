// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/System/AccelByteWarsGlobalSubsystem.h"
#include "Core/System/AccelByteWarsGlobals.h"

void UAccelByteWarsGlobalSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!CurrentGlobals && !DefaultGlobalsClass.IsNull())
	{
		TSubclassOf<UAccelByteWarsGlobals> GlobalClass = DefaultGlobalsClass.LoadSynchronous();
		CurrentGlobals = NewObject<UAccelByteWarsGlobals>(this, GlobalClass);
	}
}

void UAccelByteWarsGlobalSubsystem::Deinitialize()
{
	Super::Deinitialize();

	CurrentGlobals = nullptr;
}

bool UAccelByteWarsGlobalSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// Only create an instance if there is no override implementation defined elsewhere
		return ChildClasses.Num() == 0;
	}

	return false;
}