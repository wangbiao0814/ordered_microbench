#include "wrapper.h"
#include <cstdio>
#include "lib.h"
#include "kv.h"
#include "wh.h"

#ifdef MULTI_THREAD
void construct_container(void* container, KEY_TYPE* keyArray, VAL_TYPE* valArray, uint64_t numKeys)
{
    struct wormref * ref = NULL;
    ref = (struct wormref *)container;
    uint64_t x_rev; 
    for(uint64_t i = 0; i < numKeys; i++)
    {   
        x_rev = __builtin_bswap64(keyArray[i]);
        wh_put(ref, &x_rev, 8, &valArray[i], 8);  
    }
}

void* init_container(ExperimentConfig* pExpCfg)
{
    struct wormhole * const wh = wh_create();
    struct wormref * const container = whsafe_ref(wh);
    return (void*)container;
}
RETURN_STATUS insert_KV(void *container, KEY_TYPE key, VAL_TYPE value)
{ 
    struct wormref * ref = NULL; bool r;
    ref = (struct wormref *)container;
    KEY_TYPE x_rev = __builtin_bswap64(key);
    r = wh_put(ref, &x_rev, 8, &value, 8);  
    return RETURN_OK;
}
VAL_TYPE get_KV(void *container, KEY_TYPE key)
{
    struct wormref * ref = NULL; VAL_TYPE value; uint32_t value_len = 0; bool r;
    ref = (struct wormref *)container;
    KEY_TYPE x_rev = __builtin_bswap64(key);
    r = wh_get(ref, &x_rev, 8, &value, 8,  &value_len);
    if(r)
    {
        return value;  
    } 
    return 0;
}

VAL_TYPE range_KV(void* container, KEY_TYPE key)
{
    // struct wormref * ref = NULL; 
    // ref = (struct wormref *)container;
    // struct wormhole_iter * const iter = wh_iter_create(ref);
    // VAL_TYPE value; uint32_t value_len = 0; bool r;

    // KEY_TYPE x_rev = __builtin_bswap64(key);
    // struct kv * tmp = kv_create(&x_rev, 8, &value, 8);

    // wh_iter_seek(iter, tmp, 0); 
    // while (wh_iter_valid(iter)) {
    //     r = wh_iter_peek(iter, kbuf_out, 8, &klen_out, vbuf_out, 8, &vlen_out);
    //     if (r) {
    //     printf("wh_iter_peek klen=%u key=%.*s vlen=%u value=%.*s\n",
    //         klen_out, klen_out, kbuf_out, vlen_out, vlen_out, vbuf_out);
    //     } else {
    //     printf("ERROR!\n");
    //     }
    //     wh_iter_skip1(iter);
    // }
}

void delete_KV(void *container, KEY_TYPE key)
{

    return;
}
#else
void construct_container(void* container, KEY_TYPE* keyArray, VAL_TYPE* valArray, uint64_t numKeys)
{
    // struct wormhole* wh = (struct wormhole*)container;
    // uint64_t x_rev;
    // for(uint64_t i = 0; i < numKeys; i++)
    // {   
    //     struct kv *tmp = NULL; uint64_t x_rev;
    //     x_rev = __builtin_bswap64(keyArray[i]);
    //     tmp = kv_create(&x_rev, 8, &valArray[i], 8);
    //     whunsafe_put(wh, tmp);    
    // }
    struct wormhole* wh = (struct wormhole*)container;
    uint64_t x_rev; char keyValuePair[sizeof(struct kv) + 16];
    for(uint64_t i = 0; i < numKeys; i++)
    {   
        struct kv *tmp = (struct kv *)keyValuePair; uint64_t x_rev;
        x_rev = __builtin_bswap64(keyArray[i]);
        tmp->klen = 8; tmp->vlen = 8;
        memcpy(tmp->kv, &(x_rev), 8);
		memcpy(tmp->kv + 8, &(valArray[i]), 8);
        kv_update_hash(tmp);
        whunsafe_put(wh, tmp);    
    }
     
}

void* init_container(ExperimentConfig* pExpCfg)
{
    struct wormhole* container = whunsafe_create(NULL);
    return (void*)container;
}
RETURN_STATUS insert_KV(void *container, KEY_TYPE key, VAL_TYPE value)
{
    struct wormhole* wh = (struct wormhole*)container;
    uint64_t x_rev; char keyValuePair[sizeof(struct kv) + 16];
    x_rev = __builtin_bswap64(key);
    struct kv *tmp = (struct kv*)keyValuePair;
    tmp->klen = 8; tmp->vlen = 8;
    memcpy(tmp->kv, &(x_rev), 8);
    memcpy(tmp->kv + 8, &(value), 8);
    kv_update_hash(tmp);
    whunsafe_put(wh, tmp);    
    return RETURN_OK;
}
VAL_TYPE get_KV(void *container, KEY_TYPE key)
{
    struct wormhole* wh = (struct wormhole*)container;
    struct kv *tmp = NULL; uint64_t x_rev; struct kref kref;  
    char keyValuePair[sizeof(struct kv) + 16];
    x_rev = __builtin_bswap64(key);
    tmp = (struct kv*)keyValuePair; tmp->klen = 8; tmp->vlen = 8;
    memcpy(tmp->kv, &(x_rev), 8);
    kref = kv_kref(tmp);
    kv_update_hash(tmp);
    whunsafe_get(wh, &kref, tmp);
    return __builtin_bswap64(*(uint64_t*)(tmp->kv));
}

std::vector<VAL_TYPE> range_KV(void* container, KEY_TYPE key, int range)
{
    VAL_TYPE sum = 0; std::vector<VAL_TYPE> res;
    struct wormhole* wh = (struct wormhole*)container;
    struct wormhole_iter * iter = whunsafe_iter_create(wh);
    uint64_t x_rev; char keyValuePair[sizeof(struct kv) + 16];
    x_rev = __builtin_bswap64(key);
    struct kv *tmp = (struct kv*)keyValuePair; struct kref kref;  
    tmp->klen = 8; tmp->vlen = 8;
    memcpy(tmp->kv, &(x_rev), 8);
    kref = kv_kref(tmp);
    kv_update_hash(tmp);
    whunsafe_iter_seek(iter, &kref);
    char keyValuePairResult[sizeof(struct kv) + 16];
    struct kv *iteration_result = (struct kv*)keyValuePairResult;
    for (int j = 0; j < range; j++)
    {
        whunsafe_iter_next(iter, iteration_result);
        sum += __builtin_bswap64(*(uint64_t*)(iteration_result->kv));
        res.push_back(__builtin_bswap64(*(uint64_t*)(iteration_result->kv)));
    }
    return res;
}

void delete_KV(void *container, KEY_TYPE key)
{

    return;
}

#endif