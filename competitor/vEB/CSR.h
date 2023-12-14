#include "ASTree.h"
#include "util.h"
#include <cstdint>

typedef uint64_t KEY_TYPE;
typedef uint64_t VALUE_TYPE;
// typedef uint8_t VALUE_TYPE;


#define KEY_BYTES (sizeof(KEY_TYPE))
#define VALUE_BYTES (sizeof(VALUE_TYPE))

#define CSR_CHILD_INIT_SIZE 4
#define BLOCKS 4

typedef enum{
    CSR_NODE = 0,
    BITMAP_LEAF_NODE,
}NODE_TYPE;

typedef struct 
{
    uint32_t type;
    uint32_t padding;
}BaseNode;

typedef struct 
{
    uint32_t type; // NodeType
    uint8_t rank[4]; 
    uint64_t bitmap[4]; //Bitmap256 mark for CSRNode/BitmapLeafNode
    // uint8_t sum[16];
    // uint8_t counters[64]; //Counters and cumsum for CSR
    uint8_t csrRank[4];
    uint64_t csrBitmap[4];

    void* CSR[BLOCKS]; //CSR elements
    // uint16_t locks[4];
    void* nodes[]; // Node pointers for CSRNode/BitmapLeafNode
}CSRNode;

void SetCSRNodeBitmap(CSRNode* csrNode, uint8_t key);
bool GetCSRNodeBitmap(CSRNode* csrNode, uint8_t key);
int GetCSRNodeBitmapPopcnt(CSRNode* csrNode, uint8_t key);
int GetCSRNodeBitmapPopcntAll(CSRNode* csrNode);
int GetCSRNodeCount(CSRNode* csrNode, uint8_t key);
int GetCSRNodeCumsum(CSRNode* csrNode, uint8_t key);
int GetCSRNodeTotalCount(CSRNode* csrNode, uint8_t blockIndex);
void AddCSRNodeCount(CSRNode* csrNode, uint8_t key);

typedef struct 
{
    uint32_t type; // NodeType
    uint8_t rank[4]; 
    uint64_t bitmap[4]; // Bitmap256 mark for keys
    VALUE_TYPE values[]; // Values
}BitmapLeafNode;

void SetBitmapLeafNodeBitmap(BitmapLeafNode* bitmapLeafNode, uint8_t key);
bool GetBitmapLeafNodeBitmap(BitmapLeafNode* bitmapLeafNode, uint8_t key);
int GetBitmapLeafNodeBitmapPopcnt(BitmapLeafNode* bitmapLeafNode, uint8_t key);
int GetBitmapLeafNodeBitmapPopcntAll(BitmapLeafNode* bitmapLeafNode);


void InsertCSRNode(ASTree* root, CSRNode* node, void** p_node, void* key, VALUE_TYPE value, int level, uint64_t* levelKeys);
VALUE_TYPE GetCSRNode(CSRNode* node, void* key, int level);

void InsertBitmapLeafNode(BitmapLeafNode* node, void** p_node, void* key, VALUE_TYPE value, int level);
VALUE_TYPE GetBitmapLeafNode(BitmapLeafNode* node, void* key, int level);


inline void SetCSRNodeBitmap(CSRNode* csrNode, uint8_t key)
{
    csrNode->bitmap[key >> 6] |= (1ul << key);
    csrNode->rank[key >> 6]++;
}

inline bool GetCSRNodeBitmap(CSRNode* csrNode, uint8_t key)
{
    return (csrNode->bitmap[key >> 6] & (1ul << key));
}

inline int GetCSRNodeBitmapPopcnt(CSRNode* csrNode, uint8_t key)
{
    int res = 0;
    for(int i = 0; i < (key >> 6); i++)
    {
        res += csrNode->rank[i];
    }
    return res + __builtin_popcountll(csrNode->bitmap[key >> 6] & (((uint64_t)2 << key) - 1));
}
inline int GetCSRNodeBitmapPopcntAll(CSRNode* csrNode)
{
    return csrNode->rank[0] + csrNode->rank[1] + csrNode->rank[2] + csrNode->rank[3];
}

inline int GetCSRNodeCount(CSRNode* csrNode, uint8_t key)
{
    return ((csrNode->csrBitmap[key >> 6] & (1ul << key)) != 0);
}
inline int GetCSRNodeCumsum(CSRNode* csrNode, uint8_t key)
{
    return __builtin_popcountll(csrNode->csrBitmap[key >> 6] & (((uint64_t)1 << key) - 1));
}

inline void SetBitmapLeafNodeBitmap(BitmapLeafNode* bitmapLeafNode, uint8_t key)
{
    bitmapLeafNode->bitmap[key >> 6] |= (1ul << key);
    bitmapLeafNode->rank[key >> 6]++;
}

inline bool GetBitmapLeafNodeBitmap(BitmapLeafNode* bitmapLeafNode, uint8_t key)
{
    return (bitmapLeafNode->bitmap[key >> 6] & (1ul << key));
}

inline int GetBitmapLeafNodeBitmapPopcnt(BitmapLeafNode* bitmapLeafNode, uint8_t key)
{
    int res = 0;
    for(int i = 0; i < (key >> 6); i++)
    {
        res += bitmapLeafNode->rank[i];
    }
    return res + __builtin_popcountll(bitmapLeafNode->bitmap[key >> 6] & (((uint64_t)2 << key) - 1));
}

inline int GetBitmapLeafNodeBitmapPopcntAll(BitmapLeafNode* bitmapLeafNode)
{
    return bitmapLeafNode->rank[0] + bitmapLeafNode->rank[1] + bitmapLeafNode->rank[2] + bitmapLeafNode->rank[3];
}

inline int GetCSRNodeTotalCount(CSRNode* csrNode, uint8_t blockIndex)
{
    return csrNode->csrRank[blockIndex];
}

inline void AddCSRNodeCount(CSRNode* csrNode, uint8_t key)
{
    csrNode->csrBitmap[key >> 6] ^= (1ul << key);
    csrNode->csrRank[key >> 6]++;
}