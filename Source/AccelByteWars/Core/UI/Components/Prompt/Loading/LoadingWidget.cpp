// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/Prompt/Loading/LoadingWidget.h"
#include "Components/TextBlock.h"

void ULoadingWidget::SetLoadingMessage(const FText& LoadingMessage)
{
	Tb_LoadingMessage->SetText(LoadingMessage);
}

void ULoadingWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// disable back button, since this is a loading screen
	bIsBackHandler = false;
	GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown;
}
