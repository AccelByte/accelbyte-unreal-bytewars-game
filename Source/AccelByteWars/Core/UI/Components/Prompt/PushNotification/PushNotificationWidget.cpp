// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/Components/Prompt/PushNotification/PushNotificationWidget.h"
#include "Components/ListView.h"
#include "CommonButtonBase.h"

void UPushNotificationWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_Dismiss->OnClicked().AddUObject(this, &ThisClass::DeactivateWidget);
	Lv_PushNotification->ClearListItems();
	PendingNotifications.Empty();
}

void UPushNotificationWidget::NativeOnDeactivated()
{
	Btn_Dismiss->OnClicked().Clear();
	DismissAllNotifications();

	Super::NativeOnDeactivated();
}

void UPushNotificationWidget::PushNotification(UPushNotification* Notification)
{
	if (!Notification) 
	{
		return;
	}

	// Mark as pending notification if the max stack is reached.
	if ((uint32)Lv_PushNotification->GetNumItems() >= MaxNotificationStack)
	{
		PendingNotifications.Enqueue(Notification);
		return;
	}

	// Insert new notification at the first index.
	TArray<UObject*> LastNotifications = Lv_PushNotification->GetListItems();
	LastNotifications.Insert(Notification, 0);
	Lv_PushNotification->SetListItems(LastNotifications);

	// Focus the cursor to the new notification entry.
	Lv_PushNotification->SetUserFocus(GetOwningPlayer());

	// Start notification lifetime.
	FTimerHandle TimerHandle;
	NotificationTimers.Add(&TimerHandle);

	// Start notification lifetime.
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnNotificationLifeTimeEnds, Notification, &TimerHandle), NotificationLifeTime, false);
}

void UPushNotificationWidget::OnNotificationLifeTimeEnds(UPushNotification* Notification, FTimerHandle* NotificationTimer)
{
	// Remove notification.
	if (Notification) 
	{
		Lv_PushNotification->RemoveItem(Notification);
	}

	// Clear notification timer.
	if (NotificationTimer)
	{
		GetWorld()->GetTimerManager().ClearTimer(*NotificationTimer);
		NotificationTimers.Remove(NotificationTimer);
	}

	// Push pending notification if any.
	TryPushPendingNotifications();
}

void UPushNotificationWidget::TryPushPendingNotifications()
{
	// If no notifications left, deactivate the widget.
	if (!PendingNotifications.Peek())
	{
		if (Lv_PushNotification->GetNumItems() <= 0) 
		{
			DeactivateWidget();
		}
		return;
	}

	// Push pending notifications.
	int32 MaxToPush = MaxNotificationStack - Lv_PushNotification->GetNumItems();
	for (int32 i = 0; i < MaxToPush; i++)
	{
		if (PendingNotifications.IsEmpty())
		{
			break;
		}

		PushNotification(*PendingNotifications.Peek());
		PendingNotifications.Pop();
	}
}

void UPushNotificationWidget::DismissAllNotifications()
{
	// Clear pending notifications.
	PendingNotifications.Empty();

	// Clear dangling notification timers.
	for (FTimerHandle* NotificationTimer : NotificationTimers)
	{
		if (NotificationTimer)
		{
			GetWorld()->GetTimerManager().ClearTimer(*NotificationTimer);
		}
	}
	NotificationTimers.Empty();
}
