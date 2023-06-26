// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-9/UI/P2PMatchmakingWidget_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"

#include "TutorialModules/Module-9/P2PMatchmakingSubsystem_Starter.h"
#include "TutorialModules/Module-3/UI/QuickPlayWidget.h"
#include "TutorialModules/Module-3/UI/QuickPlayWidget_Starter.h"

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
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	ensure(PC);

	UTutorialModuleDataAsset* MatchmakingEssentialsModule = UTutorialModuleUtility::GetTutorialModuleDataAsset(FPrimaryAssetId("TutorialModule:MATCHMAKINGESSENTIALS"), this);
	ensure(MatchmakingEssentialsModule);

	UP2PMatchmakingSubsystem_Starter* P2PMatchmakingSubsystem = GameInstance->GetSubsystem<UP2PMatchmakingSubsystem_Starter>();
	ensure(P2PMatchmakingSubsystem);

	// Use Matchmaking's default files if starter mode is not active.
	if (!MatchmakingEssentialsModule->IsStarterModeActive())
	{
		UQuickPlayWidget* ParentWidget = Cast<UQuickPlayWidget>(BaseUIWidget->Stacks[EBaseUIStackType::Menu]->GetActiveWidget());
		ensure(ParentWidget);

		// When the cancel matchmaking clicked, handle the matchmaking cancelation through the P2P matchmaking subsystem.
		ParentWidget->OnRequestCancelMatchmaking.AddUObject(P2PMatchmakingSubsystem, &UP2PMatchmakingSubsystem_Starter::CancelMatchmaking);

		// Request matchmaking using P2P server. Match pool format: unreal-{gamemode}-p2p.
		const FString MatchPool = FString::Printf(TEXT("unreal-%s-p2p"), *ParentWidget->GetMatchGameMode());
		P2PMatchmakingSubsystem->StartMatchmaking(GetOwningPlayer(), MatchPool, FOnMatchmakingStateChangedDelegate::CreateUObject(ParentWidget, &UQuickPlayWidget::OnMatchmaking));
	}
	// Use Matchmaking's starter files if starter mode is not active.
	else
	{
		// TODO: Trigger start P2P matchmaking here.
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("P2P Matchmaking is not yet implemented."));
	}
}