// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "HAL/MemoryBase.h"

#include <vector>

class FPooledMalloc : public FMalloc {
    static constexpr auto BlockCount{1000};

public:
    static FPooledMalloc& Get(uint64_t AllocatedBytes = 0);

    FPooledMalloc(uint64_t AllocatedBytes);
    virtual void* Malloc(SIZE_T Size, uint32 Alignment) override;
    virtual void Free(void* Ptr) override;
    virtual void* Realloc(void* Ptr, SIZE_T NewSize, uint32 Alignment) override;

private:
    std::vector<uint8_t> MemoryPool{};
    size_t NextFreeAddressIndex{};

    SIZE_T GetAlignedSize(const SIZE_T& Size, int Alignment = 16);

    int FindUninitializedBlock();
    int FindFreeBlock(SIZE_T Size);
    int FindBlock(void* Address);
    void* AssignBlock(int BlockIndex, void* Address = nullptr, SIZE_T Size = 0);

    enum class BlockStatus { uninitialized, free, occupied };
    struct BlockDescriptor {
        void* Address{nullptr};
        uint64_t Size{};
        BlockStatus Status = BlockStatus::uninitialized;
    };

    std::vector<BlockDescriptor> MemoryBlocks{};
};
