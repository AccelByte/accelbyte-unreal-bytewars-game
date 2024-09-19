// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PushNotificationWidget.h"
#include "Components/ListView.h"
#include "CommonButtonBase.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

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

void UPushNotificationWidget::PushNotification(UObject* Notification)
{
	if (!Notification) 
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot push notification UI. The notification object is null."));
		return;
	}

	if (!Lv_PushNotification) 
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot push notification UI. List view is null."));
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

	// Start notification lifetime.
	FTimerHandle TimerHandle;
	NotificationTimers.Add(Notification, TimerHandle);

	// Start notification lifetime.
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnNotificationLifeTimeEnds, Notification), NotificationLifeTime, false);
}

void UPushNotificationWidget::RemoveNotification(UObject* Notification)
{
	if (!Notification)
	{
		return;
	}

	// Delete from pending notifications.
	if (PendingNotifications.Contains(Notification)) 
	{
		PendingNotifications.Remove(Notification);
	}

	// Delete notification timer.
	if (NotificationTimers.Contains(Notification)) 
	{
		FTimerHandle* NotificationTimer = &NotificationTimers[Notification];
		GetWorld()->GetTimerManager().ClearTimer(*NotificationTimer);
		NotificationTimers.Remove(Notification);
	}

	
	if ((!bRequireWidgetActivation || IsActivated()) && !IsUnreachable())
	{
		// Delete from notification list.
		if (!Lv_PushNotification->IsUnreachable() &&
			Lv_PushNotification->GetIndexForItem(Notification) != INDEX_NONE)
		{
			Lv_PushNotification->RemoveItem(Notification);
		}

		// Dismiss the notification if empty.
		if (PendingNotifications.IsEmpty() &&
			Lv_PushNotification->GetNumItems() <= 0)
		{
			DeactivateWidget();
		}
	}
}

void UPushNotificationWidget::TryPushPendingNotifications()
{
	if ((bRequireWidgetActivation && !IsActivated()) || IsUnreachable())
	{
		UE_LOG_ACCELBYTEWARSACTIVATABLEWIDGET(Warning, TEXT("Cannot access the push notification UI as the widget begin to tear down."));
		return;
	}

	// Push pending notifications.
	const int32 MaxToPush = MaxNotificationStack - Lv_PushNotification->GetNumItems();
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

void UPushNotificationWidget::OnNotificationLifeTimeEnds(UObject* Notification)
{
	RemoveNotification(Notification);
	TryPushPendingNotifications();
}

void UPushNotificationWidget::DismissAllNotifications()
{
	// Clear pending notifications.
	PendingNotifications.Empty();

	// Clear dangling notification timers.
	for (TTuple<UObject*, FTimerHandle>& NotificationTimer : NotificationTimers)
	{
		if (NotificationTimer.Value.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(NotificationTimer.Value);
		}
	}

	NotificationTimers.Empty();
}
