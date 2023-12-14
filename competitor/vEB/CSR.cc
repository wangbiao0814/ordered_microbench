#include "CSR.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

void InsertBitmapLeafNodeWithoutDumplicatedKey(BitmapLeafNode* bitmapLeafNode, uint8_t key, VALUE_TYPE value)
{
    int bitmapIndex = GetBitmapLeafNodeBitmapPopcnt(bitmapLeafNode, key);
    int bitmapTotal = GetBitmapLeafNodeBitmapPopcntAll(bitmapLeafNode);
    for(int i = bitmapTotal; i > bitmapIndex; i--) bitmapLeafNode->values[i] = bitmapLeafNode->values[i - 1];    
    SetBitmapLeafNodeBitmap(bitmapLeafNode, key);
    bitmapLeafNode->values[bitmapIndex] = value;
}

void InsertCSRNodeWithoutDumplicatedKey(CSRNode* csrNode, uint8_t* keyBytes, VALUE_TYPE value, int offsetBytes)
{
    int csrNodeCount = GetCSRNodeCount(csrNode, keyBytes[0]);
    int csrNodeCumsum = GetCSRNodeCumsum(csrNode, keyBytes[0]);
    int blockIndex = (keyBytes[0] >> 6);
    int csrNodeTotalCount = GetCSRNodeTotalCount(csrNode, blockIndex);
    if(csrNodeTotalCount == 0)
    {
        csrNode->CSR[blockIndex] = (uint8_t*)calloc(1, 8 * (offsetBytes + VALUE_BYTES));
    }
    else if((csrNodeTotalCount & (0x7)) == 0)
    {
        csrNode->CSR[blockIndex] = (uint8_t*)realloc(csrNode->CSR[blockIndex], (csrNodeTotalCount + 8) * (offsetBytes + VALUE_BYTES));
    }
    for(int i = csrNodeTotalCount - 1; i >= csrNodeCumsum + csrNodeCount; i--)
    {
        memcpy((uint8_t*)csrNode->CSR[blockIndex] + (i + 1) * (offsetBytes + VALUE_BYTES), (uint8_t*)csrNode->CSR[blockIndex] + (i) * (offsetBytes + VALUE_BYTES), (offsetBytes + VALUE_BYTES));
    }
    memcpy((uint8_t*)csrNode->CSR[blockIndex] + (csrNodeCumsum + csrNodeCount) * (offsetBytes + VALUE_BYTES), keyBytes + 1, offsetBytes);
    *(VALUE_TYPE*)((uint8_t*)csrNode->CSR[blockIndex] + (csrNodeCumsum + csrNodeCount) * (offsetBytes + VALUE_BYTES) + offsetBytes) = value;
    AddCSRNodeCount(csrNode, keyBytes[0]);
}

void InsertCSRNodeOnly(CSRNode* csrNode, int blockIndex, int csrNodeCount, int csrNodeCumsum, uint8_t* keyBytes, int level, VALUE_TYPE value, int offsetBytes)
{
    int csrNodeTotalCount = GetCSRNodeTotalCount(csrNode, blockIndex);
    if(csrNodeTotalCount == 0)
    {
        csrNode->CSR[blockIndex] = (uint8_t*)calloc(1, 8 * (offsetBytes + VALUE_BYTES));
    }
    else if((csrNodeTotalCount & (0x7)) == 0)
    {
        csrNode->CSR[blockIndex] = (uint8_t*)realloc(csrNode->CSR[blockIndex], (csrNodeTotalCount + 8) * (offsetBytes + VALUE_BYTES));
    }
    for(int i = csrNodeTotalCount - 1; i >= csrNodeCumsum + csrNodeCount; i--)
    {
        memcpy((uint8_t*)csrNode->CSR[blockIndex] + (i + 1) * (offsetBytes + VALUE_BYTES), (uint8_t*)csrNode->CSR[blockIndex] + (i) * (offsetBytes + VALUE_BYTES), (offsetBytes + VALUE_BYTES));
    }
    memcpy((uint8_t*)csrNode->CSR[blockIndex] + (csrNodeCumsum + csrNodeCount) * (offsetBytes + VALUE_BYTES), keyBytes + level + 1, offsetBytes);
    *(VALUE_TYPE*)((uint8_t*)csrNode->CSR[blockIndex] + (csrNodeCumsum + csrNodeCount) * (offsetBytes + VALUE_BYTES) + offsetBytes) = value;
    AddCSRNodeCount(csrNode, keyBytes[level]);
}

