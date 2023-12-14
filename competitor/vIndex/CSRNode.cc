#include "CSRNode.h"
#include "DeltaLeafNode.h"
#include "BitmapLeafNode.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

void InsertCSRNodeWithoutDumplicatedKey(CSRNode* csrNode, uint8_t* keyBytes, VALUE_TYPE value, int offsetBytes, uint8_t tag)
{
    int csrNodeCount = GetCSRNodeCount(csrNode, keyBytes[0]);
    int csrNodeCumsum = GetCSRNodeCumsum(csrNode, keyBytes[0]);
    int blockIndex = (keyBytes[0] >> 5);  

    int csrNodeTotalCount = GetCSRNodeTotalCount(csrNode, blockIndex);
    if(csrNodeTotalCount == 0)
    {
        csrNode->CSR[blockIndex] = (uint8_t*)calloc(1, 8 * (offsetBytes + 1 + VALUE_BYTES));
    }
    else if((csrNodeTotalCount & (0x7)) == 0)
    {
        uint8_t* newCSR = (uint8_t*)calloc(1, (csrNodeTotalCount + 8) * (offsetBytes + 1 + VALUE_BYTES));
        memcpy(newCSR, csrNode->CSR[blockIndex], csrNodeTotalCount * (offsetBytes + 1 + VALUE_BYTES));
        free(csrNode->CSR[blockIndex]);
        csrNode->CSR[blockIndex] = newCSR;
    }
    // if(csrNodeTotalCount == 0)
    // {
    //     csrNode->CSR[blockIndex] = (uint8_t*)calloc(1, 1 * (offsetBytes + 1 + VALUE_BYTES));
    // }
    // else if((csrNodeTotalCount & (0x7)) == 0)
    // {
    //     uint8_t* newCSR = (uint8_t*)calloc(1, (csrNodeTotalCount + 8) * (offsetBytes + 1 + VALUE_BYTES));
    //     memcpy(newCSR, csrNode->CSR[blockIndex], csrNodeTotalCount * (offsetBytes + 1 + VALUE_BYTES));
    //     free(csrNode->CSR[blockIndex]);
    //     csrNode->CSR[blockIndex] = newCSR;
    // }
    // else if((csrNodeTotalCount & (csrNodeTotalCount - 1)) == 0)
    // {
    //     uint8_t* newCSR = (uint8_t*)calloc(1, (csrNodeTotalCount << 1) * (offsetBytes + 1 + VALUE_BYTES));
    //     memcpy(newCSR, csrNode->CSR[blockIndex], csrNodeTotalCount * (offsetBytes + 1 + VALUE_BYTES));
    //     free(csrNode->CSR[blockIndex]);
    //     csrNode->CSR[blockIndex] = newCSR;
    // }
    for(int i = csrNodeTotalCount - 1; i >= csrNodeCumsum + csrNodeCount; i--)
    {
        memcpy((uint8_t*)csrNode->CSR[blockIndex] + (i + 1) * (offsetBytes + 1 + VALUE_BYTES), (uint8_t*)csrNode->CSR[blockIndex] + (i) * (offsetBytes + 1 + VALUE_BYTES), (offsetBytes + 1 + VALUE_BYTES));
    }
    *(uint8_t*)((uint8_t*)csrNode->CSR[blockIndex] + (csrNodeCumsum + csrNodeCount) * (offsetBytes + 1 + VALUE_BYTES)) = tag;
    memcpy((uint8_t*)csrNode->CSR[blockIndex] + (csrNodeCumsum + csrNodeCount) * (offsetBytes + 1 + VALUE_BYTES) + 1, keyBytes + 1, offsetBytes);
    *(VALUE_TYPE*)((uint8_t*)csrNode->CSR[blockIndex] + (csrNodeCumsum + csrNodeCount) * (offsetBytes + 1 + VALUE_BYTES) + offsetBytes + 1) = value;
    AddCSRNodeCount(csrNode, keyBytes[0]);
}

