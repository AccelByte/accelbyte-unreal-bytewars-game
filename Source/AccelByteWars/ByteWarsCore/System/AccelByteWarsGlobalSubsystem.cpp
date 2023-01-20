// Fill out your copyright notice in the Description page of Project Settings.


#include "ByteWarsCore/System/AccelByteWarsGlobalSubsystem.h"
#include "ByteWarsCore/System/AccelByteWarsGlobals.h"

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