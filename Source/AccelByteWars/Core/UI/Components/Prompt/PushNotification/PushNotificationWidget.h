// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "PushNotificationWidget.generated.h"

class UCommonButtonBase;
class UListView;

UCLASS()
class ACCELBYTEWARS_API UPushNotificationWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	void PushNotification(UObject* Notification);
	void RemoveNotification(UObject* Notification);
	void DismissAllNotifications();

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void TryPushPendingNotifications();
	void OnNotificationLifeTimeEnds(UObject* Notification);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_PushNotification;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Dismiss;

	UPROPERTY(EditDefaultsOnly)
	uint32 NotificationLifeTime = 10;

	UPROPERTY(EditDefaultsOnly)
	uint32 MaxNotificationStack = 5;

	UPROPERTY(EditDefaultsOnly)
	bool bRequireWidgetActivation = true;

	UPROPERTY()
	TArray<UObject*> PendingNotifications;

	UPROPERTY()
	TMap<UObject*, FTimerHandle> NotificationTimers;
};
