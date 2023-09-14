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
		PendingNotifications.Add(Notification);
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
	NotificationTimers.Add(Notification, &TimerHandle);

	// Start notification lifetime.
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnNotificationLifeTimeEnds, Notification), NotificationLifeTime, false);
}

void UPushNotificationWidget::RemoveNotification(UPushNotification* Notification)
{
	// Delete from pending notifications.
	if (PendingNotifications.Contains(Notification)) 
	{
		PendingNotifications.Remove(Notification);
	}

	// Delete notification timer.
	if (NotificationTimers.Contains(Notification)) 
	{
		FTimerHandle* NotificationTimer = NotificationTimers[Notification];
		GetWorld()->GetTimerManager().ClearTimer(*NotificationTimer);
		NotificationTimers.Remove(Notification);
	}

	// Delete from notification list.
	Lv_PushNotification->RemoveItem(Notification);
	Lv_PushNotification->RequestRefresh();

	// Dismiss the notification if empty.
	if (PendingNotifications.IsEmpty() && Lv_PushNotification->GetNumItems() <= 0)
	{
		DeactivateWidget();
	}
}

void UPushNotificationWidget::TryPushPendingNotifications()
{
	// Push pending notifications.
	int32 MaxToPush = MaxNotificationStack - Lv_PushNotification->GetNumItems();
	for (int32 i = 0; i < MaxToPush; i++)
	{
		if (PendingNotifications.IsEmpty())
		{
			break;
		}

		PushNotification(PendingNotifications[0]);
		PendingNotifications.RemoveAt(0);
	}
}

void UPushNotificationWidget::OnNotificationLifeTimeEnds(UPushNotification* Notification)
{
	RemoveNotification(Notification);
	TryPushPendingNotifications();
}

void UPushNotificationWidget::DismissAllNotifications()
{
	// Clear pending notifications.
	PendingNotifications.Empty();

	// Clear dangling notification timers.
	for (const auto& NotificationTimer : NotificationTimers)
	{
		if (NotificationTimer.Value)
		{
			GetWorld()->GetTimerManager().ClearTimer(*NotificationTimer.Value);
		}
	}

	NotificationTimers.Empty();
}