void InsertCSRNodeOnly(CSRNode* csrNode, int blockIndex, int csrNodeCount, int csrNodeCumsum, uint8_t* keyBytes, int level, VALUE_TYPE value, int tagOffsetBytes, uint8_t tag) //finished
{
    int csrNodeTotalCount = GetCSRNodeTotalCount(csrNode, blockIndex);
    if(csrNodeTotalCount == 0)
    {
        csrNode->CSR[blockIndex] = (uint8_t*)calloc(1, 8 * (tagOffsetBytes + VALUE_BYTES));
    }
    else if((csrNodeTotalCount & (0x7)) == 0)
    {
        csrNode->CSR[blockIndex] = (uint8_t*)realloc(csrNode->CSR[blockIndex], (csrNodeTotalCount + 8) * (tagOffsetBytes + VALUE_BYTES));
    }
    // if(csrNodeTotalCount == 0)
    // {
    //     csrNode->CSR[blockIndex] = (uint8_t*)calloc(1, 8 * (tagOffsetBytes + VALUE_BYTES));
    // }
    // else if((csrNodeTotalCount & (0x7)) == 0)
    // {
    //     csrNode->CSR[blockIndex] = (uint8_t*)realloc(csrNode->CSR[blockIndex], (csrNodeTotalCount + 8) * (tagOffsetBytes + VALUE_BYTES));
    // }
    // else if((csrNodeTotalCount & (csrNodeTotalCount - 1)) == 0)
    // {
    //     csrNode->CSR[blockIndex] = (uint8_t*)realloc(csrNode->CSR[blockIndex], (csrNodeTotalCount << 1) * (tagOffsetBytes + VALUE_BYTES));
    // }
    for(int i = csrNodeTotalCount - 1; i >= csrNodeCumsum + csrNodeCount; i--)
    {
        memcpy((uint8_t*)csrNode->CSR[blockIndex] + (i + 1) * (tagOffsetBytes + VALUE_BYTES), (uint8_t*)csrNode->CSR[blockIndex] + (i) * (tagOffsetBytes + VALUE_BYTES), (tagOffsetBytes + VALUE_BYTES));
    }
    *(uint8_t*)((uint8_t*)csrNode->CSR[blockIndex] + (csrNodeCumsum + csrNodeCount) * (tagOffsetBytes + VALUE_BYTES)) = tag;
    memcpy((uint8_t*)csrNode->CSR[blockIndex] + (csrNodeCumsum + csrNodeCount) * (tagOffsetBytes + VALUE_BYTES) + 1, keyBytes + level + 1, tagOffsetBytes - 1);
    *(VALUE_TYPE*)((uint8_t*)csrNode->CSR[blockIndex] + (csrNodeCumsum + csrNodeCount) * (tagOffsetBytes + VALUE_BYTES) + tagOffsetBytes) = value;
    AddCSRNodeCount(csrNode, keyBytes[level]);
}

void DeleteCSRNodeKeys(CSRNode* csrNode, int blockIndex, int csrNodeCount, int csrNodeCumsum, uint8_t* keyBytes, int level, int tagOffsetBytes) //finish
{
    int csrNodeTotalCount = GetCSRNodeTotalCount(csrNode, blockIndex);
    int csrNodeCapacity = (((csrNodeTotalCount - 1) >> 3) << 3) + 8;
    for(int i = csrNodeCumsum + csrNodeCount; i < csrNodeTotalCount; i++)
    {
        memcpy((uint8_t*)csrNode->CSR[blockIndex] + (i - csrNodeCount) * (tagOffsetBytes + VALUE_BYTES) , (uint8_t*)csrNode->CSR[blockIndex] + i * (tagOffsetBytes + VALUE_BYTES), tagOffsetBytes + VALUE_BYTES);
    }
    AddCSRNodeCount(csrNode, keyBytes[level]); csrNode->sum[keyBytes[level] >> 4] -= 4; csrNodeTotalCount -= 3;
    int csrNodeNewCapacity = (((csrNodeTotalCount - 1) >> 3) << 3) + 8;
    if(csrNodeTotalCount == 0)
    {
        free(csrNode->CSR[blockIndex]); csrNode->CSR[blockIndex] = NULL;
    }
    else if(csrNodeNewCapacity < csrNodeCapacity)
    {
        uint8_t* newCSR = (uint8_t*)calloc(1, csrNodeNewCapacity * (tagOffsetBytes + VALUE_BYTES));
        memcpy(newCSR, csrNode->CSR[blockIndex], (csrNodeTotalCount) * (tagOffsetBytes + VALUE_BYTES));
        free(csrNode->CSR[blockIndex]);
        csrNode->CSR[blockIndex] = newCSR;
        // csrNode->CSR[blockIndex] = (uint8_t*)realloc(csrNode->CSR[blockIndex], (csrNodeTotalCount) * (tagOffsetBytes + VALUE_BYTES));
    }
}