int ConstructCSRNodeTree(CSRNode* csrNode, int blockIndex, int csrNodeCount, int csrNodeCumsum, uint8_t* keyBytes, VALUE_TYPE value, int offsetBytes, void* nodes[], int level)
{
    uint8_t lcp[KEY_BYTES] = {0}; int lcpLength = offsetBytes;
    memcpy(lcp, keyBytes + level + 1, offsetBytes);
    for(int i = csrNodeCumsum; i < csrNodeCumsum + csrNodeCount; i++)
    {
        uint8_t* nodeKey = ((uint8_t*)csrNode->CSR[blockIndex] + i * (offsetBytes + VALUE_BYTES)); int j;
        for(j = 0; j < lcpLength; j++)
        {
            if(nodeKey[j] != lcp[j]) break;
        }
        lcpLength = j;
    }
    //Find the longest common prefix, build upper and lower CSR node using the longest common prefix and a byte after the longest common prefix
    CSRNode* tempCSRNode = NULL; BitmapLeafNode* tempBitmapLeafNode = NULL; int levelIndex;
    for(levelIndex = level + 1; levelIndex <= level + lcpLength; levelIndex++)
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
        tempBitmapLeafNode = (BitmapLeafNode*)calloc(1, sizeof(BitmapLeafNode) + 2 * sizeof(VALUE_TYPE));
        tempBitmapLeafNode->type = BITMAP_LEAF_NODE;
        for(int i = csrNodeCumsum; i < csrNodeCumsum + csrNodeCount; i++)
        {
            InsertBitmapLeafNodeWithoutDumplicatedKey(tempBitmapLeafNode, *((uint8_t*)csrNode->CSR[blockIndex] + i * (offsetBytes + VALUE_BYTES) + offsetBytes - 1), \
                                                *(VALUE_TYPE*)((uint8_t*)csrNode->CSR[blockIndex] + i * (offsetBytes + VALUE_BYTES) + offsetBytes));
        }
        InsertBitmapLeafNodeWithoutDumplicatedKey(tempBitmapLeafNode, *(keyBytes + KEY_BYTES - 1), value);
        nodes[levelIndex] = (void*)tempBitmapLeafNode;
    }
    else
    {
        if(isUpperLevel(levelIndex))
            tempCSRNode = (CSRNode*)calloc(1, sizeof(CSRNode) + CSR_CHILD_INIT_SIZE * sizeof(void*));
        else
            tempCSRNode = (CSRNode*)calloc(1, sizeof(CSRNode));  
        for(int i = csrNodeCumsum; i < csrNodeCumsum + csrNodeCount; i++)
        {
            InsertCSRNodeWithoutDumplicatedKey(tempCSRNode, (uint8_t*)csrNode->CSR[blockIndex] + i * (offsetBytes + VALUE_BYTES) + lcpLength, \
                    *(VALUE_TYPE*)((uint8_t*)csrNode->CSR[blockIndex] + i * (offsetBytes + VALUE_BYTES) + offsetBytes), offsetBytes - lcpLength - 1);
        }
        InsertCSRNodeWithoutDumplicatedKey(tempCSRNode, keyBytes + level + 1 + lcpLength, value, offsetBytes - lcpLength - 1);
        nodes[levelIndex] = (void*)tempCSRNode;
    }
    return lcpLength;
}

