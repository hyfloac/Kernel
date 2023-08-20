#include "pool_allocator.h"
#include "kalloc.h"
#include "memcpy.h"
#include "kmaths.h"
#include "memset.h"
#include "kprintf.h"

#define POOL_TAG_TAGS ('SGAT')

typedef struct PoolAllocationHeader
{
    PoolFlag Flags;
    uSys BlockSize;
    uSys AllocSize;
    uSys _Align;
} PoolAllocationHeader;

typedef struct PoolMemoryBlock
{
    u64 BasePage;
    u64 PageCount;

    struct PoolMemoryBlock* Next;
} PoolMemoryBlock;

// An AVL Tree.
typedef struct TaggedAllocationsNode
{
    TaggedAllocationsInfo Info;

    i32 Height;
    struct TaggedAllocationsNode* Left;
    struct TaggedAllocationsNode* Right;
} TaggedAllocationsNode;

TaggedAllocationsNode* TaggedAllocationsTree = NULL;
i32 TaggedAllocationsTreeCount = 0;

TaggedAllocationsNode* TaggedAllocationsNodeFreeList = NULL;

static TaggedAllocationsNode* InsertTaggedAllocation(const u32 tag);
static TaggedAllocationsNode* FindTaggedAllocationInfo(const u32 tag);

static b8 AddMoreTagNodes(const u32 nodeCount)
{
    if(TaggedAllocationsNodeFreeList)
    {
        return 0;
    }

    TaggedAllocationsNodeFreeList = kalloc(sizeof(TaggedAllocationsNode) * nodeCount);
    
    for(u32 i = 0; i < nodeCount - 1; ++i)
    {
        TaggedAllocationsNodeFreeList[i].Left = &TaggedAllocationsNodeFreeList[i + 1];
    }

    TaggedAllocationsNodeFreeList[nodeCount - 1].Left = NULL;

    return 1;
}

static void UpdateTagNodesNodeAfterAlloc(TaggedAllocationsNode* const tagNode, const u32 additionalNodes)
{
    ++tagNode->Info.CurrentAllocationCount;
    ++tagNode->Info.TotalAllocations;
    tagNode->Info.TotalAllocationSize += sizeof(TaggedAllocationsNode*) * additionalNodes;
}

void InitPoolAllocator(void)
{
    const u32 nodeCount = 256;

    if(AddMoreTagNodes(nodeCount))
    {
        TaggedAllocationsNode* const initialNode = InsertTaggedAllocation(POOL_TAG_TAGS);

        if(!initialNode)
        {
            kprintf("An initial node for the Pool allocator could not be created.\n");
            return;
        }

        UpdateTagNodesNodeAfterAlloc(initialNode, nodeCount);
    }
}

void* AllocatePoolWithTag(PoolFlag Flags, uSys SizeInBytes, u32 Tag)
{
    (void) Flags;

    const uSys sizeWithHeader = sizeof(PoolAllocationHeader) * SizeInBytes;

    void* const allocation = kalloc(sizeWithHeader);
    
    TaggedAllocationsNode* tagInfo = FindTaggedAllocationInfo(Tag);

    if(!tagInfo)
    {
        tagInfo = InsertTaggedAllocation(Tag);

        if(!tagInfo)
        {
            kfree(allocation);
            return NULL;
        }
    }

    ++tagInfo->Info.CurrentAllocationCount;
    ++tagInfo->Info.TotalAllocations;
    tagInfo->Info.TotalAllocationSize += SizeInBytes;

    if((Flags & PoolFlagZeroMemory) == PoolFlagZeroMemory)
    {
        zeromem_stosb(allocation, sizeWithHeader);
    }

    PoolAllocationHeader* const header = (PoolAllocationHeader*) allocation;
    header->Flags = Flags;
    header->BlockSize = sizeWithHeader;
    header->AllocSize = sizeWithHeader;

    return (void*) (header + 1);
}

