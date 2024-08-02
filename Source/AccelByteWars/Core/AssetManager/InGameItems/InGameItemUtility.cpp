#include "InGameItemUtility.h"

#include "InGameItemDataAsset.h"
#include "Kismet/KismetSystemLibrary.h"

UInGameItemInterface::UInGameItemInterface(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

UInGameItemDataAsset* UInGameItemUtility::GetItemDataAsset(const FString& ItemId)
{
	UObject* Obj = UKismetSystemLibrary::GetObjectFromPrimaryAssetId(UInGameItemDataAsset::GenerateAssetIdFromId(ItemId));
	return Obj ? Cast<UInGameItemDataAsset>(Obj) : nullptr;
}

UInGameItemDataAsset* UInGameItemUtility::GetItemDataAssetBySku(const EItemSkuPlatform Platform, const FString& Sku)
{
	TArray<FPrimaryAssetId> PrimaryAssetIdList;
	UKismetSystemLibrary::GetPrimaryAssetIdList(UInGameItemDataAsset::InGameItemAssetType, PrimaryAssetIdList);
	for (const FPrimaryAssetId& AssetId : PrimaryAssetIdList)
	{
		if (UInGameItemDataAsset* Item = Cast<UInGameItemDataAsset>(UKismetSystemLibrary::GetObjectFromPrimaryAssetId(AssetId));
			Item && Item->SkuMap.Contains(Platform) && Item->SkuMap[Platform].Equals(Sku))
		{
			return Item;
		}
	}

	return nullptr;
}
