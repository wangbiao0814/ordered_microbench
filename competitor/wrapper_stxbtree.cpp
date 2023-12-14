#include "wrapper.h"
#include <cstdio>
#include "btree_map.h"
#include "btree.h"
typedef stx::btree_map<uint64_t, uint64_t> MapType;


void construct_container(void* container, KEY_TYPE* keyArray, VAL_TYPE* valArray, uint64_t numKeys)
{
    MapType* idx = (MapType*)container;
    for(uint64_t i = 0; i < numKeys; i++)
    {
        (*idx)[keyArray[i]] = valArray[i];
    }
}

void* init_container(ExperimentConfig* pExpCfg)
{
    MapType* container = new MapType();
    return (void*)container;
}
RETURN_STATUS insert_KV(void *container, KEY_TYPE key, VAL_TYPE value)
{
    MapType* idx = (MapType*)container;
    (*idx)[key] = value;
    return RETURN_OK;
}
VAL_TYPE get_KV(void *container, KEY_TYPE key)
{
    MapType* idx = (MapType*)container;
    MapType::const_iterator iter = idx->find(key);
    if(iter != idx->end())
    {
        return iter->second;
    }
    return 0;
}

std::vector<VAL_TYPE> range_KV(void *container, KEY_TYPE key, int range)
{
    std::vector<VAL_TYPE> res;
    MapType* idx = (MapType*)container;
    auto iter = idx->lower_bound(key);
    uint64_t sum = 0;
    for (int i = 0; i < range; i++) {
      if (iter == idx->end()) {
	      break;
      }
      sum += iter->second;
      res.push_back(iter->second);
      ++iter;
    }
    return res;
}

void delete_KV(void *container, KEY_TYPE key)
{
    return;
}