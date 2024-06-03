// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/Prompt/Loading/ReconnectingWidget.h"
#include "Components/TextBlock.h"

void UReconnectingWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// disable back button, since this is a loading screen
	bIsBackHandler = false;
	GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown;
}
