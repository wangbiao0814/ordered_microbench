#include "vIndexNode.h"
#include "CSRNode.h"
#include "BitmapLeafNode.h"
#include "DeltaLeafNode.h"



void InsertvIndexNode(vIndex* root, vIndexNode* node, void** parrentNode, void* key, VALUE_TYPE value, int level, uint64_t* levelKeys, uint8_t tag) // finish
{
    CSRNode* csrNode = NULL; DeltaLeafNode* deltaLeafNode = NULL; BitmapLeafNode* bitmapLeafNode = NULL;
    switch (node->type)
    {
    case CSR_NODE:
        csrNode = (CSRNode*)node;
        InsertCSRNode(root, csrNode, parrentNode, key, value, level, levelKeys, tag);
        break;
    case DELTA_LEAF_NODE:
        deltaLeafNode = (DeltaLeafNode*)node;
        InsertDeltaLeafNode(root, deltaLeafNode, parrentNode, key, value, level, tag);
        break;
    case BITMAP_LEAF_NODE:
        bitmapLeafNode = (BitmapLeafNode*)node;
        InsertBitmapLeafNode(bitmapLeafNode, parrentNode, key, value, level);
        break;
    default:
        break;
    }
}


VALUE_TYPE GetvIndexNode(vIndexNode* node, void* key, int level, uint8_t tag) // finish
{
    CSRNode* csrNode = NULL; DeltaLeafNode* deltaLeafNode = NULL; BitmapLeafNode* bitmapLeafNode = NULL;
    switch (node->type)
    {
    case CSR_NODE:
        csrNode = (CSRNode*)node;
        return GetCSRNode(csrNode, key, level, tag);
    case DELTA_LEAF_NODE:
        deltaLeafNode = (DeltaLeafNode*)node;
        return GetDeltaLeafNode(deltaLeafNode, key, level, tag);
    case BITMAP_LEAF_NODE:
        bitmapLeafNode = (BitmapLeafNode*)node;
        return GetBitmapLeafNode(bitmapLeafNode, key, level);
    default:
        break;
    }
    return 0;
}

void RangevIndexNode(vIndex* root, vIndexIterator* iter, int* range, std::vector<VALUE_TYPE>& res)
{

    CSRNode* csrNode = NULL; DeltaLeafNode* deltaLeafNode = NULL; BitmapLeafNode* bitmapLeafNode = NULL; bool isAll = false;
    while(*range > 0)
    {
        switch (iter->nodes[iter->level >> 1]->type)
        {
        case CSR_NODE:
            csrNode = (CSRNode*)iter->nodes[iter->level >> 1];
            RangeCSRNode(root, csrNode, iter, range, isAll, res);
            break;
        case DELTA_LEAF_NODE:
            deltaLeafNode = (DeltaLeafNode*)iter->nodes[iter->level >> 1];
            RangeDeltaLeafNode(deltaLeafNode, iter, range, isAll, res);
            break;
        default:
            break;
        }
        if(*range <= 0)
        {
            return;
        }
        else
        {
            if(iter->level == 2)
            {
                for(int i = iter->levelKeys[iter->level >> 1] + 1; i < 65536; i++)
                {
                    if(root->directPointers[i] != NULL)
                    {
                        iter->nodes[iter->level >> 1] = (vIndexNode*)root->directPointers[i];
                        iter->levelKeys[iter->level >> 1] = i; isAll = true;
                        break;
                    }
                }
                if(!isAll) return;
            }
            else if(iter->level == 4)
            {
                iter->levelKeys[iter->level >> 1] += 1;
                if(iter->levelKeys[(iter->level >> 1) - 1] != (iter->levelKeys[iter->level >> 1] >> 16))
                {
                    for(int i = (iter->levelKeys[iter->level >> 1] >> 16); i < 65536; i++)
                    {
                        if(root->directPointers[i] != NULL)
                        {
                            iter->level = 2;
                            iter->nodes[iter->level >> 1] = (vIndexNode*)root->directPointers[i];
                            iter->levelKeys[iter->level >> 1] = i; isAll = true;
                            break;
                        }
                    }
                    if(!isAll) return;
                }
                else
                {
                    iter->bytes[iter->level - 2] = ((iter->levelKeys[iter->level >> 1] >> 8) & 0xFF);
                    iter->bytes[iter->level - 1] = ((iter->levelKeys[iter->level >> 1]) & 0xFF);
                    memset(iter->bytes + iter->level, 0, KEY_BYTES - iter->level);
                    iter->level -= 2;
                }
                // printf("iter->level = %d, key = %ld\n", iter->level, iter->levelKeys[2]);
                // iter->levelKeys[iter->level >> 1] = ((iter->levelKeys[iter->level >> 1] >> 16) << 16) + (((iter->levelKeys[iter->level >> 1] & 0xFFFF) + 1) & 0xFFFF);
                
            }
            else if(iter->level == 6)
            {
                iter->levelKeys[iter->level >> 1] += 1;
                if(iter->levelKeys[(iter->level >> 1) - 1] != (iter->levelKeys[iter->level >> 1] >> 16))
                {
                    if(iter->levelKeys[(iter->level >> 1) - 2] != (iter->levelKeys[iter->level >> 1] >> 32))
                    {
                        for(int i = (iter->levelKeys[iter->level >> 1] >> 32); i < 65536; i++)
                        {
                            if(root->directPointers[i] != NULL)
                            {
                                iter->level = 2;
                                iter->nodes[iter->level >> 1] = (vIndexNode*)root->directPointers[i];
                                iter->levelKeys[iter->level >> 1] = i; isAll = true;
                                break;
                            }
                        }
                        if(!isAll) return;
                    }
                    else
                    {
                        iter->levelKeys[(iter->level >> 1) - 1] = (iter->levelKeys[iter->level >> 1] >> 16);
                        iter->level = 2;
                        iter->bytes[iter->level] = ((iter->levelKeys[(iter->level >> 1) + 1] >> 8) & 0xFF);
                        iter->bytes[iter->level + 1] = ((iter->levelKeys[(iter->level >> 1) + 1]) & 0xFF);
                        memset(iter->bytes + iter->level + 2, 0, KEY_BYTES - iter->level - 2);
                    }
                }
                else
                {
                    iter->bytes[iter->level - 2] = ((iter->levelKeys[iter->level >> 1] >> 8) & 0xFF);
                    iter->bytes[iter->level - 1] = ((iter->levelKeys[iter->level >> 1]) & 0xFF);
                    memset(iter->bytes + iter->level, 0, KEY_BYTES - iter->level);
                    iter->level -= 2;
                }
                // printf("iter->level = %d, key = %ld\n", iter->level, iter->levelKeys[3]);
            }
        }
    }
    return;
}