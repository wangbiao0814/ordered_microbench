#ifndef _BITMAP_LEAF_NODE_H_
#define _BITMAP_LEAF_NODE_H_
#include "Define.h"
#include "vIndexNode.h"
#include <cstdint>

typedef struct 
{
    uint32_t type; // NodeType
    uint8_t rank[4]; 
    uint64_t bitmap[4]; // Bitmap256 mark for keys
    VALUE_TYPE values[]; // Values
}BitmapLeafNode;

void SetBitmapLeafNodeBitmap(BitmapLeafNode* bitmapLeafNode, uint8_t key);
bool GetBitmapLeafNodeBitmap(BitmapLeafNode* bitmapLeafNode, uint8_t key);
int GetBitmapLeafNodeBitmapPopcnt(BitmapLeafNode* bitmapLeafNode, uint8_t key);
int GetBitmapLeafNodeBitmapPopcntAll(BitmapLeafNode* bitmapLeafNode);

inline void SetBitmapLeafNodeBitmap(BitmapLeafNode* bitmapLeafNode, uint8_t key)
{
    bitmapLeafNode->bitmap[key >> 6] |= (1ul << key);
    bitmapLeafNode->rank[key >> 6]++;
}

inline bool GetBitmapLeafNodeBitmap(BitmapLeafNode* bitmapLeafNode, uint8_t key)
{
    return (bitmapLeafNode->bitmap[key >> 6] & (1ul << key));
}

inline int GetBitmapLeafNodeBitmapPopcnt(BitmapLeafNode* bitmapLeafNode, uint8_t key)
{
    int res = 0;
    for(int i = 0; i < (key >> 6); i++)
    {
        res += bitmapLeafNode->rank[i];
    }
    return res + __builtin_popcountll(bitmapLeafNode->bitmap[key >> 6] & (((uint64_t)2 << key) - 1));
}

inline int GetBitmapLeafNodeBitmapPopcntAll(BitmapLeafNode* bitmapLeafNode)
{
    return bitmapLeafNode->rank[0] + bitmapLeafNode->rank[1] + bitmapLeafNode->rank[2] + bitmapLeafNode->rank[3];
}

void InsertBitmapLeafNodeWithoutDumplicatedKey(BitmapLeafNode* bitmapLeafNode, uint8_t key, VALUE_TYPE value);
void InsertBitmapLeafNode(BitmapLeafNode* node, void** p_node, void* key, VALUE_TYPE value, int level);
VALUE_TYPE GetBitmapLeafNode(BitmapLeafNode* node, void* key, int level);

void RangeBitmapLeafNode(BitmapLeafNode* node, vIndexIterator* iter, int* range, bool isAll, std::vector<VALUE_TYPE>& res);

#endif