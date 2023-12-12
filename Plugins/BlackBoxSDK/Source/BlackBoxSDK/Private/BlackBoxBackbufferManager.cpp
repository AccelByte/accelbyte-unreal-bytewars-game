// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "BlackBoxBackbufferManager.h"

#include "BlackBoxCommon.h"
#include "BlackBoxLog.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/accelbyte/cpp/blackbox.h"

#if BLACKBOX_UE_WINDOWS
//   DX11
#    if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1

#        include "Windows/WindowsHWrapper.h"

// Disable macro redefinition warning for compatibility with Windows SDK 8+
#        pragma warning(push)
#        pragma warning(disable : 4005) // macro redefinition

// D3D headers.
#        if PLATFORM_64BITS
#            pragma pack(push, 16)
#        else
#            pragma pack(push, 8)
#        endif
#        define D3D_OVERLOADS 1
#        include "Windows/AllowWindowsPlatformTypes.h"
// D3D11.h must always after AllowWindowsPlatformTypes.h
#        include <D3D11.h>
// D3D11.h must always before HideWindowsPlatformTypes.h
#        include "Windows/HideWindowsPlatformTypes.h"

#        undef DrawText

#        pragma pack(pop)
#        pragma warning(pop)

#        include "Runtime/Windows/D3D11RHI/Public/ID3D11DynamicRHI.h"

#    else
#        include "Runtime/Windows/D3D11RHI/Private/Windows/D3D11RHIBasePrivate.h"
// D3D11StateCachePrivate.h must always after D3D11RHIBasePrivate.h
#        include "Runtime/Windows/D3D11RHI/Private/D3D11StateCachePrivate.h"
#    endif
#    include "Runtime/Windows/D3D11RHI/Public/D3D11State.h"
// D3D11Resources.h Must always after D3D11State.h
#    include "Runtime/Windows/D3D11RHI/Public/D3D11Resources.h"

//   DX12
#    if ENGINE_MAJOR_VERSION < 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 2)
#        include "Runtime/D3D12RHI/Private/D3D12RHIPrivate.h"
#        include "Runtime/D3D12RHI/Private/D3D12Texture.h"
#    else
#        include "Runtime/D3D12RHI/Private/D3D12RHICommon.h"
#        include "Runtime/D3D12RHI/Public/ID3D12DynamicRHI.h"
#    endif
#endif

FBackbufferManager::FBackbufferManager()
{
#if BLACKBOX_UE_WINDOWS
    if (FString(GDynamicRHI->GetName()) == TEXT("D3D11")) {
        UE_LOG(LogBlackBox, Log, TEXT("Initialize Using DirectX 11 Renderer"));
        CurrentRenderingAPI = blackbox::graphics_api::DX11;
    }
    else if (FString(GDynamicRHI->GetName()) == TEXT("D3D12")) {
        UE_LOG(LogBlackBox, Log, TEXT("Initialize Using DirectX 12 Renderer"));
        CurrentRenderingAPI = blackbox::graphics_api::DX12;
    }
    else {
        UE_LOG(LogBlackBox, Log, TEXT("Initialize Using Null Renderer"));
        CurrentRenderingAPI = blackbox::graphics_api::NULL_IMPL;
    }
#else
    UE_LOG(LogBlackBox, Log, TEXT("Initialize Using Null Renderer"));
    CurrentRenderingAPI = blackbox::graphics_api::NULL_IMPL;
#endif
}

FBackbufferManager::~FBackbufferManager()
{
    CurrentRenderingAPI = blackbox::graphics_api::NULL_IMPL;
    Active = false;
}

void FBackbufferManager::RegisterBackbufferCallback()
{
    if (Active) {
        return;
    }

    if (CurrentRenderingAPI == blackbox::graphics_api::NULL_IMPL) {
        Active = true;
        return;
    }

#if BLACKBOX_UE_WINDOWS
    // Setting up delegate and callback from engine
    if (FSlateApplication::IsInitialized()) {
        OnBackBufferReadyDelegate = FSlateApplication::Get().GetRenderer()->OnBackBufferReadyToPresent().AddRaw(
            this, &FBackbufferManager::OnBackBufferReady_RenderThread);
        OnSlateWindowReadyDelegate = FSlateApplication::Get().GetRenderer()->OnSlateWindowRendered().AddRaw(
            this, &FBackbufferManager::OnSlateWindowRendered_GameThread);
        Active = true;
    }
    else {
        if (IsRunningDedicatedServer()) {
            UE_LOG(
                LogBlackBox,
                Log,
                TEXT("Running as server entity, no viewport is present. Cannot bind to OnBackBufferReadyToPresent "
                     "delegate. No video will be recorded"));
        }
        else {
            UE_LOG(
                LogBlackBox,
                Log,
                TEXT("Running as client entity, no viewport is present. Cannot bind to OnBackBufferReadyToPresent "
                     "delegate. No video will be recorded"));
        }
    }
#endif // BLACKBOX_UE_WINDOWS
}

