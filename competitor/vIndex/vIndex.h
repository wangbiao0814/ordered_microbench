#ifndef _VINDEX_H_
#define _VINDEX_H_

#include <cstdint>
#include <vector>

#define USE_FLATHASH

#ifdef USE_FLATHASH
#include <gtl/phmap.hpp>
typedef gtl::flat_hash_map<uint64_t, void*> HashTableRoot;
inline uint64_t Hash64(uint64_t h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    return h;
}
#else
#include "Hashtable.h"
#endif


#include "Define.h"

typedef struct 
{
    HashTableRoot* levelHash[4];
    uint64_t bitmap256[4];
    uint64_t bitmap65536[1 << 10];
    void* directPointers[1 << 16];
}vIndex;




void InitvIndex(vIndex* root);
void InsertvIndex(vIndex* root, KEY_TYPE key, VALUE_TYPE value);
VALUE_TYPE GetvIndex(vIndex* root, KEY_TYPE key);
void RangevIndex(vIndex* root, KEY_TYPE key, int range, std::vector<VALUE_TYPE>& res);
#endif