void FreePoolWithTag(void* const Allocation, const u32 Tag)
{
    PoolAllocationHeader* const header = ((PoolAllocationHeader*) Allocation) - 1;

    kfree(header);

    TaggedAllocationsNode* const tagInfo = FindTaggedAllocationInfo(Tag);

    if(!tagInfo)
    {
        return;
    }
    
    --tagInfo->Info.CurrentAllocationCount;
    ++tagInfo->Info.TotalFrees;
}

static uSys IterateInsertTaggedAllocationInfos(const uSys maxCount, uSys i, const TaggedAllocationsNode* const tree, TaggedAllocationsInfo* const storage)
{
    if(!tree)
    {
        return 0;
    }

    i += IterateInsertTaggedAllocationInfos(maxCount, i, tree->Left, storage);
    if(i >= maxCount)
    {
        return i;
    }

    memcpy(&storage[i], &tree->Info, sizeof(*storage));
    ++i;

    if(i >= maxCount)
    {
        return i;
    }

    i += IterateInsertTaggedAllocationInfos(maxCount, i, tree->Right, storage);

    return i;
}

KError_t EnumTaggedAllocationInfos(uSys* const TagCount, TaggedAllocationsInfo* const TaggedAllocationInfos)
{
    if(!TagCount)
    {
        return KE_INVALID_ARG;
    }

    if(!TaggedAllocationInfos)
    {
        *TagCount = TaggedAllocationsTreeCount;
        return KE_OK;
    }

    const uSys totalTagsToInsert = KM_MIN(*TagCount, (uSys) TaggedAllocationsTreeCount);

    (void) IterateInsertTaggedAllocationInfos(totalTagsToInsert, 0, TaggedAllocationsTree, TaggedAllocationInfos);
    
    *TagCount = TaggedAllocationsTreeCount;

    // memcpy(&TaggedAllocationInfos[0], &TaggedAllocationsTree->Info, sizeof(*TaggedAllocationInfos));
    // *TagCount = 1;

    return KE_OK;
}

static TaggedAllocationsNode* PoolAllocatorAllocTagNode()
{
    if(!TaggedAllocationsNodeFreeList)
    {
        const u32 nodeCount = 256;

        if(AddMoreTagNodes(nodeCount))
        {
            TaggedAllocationsNode* const tagNode = FindTaggedAllocationInfo(POOL_TAG_TAGS);

            if(tagNode)
            {
                UpdateTagNodesNodeAfterAlloc(tagNode, nodeCount);
            }
        }
    }

    if(!TaggedAllocationsNodeFreeList)
    {
        return NULL;
    }

    TaggedAllocationsNode* const ret = TaggedAllocationsNodeFreeList;
    TaggedAllocationsNodeFreeList = TaggedAllocationsNodeFreeList->Left;
    ++TaggedAllocationsTreeCount;

    return ret;
}

// static void PoolAllocatorFreeTagNode(TaggedAllocationsNode* const node)
// {
//     if(!node)
//     {
//         return;
//     }

//     node->Left = TaggedAllocationsNodeFreeList;
//     TaggedAllocationsNodeFreeList = node;   
//     --TaggedAllocationsTreeCount;
// }

static i32 TaggedAllocationsTreeHeight(const TaggedAllocationsNode* tree)
{
    if(!tree)
    {
        return 0;
    }
    return tree->Height;
}

static i32 TaggedAllocationsTreeComputeBalance(const TaggedAllocationsNode* const tree)
{
    if(!tree)
    { 
        return 0; 
    }

    return (i32) TaggedAllocationsTreeHeight(tree->Left) - (i32) TaggedAllocationsTreeHeight(tree->Right);
}

