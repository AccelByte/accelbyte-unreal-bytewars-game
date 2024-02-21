// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

class FOnlineUserPresenceAccelByte;

DECLARE_DELEGATE_TwoParams(FOnPresenceTaskComplete, const bool /*bWasSuccessful*/, const TSharedPtr<FOnlineUserPresenceAccelByte> /*Presence*/);

#define ACCELBYTEWARS_LOCTEXT_NAMESPACE "AccelByteWars"

#define TEXT_PRESENCE_ONLINE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_online", "Online")
#define TEXT_PRESENCE_OFFLINE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_offline", "Offline")

#define TEXT_PRESENCE_LAST_ONLINE_YEARS NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_last_online_years_ago", "Last Online Years Ago")
#define TEXT_PRESENCE_LAST_ONLINE_MONTHS NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_last_online_months_ago", "Last Online {0} Month(s) Ago")
#define TEXT_PRESENCE_LAST_ONLINE_DAYS NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_last_online_days_ago", "Last Online {0} Day(s) Ago")
#define TEXT_PRESENCE_LAST_ONLINE_HOURS NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_last_online_hours_ago", "Last Online {0} Hour(s) Ago")
#define TEXT_PRESENCE_LAST_ONLINE_MINUTES NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_last_online_minutes_ago", "Last Online {0} Minute(s) Ago")
#define TEXT_PRESENCE_LAST_ONLINE_AWHILE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_last_online_while_ago", "Last Online a While Ago")

#define TEXT_PRESENCE_LEVEL_MAINMENU NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_level_main_menu", "In Main Menu")
#define TEXT_PRESENCE_LEVEL_GAMEPLAY NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_level_gameplay", "In Match")
#define TEXT_PRESENCE_ACTIVITY_PARTY NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_activity_party", "In Party")
#define TEXT_PRESENCE_ACTIVITY_MATCHMAKING NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_activity_matchmaking", "Matchmaking")
#define TEXT_PRESENCE_ACTIVITY_LOBBY NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_activity_lobby", "Lobby")
#define TEXT_PRESENCE_ACTIVITY_GAMEPLAY NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_activity_gameplay", "Game Mode: {0}")
#define TEXT_PRESENCE_ACTIVITY_UNKNOWN NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "presence_activity_unknown", "Unknown Activity")

inline static FString GetLastOnline(const FDateTime LastOnline) 
{
    // Only check last online within a year.
    const FDateTime CurrentTime = FDateTime::UtcNow();
    if (CurrentTime.GetYear() != LastOnline.GetYear())
    {
        return TEXT_PRESENCE_LAST_ONLINE_YEARS.ToString();
    }

    // Check last online in months.
    if (CurrentTime.GetMonth() > LastOnline.GetMonth())
    {
        const int32 Months = CurrentTime.GetMonth() - LastOnline.GetMonth();
        return FText::Format(TEXT_PRESENCE_LAST_ONLINE_MONTHS, Months).ToString();
    }

    // Check last online in days.
    if (CurrentTime.GetDay() > LastOnline.GetDay())
    {
        const int32 Days = CurrentTime.GetDay() - LastOnline.GetDay();
        return FText::Format(TEXT_PRESENCE_LAST_ONLINE_DAYS, Days).ToString();
    }

    // Check last online in hours.
    if (CurrentTime.GetHour() > LastOnline.GetHour())
    {
        const int32 Hours = CurrentTime.GetHour() - LastOnline.GetHour();
        return FText::Format(TEXT_PRESENCE_LAST_ONLINE_HOURS, Hours).ToString();
    }

    // Check last online in minutes.
    if (CurrentTime.GetMinute() > LastOnline.GetMinute())
    {
        const int32 Minutes = CurrentTime.GetMinute() - LastOnline.GetMinute();
        return FText::Format(TEXT_PRESENCE_LAST_ONLINE_MINUTES, Minutes).ToString();
    }

    return TEXT_PRESENCE_LAST_ONLINE_AWHILE.ToString();
}