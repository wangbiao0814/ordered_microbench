#ifndef _WRAPPER_H_
#define _WRAPPER_H_

#include <stdint.h>



#ifdef U64_KEY_U64_VALUE
typedef uint64_t KEY_TYPE;
typedef uint64_t VAL_TYPE;
#endif

typedef enum {
    OP_WRITE,
    OP_READ,
    OP_DELETE,
    OP_SCAN,
}OP_CODE;


#define MAX_NUM_THREADS 32

#define CONST_MILLION 1000000

typedef struct 
{
    uint64_t numKeys;
    uint64_t numTxns;
    uint16_t numTxnThread;
    uint64_t experimentID;
    char initFileName[256];
    char txnFileName[256];                             
}ExperimentConfig;


typedef struct 
{
    int threadID;
    void* container;
    KEY_TYPE* keyArray;
    VAL_TYPE* valArray;
    int* rangeArray;
    OP_CODE* opArray;
    uint64_t opCount;
}ThreadArgs;




typedef enum {
    RETURN_OK,
    RETURN_ERR,
}RETURN_STATUS;

void* init_container(ExperimentConfig* pExpCfg);

void construct_container(void* container, KEY_TYPE* keyArray, VAL_TYPE* valueArray, uint64_t numKeys);

RETURN_STATUS insert_KV(void *container, KEY_TYPE key, VAL_TYPE value);
VAL_TYPE get_KV(void *container, KEY_TYPE key);
void delete_KV(void *container, KEY_TYPE key);

uint64_t range_KV(void *container, KEY_TYPE key, int range);

#endif