#include "wrapper.h"
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "vIndex.h"


void construct_container(void* container, KEY_TYPE* keyArray, VAL_TYPE* valArray, uint64_t numKeys)
{
    //std::sort(keyArray, keyArray + numKeys);
    for(uint64_t i = 0; i < numKeys; i++)
    {
        InsertvIndex((vIndex*)container, keyArray[i], valArray[i]);
    }
}

void* init_container(ExperimentConfig* pExpCfg)
{
    vIndex* container = (vIndex*)calloc(1, sizeof(vIndex));
    InitvIndex(container);
    return (void*)container;
}
RETURN_STATUS insert_KV(void *container, KEY_TYPE key, VAL_TYPE value)
{
    InsertvIndex((vIndex*)container, key, value);
    return RETURN_OK;
}
VAL_TYPE get_KV(void *container, KEY_TYPE key)
{
    return GetvIndex((vIndex*)container, key);
}



std::vector<VAL_TYPE> range_KV(void *container, KEY_TYPE key, int range)
{
#ifdef VINDEX
    std::vector<VAL_TYPE> res;
    RangevIndex((vIndex*)container, key, range, res);
    VAL_TYPE sum = 0;
    for(int i = 0; i < res.size(); i++) sum += res[i];
    return res;
#endif
}


void delete_KV(void *container, KEY_TYPE key)
{
    return;
}