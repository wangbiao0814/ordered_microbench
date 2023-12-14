#include "ASTree.h"
#include "CSR.h"

void InitASTree(ASTree* root)
{
    root->levelHash[2] = new HashTableRoot(256);
    root->levelHash[3] = new HashTableRoot(256);
}

void InsertASTree(ASTree* root, KEY_TYPE key, VALUE_TYPE value)
{
    uint64_t levelKeys[4] = {0};
    levelKeys[1] = key >> 48; levelKeys[2] = key >> 32; levelKeys[3] = key >> 16; 
    HashTableRoot::iterator nodePointerIters[4];
    uint64_t keyRev = __builtin_bswap64(key);
    if(!root->levelHash[2]->empty() && (nodePointerIters[2] = root->levelHash[2]->find(levelKeys[2])) != root->levelHash[2]->end())
    {
        if(!root->levelHash[3]->empty() && (nodePointerIters[3] = root->levelHash[3]->find(levelKeys[3])) != root->levelHash[3]->end())
        {
            InsertCSRNode(root, (CSRNode*)(nodePointerIters[3]->second), &nodePointerIters[3]->second, (void*)(&keyRev), value, 6, levelKeys);
        }
        else
        {
            InsertCSRNode(root, (CSRNode*)(nodePointerIters[2]->second), &nodePointerIters[2]->second, (void*)(&keyRev), value, 4, levelKeys);
        }
    } 
    else
    {
        if(root->directPointers[levelKeys[1]])
        {
            InsertCSRNode(root, (CSRNode*)root->directPointers[levelKeys[1]], &root->directPointers[levelKeys[1]], (void*)(&keyRev), value, 2, levelKeys);
        }
        else
        {
            CSRNode* csrNode = (CSRNode*)calloc(1, sizeof(CSRNode) + 4 * sizeof(void*));
            InsertCSRNode(root, csrNode, NULL, (void*)(&keyRev), value, 2, levelKeys);
            root->directPointers[levelKeys[1]] = csrNode;
            root->bitmap65536[levelKeys[1] >> 6] |= (1ul << (levelKeys[1] & 0x3F));
            root->bitmap256[levelKeys[1] >> 14] |= (1ul << ((levelKeys[1] >> 8) & 0x3F));
        }
    }
}
VALUE_TYPE GetASTree(ASTree* root, KEY_TYPE key)
{

    uint64_t levelKeys[4] = {0};
    levelKeys[1] = key >> 48; levelKeys[2] = key >> 32; levelKeys[3] = key >> 16; 
    HashTableRoot::iterator nodePointerIters[4];
    uint64_t hashCodes[4] = {0};
    uint64_t keyRev = __builtin_bswap64(key);


    if(!root->levelHash[2]->empty() && (nodePointerIters[2] = root->levelHash[2]->find(levelKeys[2])) != root->levelHash[2]->end())
    {
        if(!root->levelHash[3]->empty() && (nodePointerIters[3] = root->levelHash[3]->find(levelKeys[3])) != root->levelHash[3]->end() )
        {
            return GetCSRNode((CSRNode*)nodePointerIters[3]->second, &keyRev, 6);
        }
    
        return GetCSRNode((CSRNode*)nodePointerIters[2]->second, &keyRev, 4);
    }
    
    // else
    // {
        if(root->directPointers[levelKeys[1]])
        {
            return GetCSRNode((CSRNode*)root->directPointers[levelKeys[1]], (void*)(&keyRev), 2);
        }
    // }
    return 0;
}

