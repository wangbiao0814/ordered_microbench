#include "Hashtable.h"

static inline uint8_t ctz_16(uint16_t x)
{
    uint8_t n = 1;
    if((x & 0xFF) == 0) {n += 8; x >>= 8;}
    if((x & 0x0F) == 0) {n += 4; x >>= 4;}
    if((x & 0x03) == 0) {n += 2; x >>= 2;}
    return n - (x & 1);
}

void InitHashTable(HashTableRoot* root, size_t capacity)
{
    if((capacity & (capacity - 1)) != 0)
    {
        printf("Capacity must be the power of 2!\n");
        return;
    }
    root->hashTable = (HashTable*)calloc(1, sizeof(HashTable) + capacity / BUCKET_CAPACITY * sizeof(HashBucket));
    root->hashTable->capacity = capacity;
    root->hashTable->keyCount = 0;
}

void ReleaseHashTable(HashTable* hashTable)
{
    free(hashTable);
}

void InsertHashTable(HashTableRoot* root, KEY_TYPE key, void* nodePointer)
{
    uint64_t hashCode = Hash64(key);
    uint8_t tag = (hashCode >> 56);
    uint64_t idxBucket = (hashCode & (root->hashTable->capacity / BUCKET_CAPACITY - 1));
    HashBucket* tmpBucket = &(root->hashTable->buckets[idxBucket]); 
    __m128i cmp; uint16_t bitfield; uint8_t idx;
    cmp = _mm_cmpeq_epi8(_mm_set1_epi8(tag), _mm_loadu_si128((__m128i*)(tmpBucket->fingerprints)));
    bitfield = ((uint16_t)_mm_movemask_epi8(cmp));
    bitfield &= (tmpBucket->count == 0 ? 0 : ((0xFFFF) >> (16 - tmpBucket->count)));
    while(bitfield)
    {
        idx = ctz_16(bitfield);
        if(tmpBucket->elements[idx].key == key)
        {
            tmpBucket->elements[idx].nodePointer = nodePointer;
            return;
        }
        bitfield ^= (0x1ul << idx);
    }
    while(root->hashTable->buckets[idxBucket].count == BUCKET_CAPACITY)
    {
        ResizeHashTable(root, root->hashTable->capacity * 2);
        idxBucket = (hashCode & (root->hashTable->capacity / BUCKET_CAPACITY - 1));
    }
    tmpBucket = &(root->hashTable->buckets[idxBucket]);
    tmpBucket->fingerprints[tmpBucket->count] = tag;
    tmpBucket->elements[tmpBucket->count].key = key;
    tmpBucket->elements[tmpBucket->count].nodePointer = nodePointer;
    tmpBucket->count++;
    root->hashTable->keyCount++;
}


void ResizeHashTable(HashTableRoot* root, size_t newCapacity)
{
    HashTable* newHashTable = (HashTable*)calloc(1, sizeof(HashTable) + newCapacity / BUCKET_CAPACITY * sizeof(HashBucket));
    HashTable* oldHashTable = root->hashTable;
    for(int i = 0; i < oldHashTable->capacity / BUCKET_CAPACITY; i++)
    {
        HashBucket* tmpBucket = &oldHashTable->buckets[i];
        for(int j = 0; j < tmpBucket->count; j++)
        {
            
            uint8_t tag = tmpBucket->fingerprints[j];
            uint64_t key = tmpBucket->elements[j].key;
            void* nodePointer = tmpBucket->elements[j].nodePointer;
            uint64_t hashCode = Hash64(key);

            HashBucket* newBucket = &newHashTable->buckets[hashCode & (newCapacity / BUCKET_CAPACITY - 1)];
            newBucket->fingerprints[newBucket->count] = tag;
            newBucket->elements[newBucket->count].key = tmpBucket->elements[j].key;
            newBucket->elements[newBucket->count].nodePointer = nodePointer;
            newBucket->count++;
        }
        
    }
    newHashTable->capacity = newCapacity;
    newHashTable->keyCount = root->hashTable->keyCount;
    root->hashTable = newHashTable;
    ReleaseHashTable(oldHashTable);
}

void** LookupHashTableReference(HashTableRoot* root, KEY_TYPE key)
{
    uint64_t hashCode = Hash64(key);
    uint8_t tag = (hashCode >> 56);
    uint64_t idxBucket = (hashCode & (root->hashTable->capacity / BUCKET_CAPACITY - 1));
    HashBucket* tmpBucket = &(root->hashTable->buckets[idxBucket]); 

    __m128i cmp; uint16_t bitfield; uint8_t idx;
    cmp = _mm_cmpeq_epi8(_mm_set1_epi8(tag), _mm_loadu_si128((__m128i*)(tmpBucket->fingerprints)));
    bitfield = ((uint16_t)_mm_movemask_epi8(cmp));
    bitfield &= (tmpBucket->count == 0 ? 0 : ((0xFFFF) >> (16 - tmpBucket->count)));
    while(bitfield)
    {
        idx = ctz_16(bitfield);
        if(tmpBucket->elements[idx].key == key)
        {
            return &tmpBucket->elements[idx].nodePointer;
        }
        bitfield ^= (0x1ul << idx);
    }
    return NULL;
}


bool LookupHashTableOptimistic(HashTableRoot* root, uint64_t hashCode)
{
    uint8_t tag = (hashCode >> 56);
    uint64_t idxBucket = (hashCode & (root->hashTable->capacity / BUCKET_CAPACITY - 1));
    HashBucket* tmpBucket = &(root->hashTable->buckets[idxBucket]); 

    __m128i cmp; uint16_t bitfield; uint8_t idx;
    cmp = _mm_cmpeq_epi8(_mm_set1_epi8(tag), _mm_loadu_si128((__m128i*)(tmpBucket->fingerprints)));
    bitfield = ((uint16_t)_mm_movemask_epi8(cmp));
    bitfield &= (tmpBucket->count == 0 ? 0 : ((0xFFFF) >> (16 - tmpBucket->count)));
    if(bitfield) return true;
    return false;
}


void* LookupHashTable(HashTableRoot* root, uint64_t hashCode, KEY_TYPE key)
{
    uint8_t tag = (hashCode >> 56);
    uint64_t idxBucket = (hashCode & (root->hashTable->capacity / BUCKET_CAPACITY - 1));
    HashBucket* tmpBucket = &(root->hashTable->buckets[idxBucket]); 

    __m128i cmp; uint16_t bitfield; uint8_t idx;
    cmp = _mm_cmpeq_epi8(_mm_set1_epi8(tag), _mm_loadu_si128((__m128i*)(tmpBucket->fingerprints)));
    bitfield = ((uint16_t)_mm_movemask_epi8(cmp));
    bitfield &= (tmpBucket->count == 0 ? 0 : ((0xFFFF) >> (16 - tmpBucket->count)));
    while(bitfield)
    {
        idx = ctz_16(bitfield);
        if(tmpBucket->elements[idx].key == key)
        {
            return tmpBucket->elements[idx].nodePointer;
        }
        bitfield ^= (0x1ul << idx);
    }
    return NULL;
}
