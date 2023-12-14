#include "BitmapLeafNode.h"

#include <cstdlib>
#include <cstring>

void InsertBitmapLeafNodeWithoutDumplicatedKey(BitmapLeafNode* bitmapLeafNode, uint8_t key, VALUE_TYPE value) // finish
{
    int bitmapIndex = GetBitmapLeafNodeBitmapPopcnt(bitmapLeafNode, key);
    int bitmapTotal = GetBitmapLeafNodeBitmapPopcntAll(bitmapLeafNode);
    for(int i = bitmapTotal; i > bitmapIndex; i--) bitmapLeafNode->values[i] = bitmapLeafNode->values[i - 1];    
    SetBitmapLeafNodeBitmap(bitmapLeafNode, key);
    bitmapLeafNode->values[bitmapIndex] = value;
}


void InsertBitmapLeafNode(BitmapLeafNode* bitmapLeafNode, void** parentBitmapLeafNode, void* key, VALUE_TYPE value, int level) // finish
{
    uint8_t* keyBytes = (uint8_t*)key;
    int bitmapLeafNodePopcntAll = GetBitmapLeafNodeBitmapPopcntAll(bitmapLeafNode);
    int bitmapLeafNodePopcnt = bitmapLeafNodePopcntAll > 128 ? keyBytes[level] : GetBitmapLeafNodeBitmapPopcnt(bitmapLeafNode, keyBytes[level]);

    if(GetBitmapLeafNodeBitmap(bitmapLeafNode, keyBytes[level]))
    {
        bitmapLeafNode->values[bitmapLeafNodePopcnt] = value;
    }
    else
    {   
        if(bitmapLeafNodePopcntAll >= 128)
        {
            if(bitmapLeafNodePopcntAll == 128)
            {
                BitmapLeafNode* newBitmapLeafNode = (BitmapLeafNode*)calloc(1, sizeof(BitmapLeafNode) + 2 * bitmapLeafNodePopcntAll * VALUE_BYTES); int idx = 0;
                memcpy(newBitmapLeafNode, bitmapLeafNode, sizeof(BitmapLeafNode));
                for(int i = 0; i < 256; i++)
                {
                    if(GetBitmapLeafNodeBitmap(bitmapLeafNode, i))
                    {
                        newBitmapLeafNode->values[i] = bitmapLeafNode->values[idx++];
                    }
                }
                free(bitmapLeafNode);
                bitmapLeafNode = newBitmapLeafNode;
                *parentBitmapLeafNode = newBitmapLeafNode;
            }
            bitmapLeafNode->values[keyBytes[level]] = value;
            SetBitmapLeafNodeBitmap(bitmapLeafNode, keyBytes[level]);
        }
        else
        {
            if((bitmapLeafNodePopcntAll & (bitmapLeafNodePopcntAll - 1)) == 0)
            {
                BitmapLeafNode* newBitmapLeafNode = (BitmapLeafNode*)calloc(1, sizeof(BitmapLeafNode) + 2 * bitmapLeafNodePopcntAll * VALUE_BYTES);
                memcpy(newBitmapLeafNode, bitmapLeafNode, sizeof(BitmapLeafNode) + bitmapLeafNodePopcntAll * VALUE_BYTES);
                free(bitmapLeafNode);
                bitmapLeafNode = newBitmapLeafNode;
                *parentBitmapLeafNode = newBitmapLeafNode;
            }
            for(int i = bitmapLeafNodePopcntAll; i > bitmapLeafNodePopcnt; i--) bitmapLeafNode->values[i] = bitmapLeafNode->values[i - 1];
            bitmapLeafNode->values[bitmapLeafNodePopcnt] = value;
            SetBitmapLeafNodeBitmap(bitmapLeafNode, keyBytes[level]);
        }

    }
}

VALUE_TYPE GetBitmapLeafNode(BitmapLeafNode* bitmapLeafNode, void* key, int level) // finish
{
    uint8_t* keyBytes = (uint8_t*)key;
    if(GetBitmapLeafNodeBitmap(bitmapLeafNode, keyBytes[level]))
    {
        int bitmapLeafNodePopcntAll = GetBitmapLeafNodeBitmapPopcntAll(bitmapLeafNode);
        int bitmapLeafNodePopcnt = bitmapLeafNodePopcntAll > 128 ? keyBytes[level] : GetBitmapLeafNodeBitmapPopcnt(bitmapLeafNode, keyBytes[level]) - 1;
        return bitmapLeafNode->values[bitmapLeafNodePopcnt];
    }
    return 0;
}

void RangeBitmapLeafNode(BitmapLeafNode* bitmapLeafNode, vIndexIterator* iter, int* range, bool isAll, std::vector<VALUE_TYPE>& res)
{
    int key = isAll ? 0 : iter->bytes[iter->level];
    int bitmapLeafNodePopcntAll = GetBitmapLeafNodeBitmapPopcntAll(bitmapLeafNode);
    int tempBitmapLeafNodePopcnt = GetBitmapLeafNodeBitmapPopcnt(bitmapLeafNode, key);
    if(GetBitmapLeafNodeBitmap(bitmapLeafNode, key)) tempBitmapLeafNodePopcnt -= 1;
    for(; key < 256; key++)
    {
        if(GetBitmapLeafNodeBitmap(bitmapLeafNode, key))
        {
            int bitmapLeafNodePopcnt = bitmapLeafNodePopcntAll > 128 ? key : tempBitmapLeafNodePopcnt; 
            *range = *range - 1; tempBitmapLeafNodePopcnt++;
            res.push_back(bitmapLeafNode->values[bitmapLeafNodePopcnt]);
            // printf("%ld\n", bitmapLeafNode->values[bitmapLeafNodePopcnt]);
            if(*range <= 0) return;
        }
    }
    return;
}