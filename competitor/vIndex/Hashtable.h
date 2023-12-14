#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

extern "C"
{
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <emmintrin.h>
#include <string.h>
#include <stdbool.h>
}

// #include "xxhash.h"
// #include "gtl/phmap_utils.hpp"

// #define XXHASH_SEED 0

#include "Define.h"

#define BUCKET_CAPACITY 16

typedef struct 
{
    uint64_t key; //Original key
    void* nodePointer; //Pointer to each node
}HashElement;

typedef struct HashBucketType
{
    uint64_t count; //The number of keys in a bucket
    uint8_t fingerprints[BUCKET_CAPACITY]; // Fingerprints array for searching
    HashElement elements[BUCKET_CAPACITY]; // The key-node pairs;
}HashBucket;

typedef struct 
{
    size_t keyCount; //number of keys in the hash table
    size_t capacity; //number of keys allocated in the hash table
    HashBucket buckets[];  // the bucket for keys
}HashTable;

typedef struct 
{
    HashTable* hashTable;
}HashTableRoot;

// from code.google.com/p/smhasher/wiki/MurmurHash3
inline uint64_t Hash64(uint64_t h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    return h;
}
// inline uint64_t Hash64(uint64_t h)
// {
//     return gtl::HashState().combine(0, h);
// }

inline uint64_t Dehash64(uint64_t h) {
    h ^= h >> 33;
    h *= 0x9cb4b2f8129337db;
    h ^= h >> 33;
    h *= 0x4f74430c22a54005;
    h ^= h >> 33;
    return h;
}

// inline uint64_t GetHashCode(uint64_t key, uint16_t keyLength)
// {
//     key = __builtin_bswap64(key)
//     return XXH64((void*)&key, keyLength, XXHASH_SEED);
// }

void InitHashTable(HashTableRoot* root, size_t keyCount);

void ReleaseHashTable(HashTable* hashTable);

void ResizeHashTable(HashTableRoot* root, size_t newCapacity);

void InsertHashTable(HashTableRoot* root, KEY_TYPE key, void* nodePointer);

void** LookupHashTableReference(HashTableRoot* root, KEY_TYPE key);

bool LookupHashTableOptimistic(HashTableRoot* root, uint64_t hashCode);

void* LookupHashTable(HashTableRoot* root, uint64_t hashCode, KEY_TYPE key);

inline bool Empty(HashTableRoot* root)
{
    return root->hashTable->keyCount == 0;
}

#endif