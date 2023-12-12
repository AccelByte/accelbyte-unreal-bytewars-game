// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PooledMalloc.h"

FPooledMalloc& FPooledMalloc::Get(uint64_t AllocatedBytes)
{
    auto constexpr DefaultAllocatedBytes{20 * 1024 * 1024};
    static FPooledMalloc PooledMallocObject{AllocatedBytes > 0 ? AllocatedBytes : DefaultAllocatedBytes};
    return PooledMallocObject;
}

FPooledMalloc::FPooledMalloc(uint64_t AllocatedBytes)
{
    MemoryPool.resize(AllocatedBytes, 0);
    MemoryBlocks.resize(BlockCount);
}

void* FPooledMalloc::Malloc(SIZE_T Size, uint32 Alignment)
{
    auto AlignedSize = GetAlignedSize(Size, Alignment);
    void* NewMemoryAdddress{nullptr};
    if (AlignedSize + NextFreeAddressIndex <= MemoryPool.size()) {
        auto block_index = FindUninitializedBlock();
        if (block_index != -1) {
            NewMemoryAdddress = static_cast<void*>(MemoryPool.data() + NextFreeAddressIndex);
            AssignBlock(block_index, NewMemoryAdddress, AlignedSize);
            NextFreeAddressIndex += AlignedSize;
        }
    }
    else {
        auto FreeBlock = FindFreeBlock(AlignedSize);
        if (FreeBlock != -1) {
            NewMemoryAdddress = AssignBlock(FreeBlock);
        }
    }
    return NewMemoryAdddress;
}

void FPooledMalloc::Free(void* Ptr)
{
    auto BlockIndex = FindBlock(Ptr);
    if (BlockIndex != -1) {
        auto& FreedBlock = MemoryBlocks[BlockIndex];
        FreedBlock.Status = BlockStatus::free;
    }
}

void* FPooledMalloc::Realloc(void* Ptr, SIZE_T NewSize, uint32 Alignment)
{
    if (!Ptr) {
        return Malloc(NewSize, Alignment);
    }
    auto AlignedNewSize = GetAlignedSize(NewSize, Alignment);
    void* NewAddress = nullptr;
    auto FoundBlockIndex = FindBlock(Ptr);
    if (FoundBlockIndex != -1 && MemoryBlocks[FoundBlockIndex].Size >= AlignedNewSize) {
        return MemoryBlocks[FoundBlockIndex].Address;
    }
    else {
        NewAddress = Malloc(AlignedNewSize, Alignment);
        if (!NewAddress) {
            return NewAddress;
        }
        memcpy(NewAddress, Ptr, AlignedNewSize);
    }

    if (NewAddress && NewAddress != Ptr) {
        Free(Ptr);
    }

    return NewAddress;
}

SIZE_T FPooledMalloc::GetAlignedSize(const SIZE_T& Unaligned, int Alignment)
{
    return (SIZE_T)(((uint64)Unaligned + Alignment - 1) & ~(Alignment - 1));
}

int FPooledMalloc::FindUninitializedBlock()
{
    for (auto i = 0; i < MemoryBlocks.size(); i++) {
        const auto& block = MemoryBlocks[i];
        if (block.Status == BlockStatus::uninitialized) {
            return i;
        }
    }
    return -1;
}

int FPooledMalloc::FindFreeBlock(SIZE_T Size)
{
    for (auto i = 0; i < MemoryBlocks.size(); i++) {
        const auto& Block = MemoryBlocks[i];
        if (Block.Status == BlockStatus::free && Block.Size <= Size) {
            return i;
        }
    }
    return -1;
}

int FPooledMalloc::FindBlock(void* Address)
{
    if (!Address) {
        return -1;
    }

    for (auto i = 0; i < MemoryBlocks.size(); i++) {
        const auto& block = MemoryBlocks[i];
        if (block.Address == Address) {
            return i;
        }
    }
    return -1;
}

void* FPooledMalloc::AssignBlock(int BlockIndex, void* Addr, SIZE_T Size)
{
    auto index = static_cast<size_t>(BlockIndex);
    if (Addr) {
        MemoryBlocks[index].Address = Addr;
    }
    if (Size > 0) {
        MemoryBlocks[index].Size = Size;
    }

    return MemoryBlocks[index].Address;
}
