// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GameUIManagerSubsystem.h"
#include "UI/GameUIController.h"

void UGameUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!CurrentUIController && !DefaultUIControllerClass.IsNull())
	{
		TSubclassOf<UGameUIController> UIControllerClass = DefaultUIControllerClass.LoadSynchronous();
		SwitchToUIController(NewObject<UGameUIController>(this, UIControllerClass));
	}
}

void UGameUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	SwitchToUIController(nullptr);
}

bool UGameUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
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

void UGameUIManagerSubsystem::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
	UE_LOG(LogTemp, Log, TEXT("[%s] is adding player [%s]'s to the viewport"), *GetName(), *GetNameSafe(LocalPlayer));
	
	if (ensure(LocalPlayer) && CurrentUIController)
	{
		CurrentUIController->NotifyPlayerAdded(LocalPlayer);
	}
}

void UGameUIManagerSubsystem::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	UE_LOG(LogTemp, Log, TEXT("[%s] is adding player [%s]'s to the viewport"), *GetName(), *GetNameSafe(LocalPlayer));
	
	if (LocalPlayer && CurrentUIController)
	{
		CurrentUIController->NotifyPlayerRemoved(LocalPlayer);
	}
}

void UGameUIManagerSubsystem::NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer)
{
	UE_LOG(LogTemp, Log, TEXT("[%s] is adding player [%s]'s to the viewport"), *GetName(), *GetNameSafe(LocalPlayer));
	
	if (LocalPlayer && CurrentUIController)
	{
		CurrentUIController->NotifyPlayerDestroyed(LocalPlayer);
	}
}

void UGameUIManagerSubsystem::SwitchToUIController(UGameUIController* InUIController)
{
	if (CurrentUIController != InUIController)
	{
		CurrentUIController = InUIController;
	}
}