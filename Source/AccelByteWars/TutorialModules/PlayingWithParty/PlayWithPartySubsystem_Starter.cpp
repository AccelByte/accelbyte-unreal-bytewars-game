// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/PlayingWithParty/PlayWithPartySubsystem_Starter.h"

#include "TutorialModules/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemUtils.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"

#include "JsonObjectConverter.h"

void UPlayWithPartySubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (GetSessionInterface())
    {
        // TODO: Bind session delegates here.
    }

    // Add party validation to online session related UIs.
    if (GetOnlineSession())
    {
        // TODO: Bind party online session validation delegates here.
    }
}

void UPlayWithPartySubsystem_Starter::Deinitialize()
{
    Super::Deinitialize();

    if (GetSessionInterface())
    {
        // TODO: Unbind session delegates here.
    }

    // Remove party validation to online session related UIs.
    if (GetOnlineSession())
    {
        // TODO: Unbind party online session validation delegates here.
    }
}

UAccelByteWarsOnlineSessionBase* UPlayWithPartySubsystem_Starter::GetOnlineSession() const
{
    if (!GetGameInstance())
    {
        return nullptr;
    }

    return Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
}

FOnlineSessionV2AccelBytePtr UPlayWithPartySubsystem_Starter::GetSessionInterface() const
{
    const UWorld* World = GetWorld();
    if (!ensure(World))
    {
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Online::GetSessionInterface(World));
}

FOnlineIdentityAccelBytePtr UPlayWithPartySubsystem_Starter::GetIdentityInterface() const
{
    const UWorld* World = GetWorld();
    if (!ensure(World))
    {
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Online::GetIdentityInterface(World));
}

UPromptSubsystem* UPlayWithPartySubsystem_Starter::GetPromptSubystem()
{
    if (UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()))
    {
        return GameInstance->GetSubsystem<UPromptSubsystem>();
    }

    return nullptr;
}

#pragma region "Playing With Party Module Function Definitions"
// TODO: Add your playing with party module function definitions here.
#pragma endregion
