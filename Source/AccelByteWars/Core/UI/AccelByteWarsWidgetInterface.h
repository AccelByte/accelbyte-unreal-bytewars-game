// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "AccelByteWarsWidgetInterface.generated.h"

UENUM(BlueprintType)
enum class EWidgetValidationState : uint8
{
    VALID UMETA(DisplayName = "Valid"),
    INVALID UMETA(DisplayName = "Invalid"),
    VALIDATING UMETA(DisplayName = "Validating")
};

UINTERFACE(BlueprintType)
class ACCELBYTEWARS_API UAccelByteWarsWidgetInterface : public UInterface
{
    GENERATED_BODY()
};

class ACCELBYTEWARS_API IAccelByteWarsWidgetInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AccelByteWars Widget Interface")
    void ToggleHighlight(const bool bToHighlight);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AccelByteWars Widget Interface")
    void SetWidgetValidationState(const EWidgetValidationState State, const FString& StateMessage, const FString& FallbackURL);
};