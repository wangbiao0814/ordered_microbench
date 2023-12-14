#ifndef _CSR_NODE_H_
#define _CSR_NODE_H_

#include "util.h"
#include "vIndex.h"
#include "Define.h"
#include "vIndexNode.h"
#include <cstdint>

#define CSR_CHILD_INIT_SIZE 4
#define BLOCKS 8

typedef struct 
{
    uint32_t type; // NodeType
    uint8_t rank[4]; 
    uint64_t bitmap[4]; //Bitmap256 mark for CSRNode/BitmapLeafNode
    uint8_t sum[16];
    uint8_t counters[64]; //Counters and cumsum for CSR
    void* CSR[BLOCKS]; //CSR elements
    // uint16_t locks[4];
    void* nodes[]; // Node pointers for CSRNode/BitmapLeafNode
}CSRNode;

inline void SetCSRNodeBitmap(CSRNode* csrNode, uint8_t key)
{
    csrNode->bitmap[key >> 6] |= (1ul << key);
    csrNode->rank[key >> 6]++;
}
inline bool GetCSRNodeBitmap(CSRNode* csrNode, uint8_t key)
{
    return (csrNode->bitmap[key >> 6] & (1ul << key));
}
inline int GetCSRNodeBitmapPopcnt(CSRNode* csrNode, uint8_t key)
{
    int res = 0;
    for(int i = 0; i < (key >> 6); i++)
    {
        res += csrNode->rank[i];
    }
    return res + __builtin_popcountll(csrNode->bitmap[key >> 6] & (((uint64_t)2 << key) - 1));
}
inline int GetCSRNodeBitmapPopcntAll(CSRNode* csrNode)
{
    return csrNode->rank[0] + csrNode->rank[1] + csrNode->rank[2] + csrNode->rank[3];
}


inline int GetCSRNodeCount(CSRNode* csrNode, uint8_t key)
{
    return ((csrNode->counters[key >> 2] >> ((key & 0x3) * 2)) & 0x3);
}
inline int GetCSRNodeCumsum(CSRNode* csrNode, uint8_t key)
{
    int cumsum = 0;
    for(int i = ((key >> 5) << 1); i < (key >> 4); i++) cumsum += csrNode->sum[i];
    for(int i = ((key >> 4) << 2); i < (key >> 2); i++)
    {
        cumsum += byte_count[csrNode->counters[i]];
    }
    cumsum += byte_count[csrNode->counters[key >> 2] & (0xFF >> (8 - ((key & 3) << 1)))];
    return cumsum;
}
inline int GetCSRNodeTotalCount(CSRNode* csrNode, uint8_t blockIndex)
{
    return csrNode->sum[(blockIndex << 1) + 0] + csrNode->sum[(blockIndex << 1) + 1];
}
inline void AddCSRNodeCount(CSRNode* csrNode, uint8_t key)
{
    uint8_t mask1 = 1 << ((key & 0x3) * 2); 
    uint8_t mask2 = (mask1 << 1) + mask1;
    if(mask1 & csrNode->counters[key >> 2])
    {
        csrNode->counters[key >> 2] ^= mask2;
    }
    else{
        csrNode->counters[key >> 2] ^= mask1;
    }
    csrNode->sum[key >> 4]++;
}


void InsertCSRNodeWithoutDumplicatedKey(CSRNode* csrNode, uint8_t* keyBytes, VALUE_TYPE value, int offsetBytes, uint8_t tag);
void InsertCSRNodeOnly(CSRNode* csrNode, int blockIndex, int csrNodeCount, int csrNodeCumsum, uint8_t* keyBytes, int level, VALUE_TYPE value, int tagOffsetBytes, uint8_t tag);
void DeleteCSRNodeKeys(CSRNode* csrNode, int blockIndex, int csrNodeCount, int csrNodeCumsum, uint8_t* keyBytes, int level, int tagOffsetBytes);
void InsertCSRNode(vIndex* root, CSRNode* csrNode, void** parrentCSRNode, void* key, VALUE_TYPE value, int level, uint64_t* levelKeys, uint8_t tag);
VALUE_TYPE GetCSRNode(CSRNode* csrNode, void* key, int level, uint8_t tag);

void RangeCSRNode(vIndex* root, CSRNode* csrNode, vIndexIterator* iter, int* range, bool isAll, std::vector<VALUE_TYPE>& res);

#endif