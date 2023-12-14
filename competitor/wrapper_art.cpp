#include "wrapper.h"
#include <cstdio>
#include <cstdlib>
#include "art.h"


void construct_container(void* container, KEY_TYPE* keyArray, VAL_TYPE* valArray, uint64_t numKeys)
{
    uint64_t x_rev;
    art_tree* art = (art_tree*)container;
    for(uint64_t i = 0; i < numKeys; i++)
    {
        x_rev = __builtin_bswap64(keyArray[i]);
        art_insert(art, (const unsigned char*)&x_rev, 8, (void*)valArray[i]);
    }
}

void* init_container(ExperimentConfig* pExpCfg)
{
    void *container = malloc(sizeof(art_tree));
    art_tree_init((art_tree*)container); 
    return container;
}
RETURN_STATUS insert_KV(void *container, KEY_TYPE key, VAL_TYPE value)
{
    uint64_t x_rev;
    x_rev = __builtin_bswap64(key);
    art_insert((art_tree*)container, (const unsigned char*)&x_rev, 8, (void*)value);
    return RETURN_OK;
}
VAL_TYPE get_KV(void *container, KEY_TYPE key)
{
    uint64_t x_rev, p_ret;
    x_rev = __builtin_bswap64(key);
    p_ret = (uint64_t)art_search((art_tree*)container, (const unsigned char*)&x_rev, 8);
    return p_ret;
}

void delete_KV(void *container, KEY_TYPE key)
{
    return;
}