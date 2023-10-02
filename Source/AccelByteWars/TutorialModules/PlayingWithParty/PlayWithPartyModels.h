// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "UObject/NoExportTypes.h"
#include "CoreMinimal.h"

#define ACCELBYTEWARS_LOCTEXT_NAMESPACE "AccelByteWars"

#define PARTY_MATCHMAKING_STARTED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Matchmaking Started", "Party Matchmaking Started by Party Leader")
#define PARTY_MATCHMAKING_FOUND_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Matchmaking Found", "Party Matchmaking Found, Joining Match")
#define PARTY_MATCHMAKING_CANCELED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Matchmaking Canceled", "Party Matchmaking is Canceled by Party Leader")
#define PARTY_MATCHMAKING_EXPIRED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Matchmaking Expired", "Party Matchmaking Expired")
#define PARTY_MATCHMAKING_SAFEGUARD_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Matchmaking Safeguard", "Matchmaking with this game mode is not supported when in a party")

#define JOIN_PARTY_MATCH_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Join Party Match", "Joining Party Leader Match")
#define JOIN_PARTY_MATCH_SUCCESS_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Join Party Match Success", "Success to Join Party Match")
#define JOIN_PARTY_MATCH_FAILED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Join Party Match Failed", "Failed to Join Party Match")
#define JOIN_PARTY_MATCH_SAFEGUARD_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Join Party Match Safeguard", "Cannot join session. Insufficient slots to join with party")

#define PARTY_MATCH_LEADER_SAFEGUARD_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Match Leader Safeguard", "Cannot play online session since party members are on other session")
#define PARTY_MATCH_MEMBER_SAFEGUARD_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Match Member Safeguard", "Only party leader can start online session")