void InsertCSRNodeTree(ASTree* root, CSRNode* csrNode, void** parrentCSRNode, void* nodes[], uint8_t* keyBytes, int level, int lcpLength, uint64_t* levelKeys)
{
    CSRNode* tempCSRNode = NULL;
    for(int i = level + 1; i <= level + lcpLength + 1; i++)
    {
        if(isUpperLevel(i))
        {
            // InsertHashTable(root->levelHash[i >> 1], levelKeys[i >> 1], nodes[i]);
            (*root->levelHash[i >> 1])[levelKeys[i >> 1]] = nodes[i];
            if(i == level + 1)
            {
                SetCSRNodeBitmap(csrNode, keyBytes[level]);
            }
        }
        else if(i == level + 1)
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
                csrNode->nodes[keyBytes[level]] = nodes[i];
                SetCSRNodeBitmap(csrNode, keyBytes[level]);
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
                csrNode->nodes[csrNodeBitmapPopcnt] = nodes[i];
                SetCSRNodeBitmap(csrNode, keyBytes[level]);
            }
        }
        else
        {
            tempCSRNode = (CSRNode*)nodes[i - 1]; tempCSRNode->nodes[0] = nodes[i];
        }
    }
}

void DeleteCSRNodeKeys(CSRNode* csrNode, int blockIndex, int csrNodeCount, int csrNodeCumsum, uint8_t* keyBytes, int level, int offsetBytes)
{
    int csrNodeTotalCount = GetCSRNodeTotalCount(csrNode, blockIndex);
    int csrNodeCapacity = (((csrNodeTotalCount - 1) >> 3) << 3) + 8;

    for(int i = csrNodeCumsum + csrNodeCount; i < csrNodeTotalCount; i++)
    {
        memcpy((uint8_t*)csrNode->CSR[blockIndex] + (i - csrNodeCount) * (offsetBytes + VALUE_BYTES) , (uint8_t*)csrNode->CSR[blockIndex] + i * (offsetBytes + VALUE_BYTES), offsetBytes + VALUE_BYTES);
    }
    // AddCSRNodeCount(csrNode, keyBytes[level]); csrNode->sum[keyBytes[level] >> 4] -= 4; csrNodeTotalCount -= 3;
    AddCSRNodeCount(csrNode, keyBytes[level]); csrNode->csrRank[keyBytes[level] >> 6] -= 2; csrNodeTotalCount -= 1;
    int csrNodeNewCapacity = (((csrNodeTotalCount - 1) >> 3) << 3) + 8;
    if(csrNodeTotalCount == 0)
    {
        free(csrNode->CSR[blockIndex]); csrNode->CSR[blockIndex] = NULL;
    }
    // else if((csrNodeTotalCount & (0x7)) == 0)
    else if(csrNodeNewCapacity < csrNodeCapacity)
    {
        csrNode->CSR[blockIndex] = (uint8_t*)realloc(csrNode->CSR[blockIndex], (csrNodeNewCapacity) * (offsetBytes + VALUE_BYTES));
    }
}

void InsertCSRNode(ASTree* root, CSRNode* csrNode, void** parrentCSRNode, void* key, VALUE_TYPE value, int level, uint64_t* levelKeys)
{
    uint8_t* keyBytes = (uint8_t*)key;
    uint8_t offsetBytes = KEY_BYTES - level - 1;

    int csrNodeCount = GetCSRNodeCount(csrNode, keyBytes[level]);
    int csrNodeCumsum = GetCSRNodeCumsum(csrNode, keyBytes[level]);
    int blockIndex = (keyBytes[level] >> 6);

    for(int i = csrNodeCumsum; i < csrNodeCumsum + csrNodeCount; i++)
    {
        if(memcmp(keyBytes + level + 1, (uint8_t*)csrNode->CSR[blockIndex] + i * (offsetBytes + VALUE_BYTES), offsetBytes) == 0)
        {
            *(VALUE_TYPE*)((uint8_t*)csrNode->CSR[blockIndex] + i * (offsetBytes + VALUE_BYTES) + offsetBytes) = value;
            return;
        }
    }

    if(csrNodeCount == 1)
    {
        void* nodes[KEY_BYTES] = {NULL};
        int lcpLength = ConstructCSRNodeTree(csrNode, blockIndex, csrNodeCount, csrNodeCumsum, keyBytes, value, offsetBytes, nodes, level);
        InsertCSRNodeTree(root, csrNode, parrentCSRNode, nodes, keyBytes, level, lcpLength, levelKeys);
        DeleteCSRNodeKeys((CSRNode*)*parrentCSRNode, blockIndex, csrNodeCount, csrNodeCumsum, keyBytes, level, offsetBytes);
        return;
    }
    
    if(csrNodeCount == 0 && GetCSRNodeBitmap(csrNode, keyBytes[level]))
    {
        if(!isUpperLevel(level))
        {
            printf("Error when insert new kv!\n");
            return;
        }
        int csrNodeBitmapPopcntAll = GetCSRNodeBitmapPopcntAll(csrNode);
        int csrNodeBitmapPopcnt = csrNodeBitmapPopcntAll > 128 ? keyBytes[level] : GetCSRNodeBitmapPopcnt(csrNode, keyBytes[level]) - 1;
        
        BaseNode* baseNode = (BaseNode*)csrNode->nodes[csrNodeBitmapPopcnt];
        switch (baseNode->type)
        {
        case CSR_NODE:
            InsertCSRNode(root, (CSRNode*)baseNode, &csrNode->nodes[csrNodeBitmapPopcnt], key, value, level + 1, levelKeys);
            break;
        case BITMAP_LEAF_NODE:
            InsertBitmapLeafNode((BitmapLeafNode*)baseNode, &csrNode->nodes[csrNodeBitmapPopcnt], key, value, level + 1);
            break;
        }
        return;
    }   
    InsertCSRNodeOnly(csrNode, blockIndex, csrNodeCount, csrNodeCumsum, keyBytes, level, value, offsetBytes);
}

