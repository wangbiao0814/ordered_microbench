#include "vIndex.h"
#include "vIndexNode.h"
#include "CSRNode.h"

#ifdef USE_FLATHASH
void InitvIndex(vIndex* root) //finish
{
    root->levelHash[2] = new HashTableRoot(256);
    root->levelHash[3] = new HashTableRoot(256);
}

void InsertvIndex(vIndex* root, KEY_TYPE key, VALUE_TYPE value) //finish
{
    uint64_t levelKeys[4] = {0};
    levelKeys[1] = key >> 48; levelKeys[2] = key >> 32; levelKeys[3] = key >> 16; 
    HashTableRoot::iterator nodePointerIters[4];
    uint64_t keyRev = __builtin_bswap64(key);
    uint8_t tag = Hash64(key) >> 56;
    if(!root->levelHash[2]->empty() && (nodePointerIters[2] = root->levelHash[2]->find(levelKeys[2])) != root->levelHash[2]->end())
    {
        if(!root->levelHash[3]->empty() && (nodePointerIters[3] = root->levelHash[3]->find(levelKeys[3])) != root->levelHash[3]->end())
        {
            InsertvIndexNode(root, (vIndexNode*)(nodePointerIters[3]->second), &(nodePointerIters[3]->second), (void*)(&keyRev), value, 6, levelKeys, tag);
        }
        else
        {
            InsertvIndexNode(root, (vIndexNode*)(nodePointerIters[2]->second), &(nodePointerIters[2]->second), (void*)(&keyRev), value, 4, levelKeys, tag);
        }
    } 
    else
    {
        if(root->directPointers[levelKeys[1]])
        {
            InsertvIndexNode(root, (vIndexNode*)root->directPointers[levelKeys[1]], &root->directPointers[levelKeys[1]], (void*)(&keyRev), value, 2, levelKeys, tag);
        }
        else
        {
            CSRNode* csrNode = (CSRNode*)calloc(1, sizeof(CSRNode) + 4 * sizeof(void*));
            InsertCSRNode(root, csrNode, NULL, (void*)(&keyRev), value, 2, levelKeys, tag);
            root->directPointers[levelKeys[1]] = csrNode;
            root->bitmap65536[levelKeys[1] >> 6] |= (1ul << (levelKeys[1] & 0x3F));
            root->bitmap256[levelKeys[1] >> 14] |= (1ul << ((levelKeys[1] >> 8) & 0x3F));
        }
    }
}


VALUE_TYPE GetvIndex(vIndex* root, KEY_TYPE key) //finish
{
    uint64_t levelKeys[4] = {0};
    levelKeys[1] = key >> 48; levelKeys[2] = key >> 32; levelKeys[3] = key >> 16; 
    HashTableRoot::iterator nodePointerIters[4];
    uint64_t hashCodes[4] = {0};
    uint64_t keyRev = __builtin_bswap64(key);
    uint8_t tag = Hash64(key) >> 56;

    if(!root->levelHash[2]->empty() && (nodePointerIters[2] = root->levelHash[2]->find(levelKeys[2])) != root->levelHash[2]->end())
    {
        if(!root->levelHash[3]->empty() &&(nodePointerIters[3] = root->levelHash[3]->find(levelKeys[3])) != root->levelHash[3]->end() )
        {
            return GetvIndexNode((vIndexNode*)nodePointerIters[3]->second, &keyRev, 6, tag);
        }
        return GetvIndexNode((vIndexNode*)nodePointerIters[2]->second, &keyRev, 4, tag);
    }
    
    if(root->directPointers[levelKeys[1]])
    {
        return GetvIndexNode((vIndexNode*)root->directPointers[levelKeys[1]], (void*)(&keyRev), 2, tag);
    }

    return 0;
}