void FBackbufferManager::UnregisterBackbufferCallback()
{
#if BLACKBOX_UE_WINDOWS
    if (FSlateApplication::IsInitialized()) {
        if (OnSlateWindowReadyDelegate.IsValid()) {
            FSlateApplication::Get().GetRenderer()->OnSlateWindowRendered().Remove(OnSlateWindowReadyDelegate);
            OnSlateWindowReadyDelegate = FDelegateHandle();
        }
        if (OnBackBufferReadyDelegate.IsValid()) {
            FSlateApplication::Get().GetRenderer()->OnBackBufferReadyToPresent().Remove(OnBackBufferReadyDelegate);
            OnBackBufferReadyDelegate = FDelegateHandle();
        }
    }
    Active = false;
    TgtWindowPtr = nullptr;
#endif
}

void FBackbufferManager::OnSlateWindowRendered_GameThread(SWindow& SlateWindow, void* RHIViewport)
{
#if BLACKBOX_UE_WINDOWS
    
    if (!bbx_config_get_store_crash_video() &&
        !(bbx_config_get_enable_issue_reporter() && !bbx_config_get_use_engine_to_capture_screenshot())) {
        return;
    }

    if ((SlateWindow.GetType() != EWindowType::Normal) && (SlateWindow.GetType() != EWindowType::GameWindow)) {
        return;
    }

    if (!SlateWindow.IsActive()) {
        return;
    }
    TgtWindowPtr = &SlateWindow;
#endif
}

void FBackbufferManager::OnBackBufferReady_RenderThread(SWindow& SlateWindow, const FTexture2DRHIRef& BackBuffer)
{
#if BLACKBOX_UE_WINDOWS
    if ((&SlateWindow != TgtWindowPtr) || (TgtWindowPtr == nullptr)) {
        return;
    }

    const auto BackBufferRHITexture2DPtr = BackBuffer.GetReference();
    if (!BackBufferRHITexture2DPtr) {
        return;
    }

    switch (CurrentRenderingAPI) {
    case blackbox::graphics_api::DX11: {
        FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();
        RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
#    if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
        const auto BackBufferNativeD11Texture2DPtr = GetID3D11DynamicRHI()->RHIGetResource(BackBufferRHITexture2DPtr);
        if (!BackBufferNativeD11Texture2DPtr) {
            return;
        }
#    else
        const auto BackBufferD11Texture2DPtr =
            static_cast<FD3D11Texture2D*>(GetD3D11TextureFromRHITexture(BackBufferRHITexture2DPtr));
        if (!BackBufferD11Texture2DPtr) {
            return;
        }

        const auto BackBufferNativeD11Texture2DPtr =
            static_cast<ID3D11Texture2D*>(BackBufferD11Texture2DPtr->GetNativeResource());
        if (!BackBufferNativeD11Texture2DPtr) {
            return;
        }
#    endif
        bbx_update_backbuffer_dx11_texture(BackBufferNativeD11Texture2DPtr);
    } break;
    case blackbox::graphics_api::DX12: {
#    if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
        const auto BackBufferNativeD12Texture2DPtr = GetID3D12DynamicRHI()->RHIGetResource(BackBufferRHITexture2DPtr);
        if (!BackBufferNativeD12Texture2DPtr) {
            return;
        }
#    else
        const auto BackBufferD12Texture2DPtr =
            static_cast<FD3D12Texture2D*>(GetD3D12TextureFromRHITexture(BackBufferRHITexture2DPtr));
        if (!BackBufferD12Texture2DPtr) {
            return;
        }

        const auto BackBufferNativeD12Texture2DPtr =
            static_cast<ID3D12Resource*>(BackBufferD12Texture2DPtr->GetNativeResource());
        if (!BackBufferNativeD12Texture2DPtr) {
            return;
        }
#    endif
        bbx_update_backbuffer_dx12_texture(BackBufferNativeD12Texture2DPtr);
    } break;
    case blackbox::graphics_api::NULL_IMPL: {
        // Do nothing
    } break;
    default: {
        check(false);
    } break;
    }
#endif
}

blackbox::graphics_api FBackbufferManager::GetActiveRenderingAPI()
{
    return CurrentRenderingAPI;
}

bool FBackbufferManager::GetIsActive() noexcept
{
    return Active;
}
