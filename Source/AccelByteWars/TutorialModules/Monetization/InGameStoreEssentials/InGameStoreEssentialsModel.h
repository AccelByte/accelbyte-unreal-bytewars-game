﻿// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "Core/UI/MainMenu/Store/StoreItemModel.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"

DECLARE_DELEGATE_OneParam(FOnGetOrQueryOffersByCategory, TArray<UStoreItemDataObject*> /*Offers*/)
DECLARE_DELEGATE_OneParam(FOnGetOrQueryOfferById, UStoreItemDataObject* /*Offer*/)
DECLARE_DELEGATE_OneParam(FOnGetOrQueryCategories, TArray<FOnlineStoreCategory> /*Category*/)