void InsertCSRNode(vIndex* root, CSRNode* csrNode, void** parrentCSRNode, void* key, VALUE_TYPE value, int level, uint64_t* levelKeys, uint8_t tag) 
{
    uint8_t* keyBytes = (uint8_t*)key;
    uint8_t tagOffsetBytes = KEY_BYTES - level - 1 + 1;

    
    int csrNodeCount = GetCSRNodeCount(csrNode, keyBytes[level]);
    int csrNodeCumsum = GetCSRNodeCumsum(csrNode, keyBytes[level]);
    int blockIndex = (keyBytes[level] >> 5);

    if(csrNodeCount == 0 && GetCSRNodeBitmap(csrNode, keyBytes[level]))
    {
        int csrNodeBitmapPopcntAll = GetCSRNodeBitmapPopcntAll(csrNode);
        int csrNodeBitmapPopcnt = csrNodeBitmapPopcntAll > 128 ? keyBytes[level] : GetCSRNodeBitmapPopcnt(csrNode, keyBytes[level]) - 1;
        
        vIndexNode* baseNode = (vIndexNode*)csrNode->nodes[csrNodeBitmapPopcnt];
        switch (baseNode->type)
        {
        case CSR_NODE:
            InsertCSRNode(root, (CSRNode*)baseNode, &csrNode->nodes[csrNodeBitmapPopcnt], key, value, level + 1, levelKeys, tag);
            break;
        case DELTA_LEAF_NODE:
            InsertDeltaLeafNode(root, (DeltaLeafNode*)baseNode, &csrNode->nodes[csrNodeBitmapPopcnt], key, value, level + 1, tag);
            break;
        case BITMAP_LEAF_NODE:
            InsertBitmapLeafNode((BitmapLeafNode*)baseNode, &csrNode->nodes[csrNodeBitmapPopcnt], key, value, level + 1);
            break;
        }
        return;
    }   

    for(int i = csrNodeCumsum; i < csrNodeCumsum + csrNodeCount; i++)
    {
        
        if(*(uint8_t*)((uint8_t*)csrNode->CSR[blockIndex] + i * (tagOffsetBytes + VALUE_BYTES)) == tag \
        && memcmp(keyBytes + level + 1, (uint8_t*)csrNode->CSR[blockIndex] + i * (tagOffsetBytes + VALUE_BYTES) + 1, tagOffsetBytes - 1) == 0)
        {
            *(VALUE_TYPE*)((uint8_t*)csrNode->CSR[blockIndex] + i * (tagOffsetBytes + VALUE_BYTES) + tagOffsetBytes) = value;
            return;
        }
    }

    if(csrNodeCount == 3)
    {
        DeltaLeafNode* deltaLeafNode = (DeltaLeafNode*)calloc(1, sizeof(DeltaLeafNode) + 4 * (tagOffsetBytes - 1 + VALUE_BYTES));
        deltaLeafNode->type = DELTA_LEAF_NODE;
        for(int i = csrNodeCumsum; i < csrNodeCumsum + csrNodeCount; i++)
        {
            deltaLeafNode->tags[deltaLeafNode->count] = *((uint8_t*)csrNode->CSR[blockIndex] + i * (tagOffsetBytes + VALUE_BYTES));
            memcpy((uint8_t*)deltaLeafNode->kvs + (deltaLeafNode->count) * (tagOffsetBytes - 1 + VALUE_BYTES), \
                    (uint8_t*)(csrNode->CSR[blockIndex] + i * (tagOffsetBytes + VALUE_BYTES)) + 1, tagOffsetBytes - 1 + VALUE_BYTES);
            deltaLeafNode->count++;
        }
        InsertInitDeltaLeafNode(deltaLeafNode, key, value, level + 1, tag);
        if(isUpperLevel(level + 1))
        {
#ifdef USE_FLATHASH
            (*root->levelHash[(level + 1) >> 1])[levelKeys[(level + 1) >> 1]] = deltaLeafNode;
#else
            InsertHashTable(root->levelHash[(level + 1) >> 1], levelKeys[(level + 1) >> 1], deltaLeafNode);
#endif
            // return;
        }
        else
        {
            int csrNodeBitmapPopcntAll = GetCSRNodeBitmapPopcntAll(csrNode);
            int csrNodeBitmapPopcnt = csrNodeBitmapPopcntAll > 128 ? keyBytes[level] : GetCSRNodeBitmapPopcnt(csrNode, keyBytes[level]);
            if(csrNodeBitmapPopcntAll >= 128)
            {
                if(csrNodeBitmapPopcntAll == 128)
                {
                    CSRNode* newCSRNode = (CSRNode*)calloc(1, sizeof(CSRNode) + 2 * csrNodeBitmapPopcntAll * sizeof(void*)); int idx = 0;
                    memcpy(newCSRNode, csrNode, sizeof(CSRNode));
                    for(int j = 0; j < 256; j++)
                    {
                        if(GetCSRNodeBitmap(csrNode, j))
                        {
                            newCSRNode->nodes[j] = csrNode->nodes[idx++];
                        }
                    }
                    free(csrNode);
                    csrNode = newCSRNode;
                    *parrentCSRNode = newCSRNode;
                }
                csrNode->nodes[keyBytes[level]] = deltaLeafNode;
            }
            else
            {
                if((csrNodeBitmapPopcntAll & (csrNodeBitmapPopcntAll - 1)) == 0 && csrNodeBitmapPopcntAll >= 4)
                {
                    CSRNode* newCSRNode = (CSRNode*)calloc(1, sizeof(CSRNode) + 2 * csrNodeBitmapPopcntAll * sizeof(void*)); 
                    memcpy(newCSRNode, csrNode, sizeof(CSRNode) + csrNodeBitmapPopcntAll * sizeof(void*));
                    free(csrNode);
                    csrNode = newCSRNode;
                    *parrentCSRNode = newCSRNode;
                }
                for(int j = csrNodeBitmapPopcntAll; j > csrNodeBitmapPopcnt; j--) csrNode->nodes[j] = csrNode->nodes[j - 1];
                csrNode->nodes[csrNodeBitmapPopcnt] = deltaLeafNode;
            }
        }
        SetCSRNodeBitmap(csrNode, keyBytes[level]);
        DeleteCSRNodeKeys((CSRNode*)*parrentCSRNode, blockIndex, csrNodeCount, csrNodeCumsum, keyBytes, level, tagOffsetBytes);
        return;
    }
    InsertCSRNodeOnly(csrNode, blockIndex, csrNodeCount, csrNodeCumsum, keyBytes, level, value, tagOffsetBytes, tag);
}

