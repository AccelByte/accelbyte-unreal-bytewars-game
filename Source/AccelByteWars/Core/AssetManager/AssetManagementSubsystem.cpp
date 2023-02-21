// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AssetManager/AssetManagementSubsystem.h"

void UAssetManagementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if(!AssetManager)
	{
		AssetManager = &UAccelByteWarsAssetManager::Get();
	}
}

void UAssetManagementSubsystem::Deinitialize()
{
	Super::Deinitialize();

	AssetManager = nullptr;
}