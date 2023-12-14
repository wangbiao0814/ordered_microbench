#include "DeltaLeafNode.h"
#include "CSRNode.h"
#include "BitmapLeafNode.h"
#include <cstdlib>
#include <cstring>

extern "C"
{
#include <emmintrin.h>
}

int NextPower(uint8_t input)
{
    int nextPower = 4;
    while(nextPower < input)
    {
        nextPower <<= 1;
    }
    return nextPower;
}

void InsertDeltaLeafNodeWithoutDumplicatedKey(DeltaLeafNode* deltaLeafNode, uint8_t* keyBytes, VALUE_TYPE value, int offsetBytes, uint8_t tag)
{
    deltaLeafNode->tags[deltaLeafNode->count] = tag;
    memcpy(deltaLeafNode->kvs + (offsetBytes + VALUE_BYTES) * deltaLeafNode->count, keyBytes, offsetBytes);
    *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + (offsetBytes + VALUE_BYTES) * deltaLeafNode->count + offsetBytes) = value;
    deltaLeafNode->count++;
}

void InsertTree(vIndex* root, DeltaLeafNode* deltaLeafNode, void** parrentNode, void* nodes[], uint8_t deltaLeafNodesPrefix[], uint32_t deltaLeafNodesCount, void* deltaLeafNodes[], uint8_t* keyBytes, int level, int lcpLength)
{
    uint64_t key = __builtin_bswap64(*(uint64_t*)keyBytes);
    free(deltaLeafNode);
    for(int i = level; i <= level + lcpLength; i++)
    {
        if(isUpperLevel(i))
        {
#ifdef USE_FLATHASH
            (*root->levelHash[i >> 1])[key >> (64 - (i << 3))] = nodes[i];
#else
            InsertHashTable(root->levelHash[i >> 1], key >> (64 - (i << 3)), nodes[i]);
#endif
            if(i != level + lcpLength)
            {
                CSRNode* tempCSRNode = (CSRNode*)nodes[i]; tempCSRNode->nodes[0] = nodes[i + 1];
            }
                
        }
        else if(i == level)
        {   
            *parrentNode = nodes[level];
        }
    }

    int j = level + lcpLength + 1;
    if(isUpperLevel(j))
    {
        for(int i = 0; i < deltaLeafNodesCount; i++){
#ifdef USE_FLATHASH
            (*root->levelHash[j >> 1])[((key >> (64 - ((j - 1) << 3))) << 8) + deltaLeafNodesPrefix[i]] = deltaLeafNodes[i];
#else
            InsertHashTable(root->levelHash[j >> 1], ((key >> (64 - ((j - 1) << 3))) << 8) + deltaLeafNodesPrefix[i], deltaLeafNodes[i]);
#endif 
            SetCSRNodeBitmap((CSRNode*)nodes[j - 1], deltaLeafNodesPrefix[i]);
        }
    }
    else
    {   
        CSRNode* tempCSRNode = (CSRNode*)nodes[j - 1];
        for(int k = 0; k < deltaLeafNodesCount; k++)
        {
            for(int l = k + 1; l < deltaLeafNodesCount; l++)
            {
                if(deltaLeafNodesPrefix[l] < deltaLeafNodesPrefix[k])
                {
                    uint8_t tempPrefix = deltaLeafNodesPrefix[l]; void* tempDeltaLeafNode = deltaLeafNodes[l];
                    deltaLeafNodesPrefix[l] = deltaLeafNodesPrefix[k];
                    deltaLeafNodes[l] = deltaLeafNodes[k];
                    deltaLeafNodesPrefix[k] = tempPrefix;
                    deltaLeafNodes[k] = tempDeltaLeafNode;
                }
            }
            SetCSRNodeBitmap(tempCSRNode, deltaLeafNodesPrefix[k]);
            tempCSRNode->nodes[k] = deltaLeafNodes[k];
        }
        
    }
    
}

