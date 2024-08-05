// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/InGameItems/InGameItemUtility.h"
#include "GameFramework/PlayerState.h"
#include "AccelByteWarsPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquippedItemsLoaded, APlayerState* /*Owner*/)

USTRUCT(BlueprintType)
struct FEquippedItem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere) EItemType ItemType;
	UPROPERTY(EditAnywhere) FString ItemId;
	UPROPERTY(EditAnywhere) int32 Count = 0;

	bool operator== (const FEquippedItem& Other) const
	{
		return Other.ItemType == ItemType && Other.ItemId.Equals(ItemId);
	}
};

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsPlayerState : public APlayerState
{
	GENERATED_BODY()

	//~AActor overriden functions
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ClientInitialize(AController* C) override;
	//~End of AActor overriden functions

public:
	/** @brief Called just after server load equipped item data */
	FOnEquippedItemsLoaded OnEquippedItemsLoadedDelegate;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	FString AvatarURL;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	FLinearColor TeamColor = FLinearColor::White;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 TeamId = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 MissilesFired = 0;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 KillCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 NumLivesLeft = INDEX_NONE;

	UFUNCTION()
	void RepNotify_PendingTeamAssignment();

	UFUNCTION()
	void RepNotify_EquippedItemsChanged();

	UPROPERTY(Replicated, ReplicatedUsing = "RepNotify_PendingTeamAssignment")
	bool bPendingTeamAssignment = false;

	// Number of attempt the player was almost got killed attempt in a single-lifetime
	UPROPERTY(BlueprintReadWrite, Category = Attributes, Replicated)
	int32 NumKilledAttemptInSingleLifetime = 0;

	/**
	 * @brief Get currently equipped item's ID by Item Type. Return empty string if the quantity is less than 0 or no equipped item with given type
	 * @param ItemType Equipped item's Item Type to retrieve
	 * @param bForce Set to true to ignore the quantity requirement
	 */
	UFUNCTION(BlueprintPure)
	FString GetEquippedItemId(const EItemType ItemType, const bool bForce = false);

	/**
	 * @brief Get currently equipped item by Item Type. Return empty if the quantity is less than 0 or no equipped item with given type
	 * @param ItemType Equipped item's Item Type to retrieve
	 * @param bForce Set to true to ignore the quantity requirement
	 */
	UFUNCTION(BlueprintPure)
	FEquippedItem GetEquippedItem(const EItemType ItemType, const bool bForce = false);

	/**
	 * @brief Decrease equipped item count by EItemType
	 * @param ItemType Equipped item's item type to decrease. Assuming player can only have one item equipped for each type
	 * @param DecreaseBy Decrease by how much
	 */
	UFUNCTION(BlueprintCallable)
	void DecreaseEquippedItemCount(const EItemType ItemType, int32 DecreaseBy = 1);

	/** @brief Client RPC to retrieve equipped item from its game instance and update it to player state for replication purposes. */
	UFUNCTION(Client, Reliable)
	void ClientRetrieveEquippedItems();

protected:
	/*
	 * @brief Used in server client logic only, for replication reason. Use the one in GameInstance for anything else.
	 */
	UPROPERTY(BlueprintReadWrite, Category = Attributes, ReplicatedUsing = RepNotify_EquippedItemsChanged)
	TArray<FEquippedItem> EquippedItems;

	UFUNCTION(Server, Reliable)
	void ServerUpdateEquippedItems(const TArray<FEquippedItem>& Items);

	void NotifyGameState() const;
};
