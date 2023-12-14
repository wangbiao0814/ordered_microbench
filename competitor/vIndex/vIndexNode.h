#ifndef _VINDEX_NODE_H_
#define _VINDEX_NODE_H_
#include "vIndex.h"



typedef enum{
    CSR_NODE = 0,
    DELTA_LEAF_NODE,
    BITMAP_LEAF_NODE,
}NODE_TYPE;

typedef struct 
{
    uint32_t type;
    uint32_t padding;
}vIndexNode;

typedef struct 
{
    vIndexNode* nodes[4];
    uint64_t levelKeys[4];
    uint8_t bytes[8];
    int level;
    uint32_t offset;
}vIndexIterator;


void InsertvIndexNode(vIndex* root, vIndexNode* node, void** parrentNode, void* key, VALUE_TYPE value, int level, uint64_t* levelKeys, uint8_t tag);
VALUE_TYPE GetvIndexNode(vIndexNode* node, void* key, int level, uint8_t tag);
void RangevIndexNode(vIndex* root, vIndexIterator* iter, int* range, std::vector<VALUE_TYPE>& res);


#endif