static TaggedAllocationsNode* TaggedAllocationsTreeRotateRight(TaggedAllocationsNode* const pivot)
{
    TaggedAllocationsNode* const newRoot = pivot->Left;
    TaggedAllocationsNode* const transferNode = newRoot->Right;

    newRoot->Right = pivot;
    pivot->Left = transferNode;

    pivot->Height = KM_MAX(TaggedAllocationsTreeHeight(pivot->Left), TaggedAllocationsTreeHeight(pivot->Right)) + 1;
    newRoot->Height = KM_MAX(TaggedAllocationsTreeHeight(newRoot->Left), TaggedAllocationsTreeHeight(newRoot->Right)) + 1;

    return newRoot;
}

static TaggedAllocationsNode* TaggedAllocationsTreeRotateLeft(TaggedAllocationsNode* const pivot)
{
    TaggedAllocationsNode* const newRoot = pivot->Right;
    TaggedAllocationsNode* const transferNode = newRoot->Left;

    newRoot->Left = pivot;
    pivot->Right = transferNode;

    pivot->Height = KM_MAX(TaggedAllocationsTreeHeight(pivot->Left), TaggedAllocationsTreeHeight(pivot->Right)) + 1;
    newRoot->Height = KM_MAX(TaggedAllocationsTreeHeight(newRoot->Left), TaggedAllocationsTreeHeight(newRoot->Right)) + 1;

    return newRoot;
}

static TaggedAllocationsNode* InsertTaggedAllocationTree(TaggedAllocationsNode* tree, TaggedAllocationsNode* newNode)
{
    if(!tree)
    {
        return newNode;
    }

    if(newNode->Info.Tag <= tree->Info.Tag)
    {
        tree->Left = InsertTaggedAllocationTree(tree->Left, newNode);
    }
    else if(newNode->Info.Tag > tree->Info.Tag)
    {
        tree->Right = InsertTaggedAllocationTree(tree->Right, newNode);
    }

    tree->Height = KM_MAX(TaggedAllocationsTreeHeight(tree->Left), TaggedAllocationsTreeHeight(tree->Right)) + 1;
    const i32 balance = TaggedAllocationsTreeComputeBalance(tree);

    // Left
    if(balance > 1 && newNode->Info.Tag < tree->Left->Info.Tag)
    { 
        return TaggedAllocationsTreeRotateRight(tree); 
    }

    // Right
    if(balance < -1 && newNode->Info.Tag > tree->Right->Info.Tag)
    { 
        return TaggedAllocationsTreeRotateLeft(tree);
    }

    // Left Right
    if(balance > 1 && newNode->Info.Tag > tree->Left->Info.Tag)
    {
        tree->Left = TaggedAllocationsTreeRotateLeft(tree->Left);
        return TaggedAllocationsTreeRotateRight(tree);
    }

    // Right Left
    if(balance < -1 && newNode->Info.Tag < tree->Right->Info.Tag)
    {
        tree->Right = TaggedAllocationsTreeRotateRight(tree->Right);
        return TaggedAllocationsTreeRotateLeft(tree);
    }

    return tree;
}

static TaggedAllocationsNode* InsertTaggedAllocation(const u32 tag)
{
    TaggedAllocationsNode* const node = PoolAllocatorAllocTagNode();

    if(!node)
    {
        return NULL;
    }

    node->Info.CurrentAllocationCount = 0;
    node->Info.TotalAllocations = 0;
    node->Info.TotalFrees = 0;
    node->Info.TotalAllocationSizeLow = 0;
    node->Info.TotalAllocationSizeHigh = 0;
    node->Info.Tag = tag;
    node->Height = 1;
    node->Left = NULL;
    node->Right = NULL;

    TaggedAllocationsTree = InsertTaggedAllocationTree(TaggedAllocationsTree, node);

    return node;
}

static TaggedAllocationsNode* FindTaggedAllocationInfo(const u32 tag)
{
    TaggedAllocationsNode* node = TaggedAllocationsTree;

    while(node)
    {
        if(tag == node->Info.Tag)
        { return node; }

        if(tag > node->Info.Tag)
        { node = node->Right; }
        else
        { node = node->Left; }
    }

    return NULL;
}