int ConstructTree(vIndex* root, DeltaLeafNode* deltaLeafNode, uint8_t* keyBytes, VALUE_TYPE value, int offsetBytes, void* nodes[], uint8_t deltaLeafNodesPrefix[], uint32_t* deltaLeafNodesCount, void* deltaLeafNodes[], int level, uint8_t tag)
{
    uint8_t lcp[KEY_BYTES] = {0}; int lcpLength = offsetBytes;
    memcpy(lcp, keyBytes + level, offsetBytes);
    for(int i = 0; i < deltaLeafNode->count; i++)
    {
        uint8_t* nodeKey = ((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES)); int j;
        for(j = 0; j < lcpLength; j++)
        {
            if(nodeKey[j] != lcp[j]) break;
        }
        lcpLength = j;
    }
    //Find the longest common prefix, build upper and lower CSR node using the longest common prefix and a byte after the longest common prefix
    CSRNode* tempCSRNode = NULL; BitmapLeafNode* tempBitmapLeafNode = NULL; DeltaLeafNode* tempDeltaLeafNode = NULL; int levelIndex;
    for(levelIndex = level; levelIndex < level + lcpLength; levelIndex++)
    {
        if(isUpperLevel(levelIndex))
        {
            tempCSRNode = (CSRNode*)calloc(1, sizeof(CSRNode) + CSR_CHILD_INIT_SIZE * sizeof(void*));
        }
        else
        {
            tempCSRNode = (CSRNode*)calloc(1, sizeof(CSRNode));
        }
        tempCSRNode->type = CSR_NODE;
        SetCSRNodeBitmap(tempCSRNode, keyBytes[levelIndex]);
        nodes[levelIndex] = (void*)tempCSRNode;
    }
    if(levelIndex == KEY_BYTES - 1)
    {
        tempBitmapLeafNode = (BitmapLeafNode*)calloc(1, sizeof(BitmapLeafNode) + 32 * sizeof(VALUE_TYPE));
        tempBitmapLeafNode->type = BITMAP_LEAF_NODE;
        for(int i = 0; i < deltaLeafNode->count; i++)                                                                       
        {
            InsertBitmapLeafNodeWithoutDumplicatedKey(tempBitmapLeafNode, *((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + offsetBytes - 1), \
                                                *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + offsetBytes));
        }
        InsertBitmapLeafNodeWithoutDumplicatedKey(tempBitmapLeafNode, *(keyBytes + KEY_BYTES - 1), value);
        nodes[levelIndex] = (void*)tempBitmapLeafNode;
    }
    else if(levelIndex == KEY_BYTES - 2)
    {
        tempCSRNode = (CSRNode*)calloc(1, sizeof(CSRNode) + CSR_CHILD_INIT_SIZE * sizeof(void*));
        uint8_t counter[256] = {0};
        counter[*(keyBytes + level + lcpLength)]++;
        for(int i = 0; i < deltaLeafNode->count; i++) counter[*(uint8_t*)((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + lcpLength)]++;
        for(int i = 0; i < deltaLeafNode->count; i++)
        {
            uint8_t prefix = *(uint8_t*)((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + lcpLength);
            if(counter[prefix] <= 3)
            {
                InsertCSRNodeWithoutDumplicatedKey(tempCSRNode, (uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + lcpLength, \
                    *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + offsetBytes), offsetBytes - lcpLength - 1, deltaLeafNode->tags[i]);
            }
            else
            {
                int tempCSRNodeBitmapPopcnt = GetCSRNodeBitmapPopcnt(tempCSRNode, prefix);
                if(GetCSRNodeBitmap(tempCSRNode, prefix))
                {
                    tempBitmapLeafNode = (BitmapLeafNode*)tempCSRNode->nodes[tempCSRNodeBitmapPopcnt - 1];
                }
                else
                {
                    int tempCSRNodeBitmapPopcntAll = GetCSRNodeBitmapPopcntAll(tempCSRNode);
                    for(int j = tempCSRNodeBitmapPopcntAll; j > tempCSRNodeBitmapPopcnt; j--) tempCSRNode->nodes[j] = tempCSRNode->nodes[j - 1];
                    tempBitmapLeafNode = (BitmapLeafNode*)calloc(1, sizeof(BitmapLeafNode) + VALUE_BYTES * NextPower(counter[prefix]));
                    tempBitmapLeafNode->type = BITMAP_LEAF_NODE;
                    tempCSRNode->nodes[tempCSRNodeBitmapPopcnt] = tempBitmapLeafNode;
                    SetCSRNodeBitmap(tempCSRNode, prefix);
                }
                InsertBitmapLeafNodeWithoutDumplicatedKey(tempBitmapLeafNode, *((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + offsetBytes - 1), \
                                                *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + offsetBytes));
               //insert BitmapLeafNode
            }
        }
        if(counter[keyBytes[level + lcpLength]] <= 3)
        {
            InsertCSRNodeWithoutDumplicatedKey(tempCSRNode, keyBytes + level + lcpLength, value, offsetBytes - lcpLength - 1, tag);
        }
        else
        {
            uint8_t prefix = keyBytes[level + lcpLength];
            int tempCSRNodeBitmapPopcnt = GetCSRNodeBitmapPopcnt(tempCSRNode, prefix);
            if(GetCSRNodeBitmap(tempCSRNode, prefix))
            {
                tempBitmapLeafNode = (BitmapLeafNode*)tempCSRNode->nodes[tempCSRNodeBitmapPopcnt - 1];
            }
            else
            {
                int tempCSRNodeBitmapPopcntAll = GetCSRNodeBitmapPopcntAll(tempCSRNode);
                for(int j = tempCSRNodeBitmapPopcntAll; j > tempCSRNodeBitmapPopcnt; j--) tempCSRNode->nodes[j] = tempCSRNode->nodes[j - 1];
                tempBitmapLeafNode = (BitmapLeafNode*)calloc(1, sizeof(BitmapLeafNode) + VALUE_BYTES * NextPower(counter[prefix]));
                tempBitmapLeafNode->type = BITMAP_LEAF_NODE;
                tempCSRNode->nodes[tempCSRNodeBitmapPopcnt] = tempBitmapLeafNode;
                SetCSRNodeBitmap(tempCSRNode, prefix);
            }
            InsertBitmapLeafNodeWithoutDumplicatedKey(tempBitmapLeafNode, keyBytes[level + lcpLength + 1], value);
        }
        nodes[levelIndex] = (void*)tempCSRNode;
    }
    else
    {
        if(isUpperLevel(levelIndex))
            tempCSRNode = (CSRNode*)calloc(1, sizeof(CSRNode) + CSR_CHILD_INIT_SIZE * sizeof(void*));
        else
            tempCSRNode = (CSRNode*)calloc(1, sizeof(CSRNode));  
        uint8_t counter[256] = {0};
        counter[*(keyBytes + level + lcpLength)]++;
        for(int i = 0; i < deltaLeafNode->count; i++) counter[*(uint8_t*)((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + lcpLength)]++;

        
        for(int i = 0; i < deltaLeafNode->count; i++)
        {
            uint8_t prefix = *(uint8_t*)((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + lcpLength);
            if(counter[prefix] <= 3)
            {
                InsertCSRNodeWithoutDumplicatedKey(tempCSRNode, (uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + lcpLength, \
                    *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + offsetBytes), offsetBytes - lcpLength - 1, deltaLeafNode->tags[i]);
            }
            else
            {
                for(int i = 0; i < *deltaLeafNodesCount; i++)
                {
                    if(deltaLeafNodesPrefix[i] == prefix)
                    {   
                        tempDeltaLeafNode = (DeltaLeafNode*)deltaLeafNodes[i];
                    }
                    
                }
                if(tempDeltaLeafNode == NULL)
                {
                    tempDeltaLeafNode = (DeltaLeafNode*)calloc(1, sizeof(DeltaLeafNode) + (offsetBytes - lcpLength - 1 + VALUE_BYTES) * NextPower(counter[prefix]));
                    tempDeltaLeafNode->type = DELTA_LEAF_NODE;
                    deltaLeafNodesPrefix[*deltaLeafNodesCount] = prefix;
                    deltaLeafNodes[*deltaLeafNodesCount] = (void*)tempDeltaLeafNode;
                    (*deltaLeafNodesCount)++;
                }
                InsertDeltaLeafNodeWithoutDumplicatedKey(tempDeltaLeafNode, ((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + lcpLength + 1), \
                                                            *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + offsetBytes), \
                                                            offsetBytes - lcpLength - 1, deltaLeafNode->tags[i]);
                tempDeltaLeafNode = NULL;
            }
        }
        uint8_t prefix = keyBytes[level + lcpLength];
        if(counter[prefix] <= 3)
        {
            InsertCSRNodeWithoutDumplicatedKey(tempCSRNode, keyBytes + level + lcpLength, value, offsetBytes - lcpLength - 1, tag);
        }
        else
        {
            for(int i = 0; i < *deltaLeafNodesCount; i++)
            {
                if(deltaLeafNodesPrefix[i] == prefix)
                {   
                    tempDeltaLeafNode = (DeltaLeafNode*)deltaLeafNodes[i];
                }
                
            }
            if(tempDeltaLeafNode == NULL)
            {
                tempDeltaLeafNode = (DeltaLeafNode*)calloc(1, sizeof(DeltaLeafNode) + (offsetBytes - lcpLength - 1 + VALUE_BYTES) * NextPower(counter[prefix]));
                tempDeltaLeafNode->type = DELTA_LEAF_NODE;
                deltaLeafNodesPrefix[*deltaLeafNodesCount] = prefix;
                deltaLeafNodes[*deltaLeafNodesCount] = (void*)tempDeltaLeafNode;
                (*deltaLeafNodesCount)++;
            }
            InsertDeltaLeafNodeWithoutDumplicatedKey(tempDeltaLeafNode, keyBytes + KEY_BYTES - offsetBytes + lcpLength + 1, \
                                                        value, offsetBytes - lcpLength - 1, tag);
            tempDeltaLeafNode = NULL;
        }
        nodes[levelIndex] = (void*)tempCSRNode;
    }
    return lcpLength;
}

void InsertInitDeltaLeafNode(DeltaLeafNode* deltaLeafNode, void* key, VALUE_TYPE value, int level, uint8_t tag) //finish
{
    uint8_t* keyBytes = (uint8_t*)key;
    uint8_t offsetBytes = KEY_BYTES - level; int idx;
    deltaLeafNode->tags[deltaLeafNode->count] = tag;
    memcpy((uint8_t*)deltaLeafNode->kvs + deltaLeafNode->count * (offsetBytes + VALUE_BYTES), key + level, offsetBytes);
    *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + deltaLeafNode->count * (offsetBytes + VALUE_BYTES) + offsetBytes) = value;
    deltaLeafNode->count++;
}

void InsertDeltaLeafNode(vIndex* root, DeltaLeafNode* deltaLeafNode, void** parentDeltaLeafNode, void* key, VALUE_TYPE value, int level, uint8_t tag)
{
    uint8_t* keyBytes = (uint8_t*)key;
    uint8_t offsetBytes = KEY_BYTES - level; 

    __m128i cmp; uint16_t bitfield; uint8_t idx;
    cmp = _mm_cmpeq_epi8(_mm_set1_epi8(tag), _mm_loadu_si128((__m128i*)(deltaLeafNode->tags)));
    bitfield = ((uint16_t)_mm_movemask_epi8(cmp));
    bitfield &= (deltaLeafNode->count == 0 ? 0 : ((0xFFFF) >> (16 - deltaLeafNode->count)));
    while(bitfield)
    {
        idx = ctz_16(bitfield);
        if(memcmp((uint8_t*)key + level, (uint8_t*)deltaLeafNode->kvs + idx * (offsetBytes + VALUE_BYTES), offsetBytes) == 0)
        {
            *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + idx * (offsetBytes + VALUE_BYTES) + offsetBytes) = value;
            return;
        }
        bitfield ^= (0x1ul << idx);
    }

    // int idx;
    // for(idx = 0; idx < deltaLeafNode->count; idx++)
    // {
    //     if(deltaLeafNode->tags[idx] == tag && memcmp(keyBytes + level, (uint8_t*)deltaLeafNode->kvs + idx * (offsetBytes + VALUE_BYTES), offsetBytes) == 0)
    //     {
    //         *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + idx * (offsetBytes + VALUE_BYTES) + offsetBytes) = value;
    //         return;
    //     }
    // }

    if(deltaLeafNode->count == 16)
    {   
        void* nodes[KEY_BYTES] = {NULL};
        uint8_t deltaLeafNodesPrefix[4] = {0}; uint32_t deltaLeafNodesCount = 0; void* deltaLeafNodes[4] = {NULL};
        int lcpLength = ConstructTree(root, deltaLeafNode, keyBytes, value, offsetBytes, nodes, deltaLeafNodesPrefix, &deltaLeafNodesCount, deltaLeafNodes, level, tag);
        InsertTree(root, deltaLeafNode, parentDeltaLeafNode, nodes, deltaLeafNodesPrefix, deltaLeafNodesCount, deltaLeafNodes, keyBytes, level, lcpLength);
        return;
    }

    if((deltaLeafNode->count & (deltaLeafNode->count - 1)) == 0)
    {
        DeltaLeafNode* newDeltaLeafNode = (DeltaLeafNode*)calloc(1, sizeof(DeltaLeafNode) + deltaLeafNode->count * 2 * (offsetBytes + VALUE_BYTES));
        memcpy(newDeltaLeafNode, deltaLeafNode, sizeof(DeltaLeafNode) + deltaLeafNode->count * (offsetBytes + VALUE_BYTES));
        free(deltaLeafNode);
        deltaLeafNode = newDeltaLeafNode;
        *parentDeltaLeafNode = deltaLeafNode;
    }
    deltaLeafNode->tags[deltaLeafNode->count] = tag;
    memcpy((uint8_t*)deltaLeafNode->kvs + deltaLeafNode->count * (offsetBytes + VALUE_BYTES), key + level, offsetBytes);
    *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + deltaLeafNode->count * (offsetBytes + VALUE_BYTES) + offsetBytes) = value;
    deltaLeafNode->count++;
}

VALUE_TYPE GetDeltaLeafNode(DeltaLeafNode* deltaLeafNode, void* key, int level, uint8_t tag)
{
    uint8_t* keyBytes = (uint8_t*)key;
    uint8_t offsetBytes = KEY_BYTES - level;

    __m128i cmp; uint16_t bitfield; uint8_t idx;
    cmp = _mm_cmpeq_epi8(_mm_set1_epi8(tag), _mm_loadu_si128((__m128i*)(deltaLeafNode->tags)));
    bitfield = ((uint16_t)_mm_movemask_epi8(cmp));
    bitfield &= (deltaLeafNode->count == 0 ? 0 : ((0xFFFF) >> (16 - deltaLeafNode->count)));
    while(bitfield)
    {
        idx = ctz_16(bitfield);
        if(memcmp((uint8_t*)key + level, (uint8_t*)deltaLeafNode->kvs + idx * (offsetBytes + VALUE_BYTES), offsetBytes) == 0)
        {
            return *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + idx * (offsetBytes + VALUE_BYTES) + offsetBytes);
        }
        bitfield ^= (0x1ul << idx);
    }

    // for(int i = 0; i < deltaLeafNode->count; i++)
    // {
    //     if(deltaLeafNode->tags[i] == tag && memcmp((uint8_t*)key + level, (uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES), offsetBytes) == 0)
    //     {
    //         return *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + offsetBytes);
    //     }
    // }
    return 0;
}

void RangeDeltaLeafNode(DeltaLeafNode* deltaLeafNode, vIndexIterator* iter, int* range, bool isAll, std::vector<VALUE_TYPE>& res)
{
    uint8_t offsetBytes = KEY_BYTES - iter->level;
    for(int i = 0; i < deltaLeafNode->count; i++)
    {
        res.push_back(*(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + offsetBytes));
        // printf("%ld\n", *(VALUE_TYPE*)((uint8_t*)deltaLeafNode->kvs + i * (offsetBytes + VALUE_BYTES) + offsetBytes));
        *range = *range - 1;
        if(*range <= 0) return;
    }
    return;
}