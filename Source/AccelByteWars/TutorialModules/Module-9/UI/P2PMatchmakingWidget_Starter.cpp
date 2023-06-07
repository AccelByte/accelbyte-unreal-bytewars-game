// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-9/UI/P2PMatchmakingWidget_Starter.h"
#include "TutorialModules/Module-9/P2PMatchmakingSubsystem_Starter.h"
#include "TutorialModules/Module-3/UI/QuickPlayWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"

void UP2PMatchmakingWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_StartP2PMatchmaking->OnClicked().AddUObject(this, &ThisClass::OnStartP2PMatchmakingButtonClicked);
}

void UP2PMatchmakingWidget_Starter::NativeDestruct()
{
	Super::NativeDestruct();

	Btn_StartP2PMatchmaking->OnClicked().Clear();
}

void UP2PMatchmakingWidget_Starter::OnStartP2PMatchmakingButtonClicked()
{
	// TODO: Trigger start P2P matchmaking here.
}