VALUE_TYPE GetCSRNode(CSRNode* csrNode, void* key, int level)
{
    uint8_t* keyBytes = (uint8_t*)key;
    uint8_t offsetBytes = KEY_BYTES - level - 1;

    int csrNodeCount = GetCSRNodeCount(csrNode, keyBytes[level]);
    int csrNodeCumsum = GetCSRNodeCumsum(csrNode, keyBytes[level]);
    int blockIndex = (keyBytes[level] >> 6);
    
    if(isUpperLevel(level) && csrNodeCount == 0 && GetCSRNodeBitmap(csrNode, keyBytes[level]))
    {
        int csrNodeBitmapPopcntAll = GetCSRNodeBitmapPopcntAll(csrNode);
        int csrNodeBitmapPopcnt = csrNodeBitmapPopcntAll > 128 ? keyBytes[level] : GetCSRNodeBitmapPopcnt(csrNode, keyBytes[level]) - 1;
        
        BaseNode* baseNode = (BaseNode*)csrNode->nodes[csrNodeBitmapPopcnt];
        switch (baseNode->type)
        {
        case CSR_NODE:
            return GetCSRNode((CSRNode*)baseNode, key, level + 1);
        case BITMAP_LEAF_NODE:
            return GetBitmapLeafNode((BitmapLeafNode*)baseNode, key, level + 1);
        }
    }

    for(int i = csrNodeCumsum; i < csrNodeCumsum + csrNodeCount; i++)
    {
        if(memcmp(keyBytes + level + 1, (uint8_t*)csrNode->CSR[blockIndex] + i * (offsetBytes + VALUE_BYTES), offsetBytes ) == 0)
        {
            return *(VALUE_TYPE*)((uint8_t*)csrNode->CSR[blockIndex] + i * (offsetBytes + VALUE_BYTES) + offsetBytes);
            // return 1;
        }
    }
    return 0;
}

void InsertBitmapLeafNode(BitmapLeafNode* bitmapLeafNode, void** parentBitmapLeafNode, void* key, VALUE_TYPE value, int level)
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

VALUE_TYPE GetBitmapLeafNode(BitmapLeafNode* bitmapLeafNode, void* key, int level)
{
    uint8_t* keyBytes = (uint8_t*)key;
    if(GetBitmapLeafNodeBitmap(bitmapLeafNode, keyBytes[level]))
    {
        int bitmapLeafNodePopcntAll = GetBitmapLeafNodeBitmapPopcntAll(bitmapLeafNode);
        int bitmapLeafNodePopcnt = bitmapLeafNodePopcntAll > 128 ? keyBytes[level] : GetBitmapLeafNodeBitmapPopcnt(bitmapLeafNode, keyBytes[level]) - 1;
        return bitmapLeafNode->values[bitmapLeafNodePopcnt];
        // return 1;
    }
    return 0;
}
