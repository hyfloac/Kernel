#include "pool_allocator.h"

typedef struct PoolAllocationHeader
{
    u32 Tag;
    PoolFlag Flags;
    uSys BlockSize;
    uSys AllocationSize;
} PoolAllocationHeader;

typedef struct PoolMemoryBlock
{
    u64 BasePage;
    u64 PageCount;

    PoolBlocks* Next;
} PoolMemoryBlock;

// An AVL Tree.
typedef struct PoolTree
{
    PoolInfo Info;
    PoolMemoryBlock* MemoryBlocks;

    i32 Height;
    PoolTree* Left;
    PoolTree* Right;
} PoolTree;

static KError_t InsertNode(PoolTree** TreeRoot, PoolTree* NewNode);

void* AllocatePoolWithTag(PoolFlag Flags, uSys SizeInBytes, u32 Tag);

KError_t EnumPools(uSys* PoolCount, PoolInfo* PoolInfos);

static KError_t InsertNode(PoolTree** constTreeRoot, PoolTree* const NewNode)
{
    if(!TreeRoot || !(*TreeRoot) || *NewNode)
    {
        return KE_INVALID_ARG;
    }

    if(NewNode->Info.PoolTag < (*TreeRoot)->Info.PoolTag)
    {
        if((*TreeRoot)->Left)
        {
            return InsertNode((*TreeRoot)->Left, NewNode);
        }
    }

    return KE_OK;
}


