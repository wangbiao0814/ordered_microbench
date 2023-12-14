#include "wrapper.h"
#include <libcuckoo/cuckoohash_map.hh>

inline uint64_t Hash64(uint64_t h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    return h;
}

void construct_container(void* container, KEY_TYPE* keyArray, VAL_TYPE* valArray, uint64_t numKeys)
{
    libcuckoo::cuckoohash_map<KEY_TYPE, VAL_TYPE>* hashmap = (libcuckoo::cuckoohash_map<KEY_TYPE, VAL_TYPE>*)container;
    for(uint64_t i = 0; i < numKeys; i++)
    {
        hashmap->insert(Hash64(keyArray[i]), valArray[i]);
    }
}

void* init_container(ExperimentConfig* pExpCfg)
{
    libcuckoo::cuckoohash_map<KEY_TYPE, VAL_TYPE>* container = new libcuckoo::cuckoohash_map<KEY_TYPE, VAL_TYPE>(pExpCfg->numKeys);
    // container->reserve(pExpCfg->numWrites);
    return (void*)container;
}
RETURN_STATUS insert_KV(void *container, KEY_TYPE key, VAL_TYPE value)
{
    libcuckoo::cuckoohash_map<KEY_TYPE, VAL_TYPE>* hashmap = (libcuckoo::cuckoohash_map<KEY_TYPE, VAL_TYPE>*)container;
    hashmap->insert(Hash64(key), value);
    return RETURN_OK;
}
VAL_TYPE get_KV(void *container, KEY_TYPE key)
{
    libcuckoo::cuckoohash_map<KEY_TYPE, VAL_TYPE>* hashmap = (libcuckoo::cuckoohash_map<KEY_TYPE, VAL_TYPE>*)container;
    VAL_TYPE res;
    hashmap->find(Hash64(key), res);
    return res;
}

void delete_KV(void *container, KEY_TYPE key)
{
    return;
}



