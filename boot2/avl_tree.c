#include "avl_tree.h"
#include "kmaths.h"

EmbeddedAvlTree* AvlTree_AvlTree(EmbeddedAvlTree* const tree)
{
    tree->Left = nullptr;
    tree->Right = nullptr;
    tree->Height = 0;
    return tree;
}

static uSys AvlTree_Height(const EmbeddedAvlTree* const tree)
{
    if(!tree)
    {
        return 0;
    }

    return tree->Height;
}

static i32 AvlTree_ComputeBalance(const EmbeddedAvlTree* const tree)
{
    if(!tree)
    { 
        return 0;    
    }

    return (i32) AvlTree_Height(tree->Left) - (i32) AvlTree_Height(tree->Right);
}

static EmbeddedAvlTree* AvlTree_RotateRight(EmbeddedAvlTree* const pivot)
{
    EmbeddedAvlTree* newRoot = pivot->Left;
    EmbeddedAvlTree* transferNode = newRoot->Right;

    newRoot->Right = pivot;
    pivot->Left = transferNode;

    pivot->Height = KM_MAX(AvlTree_Height(pivot->Left), AvlTree_Height(pivot->Right)) + 1;
    newRoot->Height = KM_MAX(AvlTree_Height(newRoot->Left), AvlTree_Height(newRoot->Right)) + 1;

    return newRoot;
}

static EmbeddedAvlTree* AvlTree_RotateLeft(EmbeddedAvlTree* const pivot)
{
    EmbeddedAvlTree* newRoot = pivot->Right;
    EmbeddedAvlTree* transferNode = newRoot->Left;

    newRoot->Left = pivot;
    pivot->Right = transferNode;

    pivot->Height = KM_MAX(AvlTree_Height(pivot->Left), AvlTree_Height(pivot->Right)) + 1;
    newRoot->Height = KM_MAX(AvlTree_Height(newRoot->Left), AvlTree_Height(newRoot->Right)) + 1;

    return newRoot;
}

EmbeddedAvlTree* AvlTree_Insert(EmbeddedAvlTree* const tree, EmbeddedAvlTree* const node, AvlTree_Comparator_f comparator)
{
    if(!tree)
    {
        return node;
    }

    const i32 comp = comparator(tree, node);

    if(comp < 0)
    { tree->Left = AvlTree_Insert(tree->Left, node, comparator); }
    else if(comp > 0)
    { tree->Right = AvlTree_Insert(tree->Right, node, comparator); }
    else
    {
        // If equal insert as if greater.
        tree->Right = AvlTree_Insert(tree->Right, node, comparator);
    }

    tree->Height = KM_MAX(AvlTree_Height(tree->Left), AvlTree_Height(tree->Right)) + 1;
    const i32 balance = AvlTree_ComputeBalance(tree);

    const i32 compLeft = comparator(tree->Left, node);
    const i32 compRight = comparator(tree->Right, node);

    // Left Left
    if(balance > 1 && compLeft < 0)
    { return AvlTree_RotateRight(tree); }

    // Right Right
    if(balance < -1 && compRight > 0)
    { return AvlTree_RotateLeft(tree); }

    // Left Right
    if(balance > 1 && compLeft > 0)
    {
        tree->Left = AvlTree_RotateLeft(tree->Left);
        return AvlTree_RotateRight(tree);
    }

    // Right Left
    if(balance < -1 && compRight < 0)
    {
        tree->Right = AvlTree_RotateRight(tree->Right);
        return AvlTree_RotateLeft(tree);
    }

    return tree;
}

EmbeddedAvlTree* AvlTree_Remove(EmbeddedAvlTree* const root, EmbeddedAvlTree* const node, AvlTree_Comparator_f comparator);

EmbeddedAvlTree* AvlTree_Find(EmbeddedAvlTree* const tree, EmbeddedAvlTree* const search, AvlTree_Comparator_f comparator)
{
    EmbeddedAvlTree* node = tree;

    while(node)
    {
        const i32 comp = comparator(tree, search);

        if(comp == 0)
        { return node; }

        if(comp > 0)
        { node = node->Right; }
        else
        { node = node->Left; }
    }

    return nullptr;
}

