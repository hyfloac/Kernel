#pragma once

#include "kstdint.h"
#include "offsetof.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EmbeddedAvlTree
{
    struct EmbeddedAvlTree* Left;
    struct EmbeddedAvlTree* Right;
    uSys Height;
} EmbeddedAvlTree;

typedef i32(*AvlTree_Comparator_f)(EmbeddedAvlTree* const branch, EmbeddedAvlTree* const node);

EmbeddedAvlTree* AvlTree_AvlTree(EmbeddedAvlTree* const tree);

EmbeddedAvlTree* AvlTree_Insert(EmbeddedAvlTree* const tree, EmbeddedAvlTree* const node, AvlTree_Comparator_f comparator);
EmbeddedAvlTree* AvlTree_Remove(EmbeddedAvlTree* const tree, EmbeddedAvlTree* const node, AvlTree_Comparator_f comparator);
EmbeddedAvlTree* AvlTree_Find(EmbeddedAvlTree* const tree, EmbeddedAvlTree* const search, AvlTree_Comparator_f comparator);

#define EMBEDDED_OBJECT(EmbeddedObject, ParentType, EmbeddedName) ((ParentType*) (((u8*) (EmbeddedObject)) - offsetof(ParentType, EmbeddedName)))

#ifdef __cplusplus
} /* extern "C" */
#endif
