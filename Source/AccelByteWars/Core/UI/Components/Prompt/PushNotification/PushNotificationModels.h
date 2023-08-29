#pragma once

#include "UObject/NoExportTypes.h"
#include "CoreMinimal.h"
#include "PushNotificationModels.generated.h"

UENUM(BlueprintType)
enum class EPushNotificationActionResult : uint8
{
	Button1 = 0,
	Button2,
	Button3
};

DECLARE_DELEGATE_OneParam(FPushNotificationDelegate, EPushNotificationActionResult /* ActionButtonResult */);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPushNotificationDynamicDelegate, EPushNotificationActionResult, ActionButtonResult);

UCLASS()
class ACCELBYTEWARS_API UPushNotification : public UObject
{
	GENERATED_BODY()

public:
	FString IconImageURL;
	FText Message;
	TArray<FText> ActionButtonTexts;
	FPushNotificationDelegate ActionButtonCallback;
	FPushNotificationDynamicDelegate ActionButtonDynamicCallback;
};
