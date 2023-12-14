#ifndef _DELTA_LEAF_NODE_H_
#define _DELTA_LEAF_NODE_H_
#include <cstdint>
#include "vIndex.h"
#include "vIndexNode.h"
#include "Define.h"

typedef struct 
{
    uint32_t type; // NodeType
    uint32_t count;
    uint8_t tags[16];
    uint8_t kvs[];
}DeltaLeafNode;

void InsertDeltaLeafNodeWithoutDumplicatedKey(DeltaLeafNode* deltaLeafNode, uint8_t* keyBytes, VALUE_TYPE value, int offsetBytes, uint8_t tag);
void InsertInitDeltaLeafNode(DeltaLeafNode* deltaLeafNode, void* key, VALUE_TYPE value, int level, uint8_t tag);
void InsertDeltaLeafNode(vIndex* root, DeltaLeafNode* deltaLeafNode, void** parentDeltaLeafNode, void* key, VALUE_TYPE value, int level, uint8_t tag);
VALUE_TYPE GetDeltaLeafNode(DeltaLeafNode* node, void* key, int level, uint8_t tag);

void RangeDeltaLeafNode(DeltaLeafNode* node, vIndexIterator* iter, int* range, bool isAll, std::vector<VALUE_TYPE>& res);

#endif