#include "wrapper.h"
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "ASTree.h"


void construct_container(void* container, KEY_TYPE* keyArray, VAL_TYPE* valArray, uint64_t numKeys)
{
    // std::sort(keyArray, keyArray + numKeys);
    for(uint64_t i = 0; i < numKeys; i++)
    {
        InsertASTree((ASTree*)container, keyArray[i], valArray[i]);
    }
}

void* init_container(ExperimentConfig* pExpCfg)
{
    ASTree* container = (ASTree*)calloc(1, sizeof(ASTree));
    InitASTree(container);
    return (void*)container;
}
RETURN_STATUS insert_KV(void *container, KEY_TYPE key, VAL_TYPE value)
{
    InsertASTree((ASTree*)container, key, value);
    return RETURN_OK;
}
VAL_TYPE get_KV(void *container, KEY_TYPE key)
{
    return GetASTree((ASTree*)container, key);
}

void delete_KV(void *container, KEY_TYPE key)
{
    return;
}