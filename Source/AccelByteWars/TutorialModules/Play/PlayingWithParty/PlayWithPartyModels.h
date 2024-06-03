// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "UObject/NoExportTypes.h"
#include "CoreMinimal.h"

#define PARTY_MEMBERS_GAME_SESSION_ID "PARTY_MEMBERS_GAME_SESSION_ID"

#define ACCELBYTEWARS_LOCTEXT_NAMESPACE "AccelByteWars"

#define PARTY_MATCHMAKING_STARTED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Matchmaking Started", "Party Matchmaking Started by Party Leader")
#define PARTY_MATCHMAKING_SUCCESS_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Matchmaking Success", "Party Matchmaking Found, Joining Match")
#define PARTY_MATCHMAKING_FAILED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Matchmaking Failed", "Party Matchmaking failed")
#define PARTY_MATCHMAKING_CANCELED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Matchmaking Canceled", "Party Matchmaking is canceled by party leader")
#define PARTY_MATCHMAKING_EXPIRED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Matchmaking Expired", "Party Matchmaking expired")
#define PARTY_MATCHMAKING_SAFEGUARD_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Matchmaking Safeguard", "Matchmaking with this game mode is not supported when in a party")

#define JOIN_PARTY_GAME_SESSION_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Join Party Game Session", "Joining Party Leader Game Session")
#define JOIN_PARTY_GAME_SESSION_FAILED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Join Party Game Session Failed", "Failed to Join Party Game Session")
#define JOIN_PARTY_GAME_SESSION_CANCELED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Join Party Game Session Canceled", "Party game session is canceled by party leader")
#define JOIN_PARTY_GAME_SESSION_WAIT_SERVER_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Join Party Game Session Wait Server", "Joined Party Game Session. Waiting for Server")
#define JOIN_PARTY_GAME_SESSION_SERVER_ERROR_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Join Party Game Session Server Error", "Party Game Session failure. Cannot find game server.")
#define JOIN_PARTY_GAME_SESSION_SAFEGUARD_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Join Party Game Session Safeguard", "Cannot join session. Insufficient slots to join with party")

#define PARTY_GAME_SESSION_LEADER_SAFEGUARD_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Game Session Leader Safeguard", "Cannot play online session since party members are on other session")
#define PARTY_GAME_SESSION_MEMBER_SAFEGUARD_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Game Session Member Safeguard", "Only party leader can start online session")