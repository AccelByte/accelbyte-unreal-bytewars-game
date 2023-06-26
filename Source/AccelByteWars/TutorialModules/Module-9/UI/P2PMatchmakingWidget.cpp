// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-9/UI/P2PMatchmakingWidget.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"

#include "TutorialModules/Module-9/P2PMatchmakingSubsystem.h"
#include "TutorialModules/Module-3/UI/QuickPlayWidget.h"
#include "TutorialModules/Module-3/UI/QuickPlayWidget_Starter.h"

void UP2PMatchmakingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_StartP2PMatchmaking->OnClicked().AddUObject(this, &ThisClass::OnStartP2PMatchmakingButtonClicked);
}

void UP2PMatchmakingWidget::NativeDestruct()
{
	Super::NativeDestruct();

	Btn_StartP2PMatchmaking->OnClicked().Clear();
}

void UP2PMatchmakingWidget::OnStartP2PMatchmakingButtonClicked()
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	UAccelByteWarsBaseUI* BaseUIWidget = GameInstance->GetBaseUIWidget();
	ensure(BaseUIWidget);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	ensure(PC);

	UTutorialModuleDataAsset* MatchmakingEssentialsModule = UTutorialModuleUtility::GetTutorialModuleDataAsset(FPrimaryAssetId("TutorialModule:MATCHMAKINGESSENTIALS"), this);
	ensure(MatchmakingEssentialsModule);

	UP2PMatchmakingSubsystem* P2PMatchmakingSubsystem = GameInstance->GetSubsystem<UP2PMatchmakingSubsystem>();
	ensure(P2PMatchmakingSubsystem);

	// Use Matchmaking's default files if starter mode is not active.
	if (!MatchmakingEssentialsModule->IsStarterModeActive())
	{
		UQuickPlayWidget* ParentWidget = Cast<UQuickPlayWidget>(BaseUIWidget->Stacks[EBaseUIStackType::Menu]->GetActiveWidget());
		ensure(ParentWidget);

		// When the cancel matchmaking clicked, handle the matchmaking cancelation through the P2P matchmaking subsystem.
		ParentWidget->OnRequestCancelMatchmaking.AddUObject(P2PMatchmakingSubsystem, &UP2PMatchmakingSubsystem::CancelMatchmaking);

		// Request matchmaking using P2P server. Match pool format: unreal-{gamemode}-p2p.
		const FString MatchPool = FString::Printf(TEXT("unreal-%s-p2p"), *ParentWidget->GetMatchGameMode());
		P2PMatchmakingSubsystem->StartMatchmaking(GetOwningPlayer(), MatchPool, FOnMatchmakingStateChangedDelegate::CreateUObject(ParentWidget, &UQuickPlayWidget::OnMatchmaking));
	}
	// Use Matchmaking's starter files if starter mode is not active.
	else 
	{
		UQuickPlayWidget_Starter* ParentWidget = Cast<UQuickPlayWidget_Starter>(BaseUIWidget->Stacks[EBaseUIStackType::Menu]->GetActiveWidget());
		ensure(ParentWidget);

		// When the cancel matchmaking clicked, handle the matchmaking cancelation through the P2P matchmaking subsystem.
		ParentWidget->OnRequestCancelMatchmaking.AddUObject(P2PMatchmakingSubsystem, &UP2PMatchmakingSubsystem::CancelMatchmaking);

		// Request matchmaking using P2P server. Match pool format: unreal-{gamemode}-p2p.
		const FString MatchPool = FString::Printf(TEXT("unreal-%s-p2p"), *ParentWidget->GetMatchGameMode());
		P2PMatchmakingSubsystem->StartMatchmaking(GetOwningPlayer(), MatchPool, FOnMatchmakingStateChangedDelegate::CreateUObject(ParentWidget, &UQuickPlayWidget_Starter::OnMatchmaking));
	}
}