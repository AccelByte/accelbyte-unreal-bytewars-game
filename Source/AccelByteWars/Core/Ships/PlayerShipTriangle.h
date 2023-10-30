// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/Ships/PlayerShipBase.h"
#include "PlayerShipTriangle.generated.h"

UCLASS()
class ACCELBYTEWARS_API APlayerShipTriangle : public APlayerShipBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APlayerShipTriangle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief Current ship color, set only on spawn
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_Color)
		FLinearColor Color = FLinearColor::Yellow;

	/**
	 * @brief Lets other players know when a player ship is changing their ship color on spawn
	 */
	UFUNCTION(BlueprintCallable, Server, Unreliable, Category = AccelByteWars)
		void Server_SetColor(FLinearColor InColor);

	/**
	 * @brief Generic OnRep notify for color update
	 */
	UFUNCTION()
		void OnRepNotify_Color();

	/**
	 * @brief Generic Sets the ship color
	 */
	UFUNCTION()
		virtual void SetShipColor(FLinearColor InColor) override { Server_SetColor(InColor); }

};
