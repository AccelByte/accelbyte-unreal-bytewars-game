// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineError.h"
#include "Models/AccelByteOauth2Models.h"

// @@@SNIPSTART LoginQueueModel.h-delegatemacro
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLoginQueueCancelCompleted, const APlayerController*, const FOnlineError&)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLoginQueued, const APlayerController*, const FAccelByteModelsLoginQueueTicketInfo& TicketInfo)
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnLoginTicketStatusUpdated, const APlayerController*, const FAccelByteModelsLoginQueueTicketInfo&, const FOnlineError&)
// @@@SNIPEND