void RangevIndex(vIndex* root, KEY_TYPE key, int range, std::vector<VALUE_TYPE>& res)
{
    vIndexIterator vidxIter; vidxIter.level = 0;
    for(int i = 0; i < 4; i++) 
    {
        vidxIter.nodes[i] = NULL;
    }
    for(int i = 0; i < 8; i++) vidxIter.bytes[i] = ((key >> ((7 - i) << 3)) & 0xFF);
    vidxIter.levelKeys[0] = 0; vidxIter.levelKeys[1] = key >> 48; vidxIter.levelKeys[2] = key >> 32; vidxIter.levelKeys[3] = key >> 16; 
    HashTableRoot::iterator nodePointerIters[4];

    if(root->directPointers[vidxIter.levelKeys[1]])
    {
        vidxIter.level = 2; vidxIter.nodes[1] = (vIndexNode*)root->directPointers[vidxIter.levelKeys[1]]; vidxIter.offset = (vidxIter.levelKeys[2] & 0xFFFF);
    }

    if(!root->levelHash[2]->empty() && (nodePointerIters[2] = root->levelHash[2]->find(vidxIter.levelKeys[2])) != root->levelHash[2]->end())
    {
        vidxIter.level = 4; vidxIter.nodes[2] = (vIndexNode*)nodePointerIters[2]->second; vidxIter.offset = (vidxIter.levelKeys[3] & 0xFFFF);
    }
    if(!root->levelHash[3]->empty() &&(nodePointerIters[3] = root->levelHash[3]->find(vidxIter.levelKeys[3])) != root->levelHash[3]->end() )
    {
        vidxIter.level = 6; vidxIter.nodes[3] = (vIndexNode*)nodePointerIters[3]->second; 
    }
    if(vidxIter.level == 0) return;
    // printf("Range key = %ld, range = %d\n", key, range);
    RangevIndexNode(root, &vidxIter, &range, res);
}

#else
void InitvIndex(vIndex* root) //finish
{
    root->levelHash[2] = (HashTableRoot*)calloc(1, sizeof(HashTableRoot));
    root->levelHash[3] = (HashTableRoot*)calloc(1, sizeof(HashTableRoot));
    InitHashTable(root->levelHash[2], 16);
    InitHashTable(root->levelHash[3], 16);
}

void InsertvIndex(vIndex* root, KEY_TYPE key, VALUE_TYPE value) //finish
{
    uint64_t levelKeys[4] = {0};
    levelKeys[1] = key >> 48; levelKeys[2] = key >> 32; levelKeys[3] = key >> 16; 
    void** nodePointer[4] = {NULL};
    uint64_t keyRev = __builtin_bswap64(key);
    uint8_t tag = Hash64(key) >> 56;
    if(!Empty(root->levelHash[2]) && (nodePointer[2] = LookupHashTableReference(root->levelHash[2], levelKeys[2])) != NULL)
    {
        if(!Empty(root->levelHash[3]) && (nodePointer[3] = LookupHashTableReference(root->levelHash[3], levelKeys[3])) != NULL)
        {
            InsertvIndexNode(root, (vIndexNode*)(*nodePointer[3]), nodePointer[3], (void*)(&keyRev), value, 6, levelKeys, tag);
        }
        else
        {
            InsertvIndexNode(root, (vIndexNode*)(*nodePointer[2]), nodePointer[2], (void*)(&keyRev), value, 4, levelKeys, tag);
        }
    } 
    else
    {
        if(root->directPointers[levelKeys[1]])
        {
            InsertvIndexNode(root, (vIndexNode*)root->directPointers[levelKeys[1]], &root->directPointers[levelKeys[1]], (void*)(&keyRev), value, 2, levelKeys, tag);
        }
        else
        {
            CSRNode* csrNode = (CSRNode*)calloc(1, sizeof(CSRNode) + 4 * sizeof(void*));
            InsertCSRNode(root, csrNode, NULL, (void*)(&keyRev), value, 2, levelKeys, tag);
            root->directPointers[levelKeys[1]] = csrNode;
            root->bitmap65536[levelKeys[1] >> 6] |= (1ul << (levelKeys[1] & 0x3F));
            root->bitmap256[levelKeys[1] >> 14] |= (1ul << ((levelKeys[1] >> 8) & 0x3F));
        }
    }
}

// VALUE_TYPE GetvIndex(vIndex* root, KEY_TYPE key) //finish
// {

//     uint64_t levelKeys[4] = {0};
//     levelKeys[1] = key >> 48; levelKeys[2] = key >> 32; levelKeys[3] = key >> 16; 
//     void* nodePointer[4] = {NULL};
//     uint64_t hashCodes[4] = {0};
//     uint64_t keyRev = __builtin_bswap64(key);
//     uint8_t tag = Hash64(key) >> 56;
    