VALUE_TYPE GetCSRNode(CSRNode* csrNode, void* key, int level, uint8_t tag)
{
    uint8_t* keyBytes = (uint8_t*)key;
    uint8_t tagOffsetBytes = KEY_BYTES - level - 1 + 1;

    int csrNodeCount = GetCSRNodeCount(csrNode, keyBytes[level]);
    int csrNodeCumsum = GetCSRNodeCumsum(csrNode, keyBytes[level]);
    int blockIndex = (keyBytes[level] >> 5);
    
    if(csrNodeCount == 0 && GetCSRNodeBitmap(csrNode, keyBytes[level]))
    {
        int csrNodeBitmapPopcntAll = GetCSRNodeBitmapPopcntAll(csrNode);
        int csrNodeBitmapPopcnt = csrNodeBitmapPopcntAll > 128 ? keyBytes[level] : GetCSRNodeBitmapPopcnt(csrNode, keyBytes[level]) - 1;
        
        vIndexNode* baseNode = (vIndexNode*)csrNode->nodes[csrNodeBitmapPopcnt];
        switch (baseNode->type)
        {
        case CSR_NODE:
            return GetCSRNode((CSRNode*)baseNode, key, level + 1, tag);
        case DELTA_LEAF_NODE:
            return GetDeltaLeafNode((DeltaLeafNode*)baseNode, key, level + 1, tag);
        case BITMAP_LEAF_NODE:
            return GetBitmapLeafNode((BitmapLeafNode*)baseNode, key, level + 1);
        }
    }
    // if(csrNodeCount != 0)
    // {
    //     return 0;
    //     printf("%ld\n", csrNodeCount);
    // }
    for(int i = csrNodeCumsum; i < csrNodeCumsum + csrNodeCount; i++)
    {
        if(*(uint8_t*)((uint8_t*)csrNode->CSR[blockIndex] + i * (tagOffsetBytes + VALUE_BYTES)) == tag \
        && memcmp(keyBytes + level + 1, (uint8_t*)csrNode->CSR[blockIndex] + i * (tagOffsetBytes + VALUE_BYTES) + 1, tagOffsetBytes - 1) == 0)
        {
            return *(VALUE_TYPE*)((uint8_t*)csrNode->CSR[blockIndex] + i * (tagOffsetBytes + VALUE_BYTES) + tagOffsetBytes);
        }
        // return 0;
    }
    return 0;

}


