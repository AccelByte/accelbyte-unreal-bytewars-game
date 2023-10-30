// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/Player/CommonLocalPlayer.h"
#include "Core/UI/GameUIManagerSubsystem.h"
#include "Core/UI/GameUIController.h"
#include "GameFramework/PlayerController.h"

UCommonLocalPlayer::UCommonLocalPlayer()
	: Super(FObjectInitializer::Get())
{
}

FDelegateHandle UCommonLocalPlayer::CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());

	if (PC)
	{
		Delegate.Execute(this, PC);
	}

	return OnPlayerControllerSet.Add(Delegate);
}

FDelegateHandle UCommonLocalPlayer::CallAndRegister_OnPlayerStateSet(FPlayerStateSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APlayerState* PlayerState = PC ? PC->PlayerState : nullptr;

	if (PlayerState)
	{
		Delegate.Execute(this, PlayerState);
	}
	
	return OnPlayerStateSet.Add(Delegate);
}

FDelegateHandle UCommonLocalPlayer::CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate)
{
	APlayerController* PC = GetPlayerController(GetWorld());
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	if (Pawn)
	{
		Delegate.Execute(this, Pawn);
	}

	return OnPlayerPawnSet.Add(Delegate);
}

bool UCommonLocalPlayer::GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const
{
	if (!bIsPlayerViewEnabled)
	{
		return false;
	}

	return Super::GetProjectionData(Viewport, ProjectionData, StereoViewIndex);
}

UAccelByteWarsBaseUI* UCommonLocalPlayer::GetRootUILayout() const
{
	if (UGameUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UGameUIManagerSubsystem>())
	{
		if (UGameUIController* Policy = UIManager->GetCurrentUIController())
		{
			return Policy->GetRootLayout(this);
		}
	}

	return nullptr;
}