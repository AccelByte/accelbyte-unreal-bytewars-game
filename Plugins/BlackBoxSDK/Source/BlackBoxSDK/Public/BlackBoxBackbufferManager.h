// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "Core/accelbyte/cpp/utils/graphics_api_list.h"
#include "Engine.h"
#include "Engine/GameEngine.h"
#include "Misc/CoreDelegates.h"

class FBackbufferManager {
public:
    FBackbufferManager();
    ~FBackbufferManager();
    void RegisterBackbufferCallback();
    void UnregisterBackbufferCallback();
    blackbox::graphics_api GetActiveRenderingAPI();
    bool GetIsActive() noexcept;

private:
    void OnBackBufferReady_RenderThread(SWindow& SlateWindow, const FTexture2DRHIRef& BackBuffer);
    void OnSlateWindowRendered_GameThread(SWindow& SlateWindow, void* RHIViewport);

private:
    bool Active = false;
    FDelegateHandle OnBackBufferReadyDelegate{};
    FDelegateHandle OnSlateWindowReadyDelegate{};
    TAtomic<void*> TgtWindowPtr{};
    blackbox::graphics_api CurrentRenderingAPI = blackbox::graphics_api::NULL_IMPL;
};