void RangeCSRNode(vIndex* root, CSRNode* csrNode, vIndexIterator* iter, int* range, bool isAll, std::vector<VALUE_TYPE>& res)
{
    uint8_t tagOffsetBytes = KEY_BYTES - iter->level - 1 + 1;
    int key = isAll ? 0 : iter->bytes[iter->level];
    int csrNodeCumsum = GetCSRNodeCumsum(csrNode, key);
    int csrNodeBitmapPopcntAll = GetCSRNodeBitmapPopcntAll(csrNode);
    int tempCSRNodeBitmapPopcnt = GetCSRNodeBitmapPopcnt(csrNode, key);
    if(GetCSRNodeBitmap(csrNode, key)) tempCSRNodeBitmapPopcnt -= 1;
    
    for(; key < 256; key++)
    {
        int csrNodeCount = GetCSRNodeCount(csrNode, key);
        int csrNodeCumsum = GetCSRNodeCumsum(csrNode, key);
        int blockIndex = (key >> 5);
        
        if((key & 0x1f) == 0x0) 
        {
            csrNodeCumsum = GetCSRNodeCumsum(csrNode, key);
        }
        if(csrNodeCount == 0 && GetCSRNodeBitmap(csrNode, key))
        {
            vIndexNode* baseNode = NULL;
            if(!isUpperLevel(iter->level))
            {
                iter->offset = ((iter->offset & 0xFF00) + key);
                baseNode = (vIndexNode*)(*root->levelHash[(iter->level + 1) >> 1])[(iter->levelKeys[(iter->level - 1) >> 1] << 16) + iter->offset];
                
            }
            else
            {   
                iter->offset = ((key << 8) + (iter->offset & 0xFF));
                int csrNodeBitmapPopcnt = csrNodeBitmapPopcntAll > 128 ? key : tempCSRNodeBitmapPopcnt;
                baseNode = (vIndexNode*)csrNode->nodes[csrNodeBitmapPopcnt]; tempCSRNodeBitmapPopcnt++;
            }

            uint64_t tempLevelKey; uint32_t tempOffset;
            iter->level++;
            switch (baseNode->type)
            {
            case CSR_NODE:
                if(isUpperLevel(iter->level))
                {
                    tempLevelKey = iter->levelKeys[(iter->level) >> 1];
                    tempOffset = iter->offset;
                    iter->levelKeys[(iter->level) >> 1] =  (iter->levelKeys[((iter->level) >> 1) - 1] << 16) + iter->offset;
                    iter->offset = 0;
                }
                RangeCSRNode(root, (CSRNode*)baseNode, iter, range, isAll, res);
                if(isUpperLevel(iter->level))
                {
                    iter->offset = tempOffset;
                    iter->levelKeys[(iter->level) >> 1] = tempLevelKey;
                }
                break;
            case DELTA_LEAF_NODE:
                RangeDeltaLeafNode((DeltaLeafNode*)baseNode, iter, range, isAll, res);
                break;
            case BITMAP_LEAF_NODE:
                RangeBitmapLeafNode((BitmapLeafNode*)baseNode, iter, range, isAll, res);
                break;
            }
            iter->level--;
        }
        if(*range <= 0) return;
        for(int i = csrNodeCumsum; i < csrNodeCumsum + csrNodeCount; i++)
        {
            {
                *range = *range - 1;
                res.push_back(*(VALUE_TYPE*)((uint8_t*)csrNode->CSR[blockIndex] + i * (tagOffsetBytes + VALUE_BYTES) + tagOffsetBytes));
            }
        }
        csrNodeCumsum += csrNodeCount;
        isAll = true;
    }
    return;
}