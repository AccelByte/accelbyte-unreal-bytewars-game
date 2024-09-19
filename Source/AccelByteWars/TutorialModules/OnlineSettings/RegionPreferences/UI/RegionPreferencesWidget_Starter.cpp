// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "RegionPreferencesWidget_Starter.h"
#include "Components/ListView.h"
#include "Core/System/AccelByteWarsGameInstance.h"

void URegionPreferencesWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);
	
	RegionPreferencesSubsystem = GameInstance->GetSubsystem<URegionPreferencesSubsystem_Starter>();
	
	// put your code here
}

void URegionPreferencesWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();
	Lv_Regions->ClearListItems();
	
	// put your code here
}

void URegionPreferencesWidget_Starter::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// put your code here
}

void URegionPreferencesWidget_Starter::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 825.0f, 160.0f));

	// put your code here
}

#pragma region "Tutorial"
	// put your code here
#pragma endregion