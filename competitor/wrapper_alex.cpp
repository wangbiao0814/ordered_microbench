#include "wrapper.h"
#include <cstdio>
#include <cstdlib>
#include "core/alex_map.h"


void construct_container(void* container, KEY_TYPE* keyArray, VAL_TYPE* valArray, uint64_t numKeys)
{
    alex::AlexMap<KEY_TYPE, VAL_TYPE>* index = (alex::AlexMap<KEY_TYPE, VAL_TYPE>*)container;
    for(uint64_t i = 0; i < numKeys; i++)
    {
        index->insert(keyArray[i], valArray[i]);
    }
}

void* init_container(ExperimentConfig* pExpCfg)
{
    alex::AlexMap<KEY_TYPE, VAL_TYPE>* container = new alex::AlexMap<KEY_TYPE, VAL_TYPE>();
    return container;
}
RETURN_STATUS insert_KV(void *container, KEY_TYPE key, VAL_TYPE value)
{
    alex::AlexMap<KEY_TYPE, VAL_TYPE>* index = (alex::AlexMap<KEY_TYPE, VAL_TYPE>*)container;
    index->insert(key, value);
    return RETURN_OK;
}
VAL_TYPE get_KV(void *container, KEY_TYPE key)
{
    alex::AlexMap<KEY_TYPE, VAL_TYPE>* index = (alex::AlexMap<KEY_TYPE, VAL_TYPE>*)container;
    auto it = index->find(key);
    if (it != index->end()) {
        return it.key();
    }
    return 0;
}

void delete_KV(void *container, KEY_TYPE key)
{
    return;
}