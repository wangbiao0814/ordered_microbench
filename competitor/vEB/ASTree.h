#ifndef _ASTREE_H_
#define _ASTREE_H_

#include <cstdint>



#include <gtl/phmap.hpp>
typedef gtl::flat_hash_map<uint64_t, void*> HashTableRoot;


#define KEY_TYPE uint64_t
#define VALUE_TYPE uint64_t

#define KEY_BYTES (sizeof(KEY_TYPE))
#define VALUE_BYTES (sizeof(VALUE_TYPE))




typedef struct 
{
    HashTableRoot* levelHash[4];
    uint64_t bitmap256[4];
    uint64_t bitmap65536[1 << 10];
    void* directPointers[1 << 16];
}ASTree;


void InitASTree(ASTree* root);
void InsertASTree(ASTree* root, KEY_TYPE key, VALUE_TYPE value);
VALUE_TYPE GetASTree(ASTree* root, KEY_TYPE key);

#endif