//     if(!Empty(root->levelHash[2]))
//     {
//         hashCodes[2] = Hash64(levelKeys[2]);
//         if(LookupHashTableOptimistic(root->levelHash[2], hashCodes[2]))
//         {
//             if(!Empty(root->levelHash[3]))
//             {
//                 hashCodes[3] = Hash64(levelKeys[3]);
//                 if((nodePointer[3] = LookupHashTable(root->levelHash[3], hashCodes[3], levelKeys[3])) != NULL)
//                 {
//                     return GetvIndexNode((vIndexNode*)nodePointer[3], &keyRev, 6, tag);
//                 }
//                 else if((nodePointer[2] = LookupHashTable(root->levelHash[2], hashCodes[2], levelKeys[2])) != NULL)
//                 {
//                     return GetvIndexNode((vIndexNode*)nodePointer[2], &keyRev, 4, tag);
//                 }
//             }
//             else if((nodePointer[2] = LookupHashTable(root->levelHash[2], hashCodes[2], levelKeys[2])) != NULL)
//             {
//                 return GetvIndexNode((vIndexNode*)nodePointer[2], &keyRev, 4, tag);
//             }
//         }
//     } 
//     if(root->directPointers[levelKeys[1]])
//     {
//         return GetvIndexNode((vIndexNode*)root->directPointers[levelKeys[1]], (void*)(&keyRev), 2, tag);
//     }
//     return 0;
// }

VALUE_TYPE GetvIndex(vIndex* root, KEY_TYPE key) //finish
{

    uint64_t levelKeys[4] = {0};
    levelKeys[1] = key >> 48; levelKeys[2] = key >> 32; levelKeys[3] = key >> 16; 
    void* nodePointer[4] = {NULL};
    uint64_t hashCodes[4] = {0};
    uint64_t keyRev = __builtin_bswap64(key);
    uint8_t tag = Hash64(key) >> 56;
    
    if(!Empty(root->levelHash[2]))
    {
        hashCodes[2] = Hash64(levelKeys[2]);
        if(LookupHashTableOptimistic(root->levelHash[2], hashCodes[2]))
        {
            if(!Empty(root->levelHash[3]))
            {
                hashCodes[3] = Hash64(levelKeys[3]);
                if((nodePointer[3] = LookupHashTable(root->levelHash[3], hashCodes[3], levelKeys[3])) != NULL)
                {
                    printf("3\n");
                    return GetvIndexNode((vIndexNode*)nodePointer[3], &keyRev, 6, tag);
                }
                else if((nodePointer[2] = LookupHashTable(root->levelHash[2], hashCodes[2], levelKeys[2])) != NULL)
                {
                    return GetvIndexNode((vIndexNode*)nodePointer[2], &keyRev, 4, tag);
                }
            }
            else if((nodePointer[2] = LookupHashTable(root->levelHash[2], hashCodes[2], levelKeys[2])) != NULL)
            {
                return GetvIndexNode((vIndexNode*)nodePointer[2], &keyRev, 4, tag);
            }
        }
    } 
    if(root->directPointers[levelKeys[1]])
    {
        return GetvIndexNode((vIndexNode*)root->directPointers[levelKeys[1]], (void*)(&keyRev), 2, tag);
    }
    return 0;
}


// VALUE_TYPE GetvIndex(vIndex* root, KEY_TYPE key) //finish
// {
//     uint64_t levelKeys[4] = {0};
//     levelKeys[1] = key >> 48; levelKeys[2] = key >> 32; levelKeys[3] = key >> 16; 
//     void* nodePointer[4] = {NULL};
//     uint64_t hashCodes[4] = {0};
//     uint64_t keyRev = __builtin_bswap64(key);
//     uint8_t tag = Hash64(key) >> 56;
//     if(!Empty(root->levelHash[2]))
//     {
//         hashCodes[2] = Hash64(levelKeys[2]);
//         if((nodePointer[2] = LookupHashTable(root->levelHash[2], hashCodes[2], levelKeys[2])) != NULL)
//         {

//             if(!Empty(root->levelHash[3]))
//             {
//                 hashCodes[3] = Hash64(levelKeys[3]);
//                 if((nodePointer[3] = LookupHashTable(root->levelHash[3], hashCodes[3], levelKeys[3])) != NULL)
//                 {
//                     return GetvIndexNode((vIndexNode*)nodePointer[3], &keyRev, 6, tag);
//                 }
//             }
//             return GetvIndexNode((vIndexNode*)nodePointer[2], &keyRev, 4, tag);
//         }
//     } 
//     if(root->directPointers[levelKeys[1]])
//     {
//         return GetvIndexNode((vIndexNode*)root->directPointers[levelKeys[1]], (void*)(&keyRev), 2, tag);
//     }
//     return 0;
// }
#endif