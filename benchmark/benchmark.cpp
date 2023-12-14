
extern "C"
{
 #include <malloc.h>  
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

void malloc_stats(void);

}
#include <random>
#include <algorithm>
#include "selfsimilar_int_distribution.h"
#include <unordered_map>
#include "benchmark.h"
#include "dataloader.h"

#ifdef PAPI_CACHE
#include "papi.h"
#endif


size_t GetMemoryUsage()
{
    FILE *fp = fopen("/proc/self/statm", "r");
    if(fp == NULL)
    {
        fprintf(stderr, "Could not open /proc/self/statm to read memory useage\n");
        exit(1);
    }
    uint64_t unused, rss;
    if(fscanf(fp, "%ld %ld %ld %ld %ld %ld %ld", &unused, &rss, &unused, &unused, &unused, &unused, &unused) != 7)
    {
        exit(1);
    }
    fclose(fp);

    return rss * (4096 / 1024);
}



pthread_barrier_t g_barrier;
pthread_barrier_t g_write_barrier;
pthread_barrier_t g_read_barrier;

void BindToCore(uint32_t coreID)
{
    uint32_t nCores = sysconf(_SC_NPROCESSORS_ONLN);
    assert(coreID < nCores);
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(coreID, &mask);
    int rc = sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    if(rc != 0)
    {
        printf("Failed to sched_setaffinity %d\n", rc);
    }
    else 
    {
        // printf("Bind to core %u\n", coreID);
    }
}

void* ExecuteWorkload(void *pThreadArgs)
{
    ThreadArgs* pThdArgs = (ThreadArgs*)pThreadArgs;
    int threadID = pThdArgs->threadID;
    BindToCore(threadID);

    uint64_t opCount = pThdArgs->opCount;
    KEY_TYPE* keyArray = pThdArgs->keyArray;
    VAL_TYPE* valArray = pThdArgs->valArray;
    int* rangeArray = pThdArgs->rangeArray;
    OP_CODE* opArray = pThdArgs->opArray;

#ifdef PAPI_CACHE
    int EventSet;
    long_long values[5], values1[5], values2[5];
    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT)
        exit(-1);
    EventSet = PAPI_NULL;
    if (PAPI_create_eventset(&EventSet) != PAPI_OK)
        exit(-1);

    // if (PAPI_add_event(EventSet, PAPI_L1_TCM) != PAPI_OK)
    //     exit(-1);
    // if (PAPI_add_event(EventSet, PAPI_L2_TCM) != PAPI_OK)
    //     exit(-1);
    // if (PAPI_add_event(EventSet, PAPI_L3_TCM) != PAPI_OK)
    //     exit(-1);
    if (PAPI_add_event(EventSet, PAPI_TOT_CYC) != PAPI_OK)
    {
        printf("Unable PAPI_TOT_CYC\n");
        exit(-1);
    }
    if (PAPI_add_event(EventSet, PAPI_BR_MSP) != PAPI_OK)
    {
        printf("Unable PAPI_BR_MSP\n");
        exit(-1);
    }
    if (PAPI_add_event(EventSet, PAPI_TOT_INS) != PAPI_OK)
    {
        printf("Unable PAPI_TOT_INS\n");
        exit(-1);
    }
    /* Start counting events */

    if (PAPI_start(EventSet) != PAPI_OK)
    {
        printf("Unable to start event\n");
        exit(-1);
    }
    /* Read counters before workload running*/
    if (PAPI_read(EventSet, values1) != PAPI_OK)
        exit(-1);
#endif

    uint64_t res = 0, i; std::vector<VAL_TYPE> results;
    for(i = 0; i < opCount; i++)
    {
        switch (opArray[i])
        {
        case OP_WRITE:
            insert_KV(pThdArgs->container, keyArray[i], keyArray[i]);
            break;
        case OP_READ:
            if(get_KV(pThdArgs->container, keyArray[i]) != keyArray[i])
            {
                // printf("error when key = %lx, result = %lx\n", keyArray[i], get_KV(pThdArgs->container, keyArray[i]));
                // return NULL;
            }
            break;
        case OP_DELETE:
            delete_KV(pThdArgs->container, keyArray[i]);
            break;
        case OP_SCAN:
#ifdef STXBTREE
            range_KV(pThdArgs->container, valArray[keyArray[i]], rangeArray[i]);
#endif
#ifdef VINDEX
            range_KV(pThdArgs->container, valArray[keyArray[i]], rangeArray[i]);
#endif
#ifdef WORMHOLE
            range_KV(pThdArgs->container, valArray[keyArray[i]], rangeArray[i]);
#endif
#ifdef ART
            for(int j = keyArray[i]; j < keyArray[i] + rangeArray[i]; j++)
            {
                // if(get_KV(pThdArgs->container, valArray[j]) != valArray[j])
                // {
                //     // printf("error\n");
                //     // return NULL;
                // }
                results.push_back(get_KV(pThdArgs->container, valArray[j]));
            }
#endif

            break;
        default:
            break;
        }

    }

#ifdef PAPI_CACHE
    /* Clean up EventSet */
    if (PAPI_stop(EventSet, values2) != PAPI_OK)
        exit(-1);
    printf("L1 miss = %ld\n", values2[0] - values1[0]);
    printf("L2 miss = %ld\n", values2[1] - values1[1]);
    printf("L3 miss = %ld\n", values2[2] - values1[2]);
    printf("Total cycles = %ld\n", values2[3] - values1[3]);
    printf("Mispredict branches = %ld\n", values2[4] - values1[4]);
    printf("Total instructions = %ld\n", values2[5] - values1[5]);
    
    if (PAPI_cleanup_eventset(EventSet) != PAPI_OK)

        exit(-1);

    /* Destroy the EventSet */

    if (PAPI_destroy_eventset(&EventSet) != PAPI_OK)

        exit(-1);

    /* Shutdown PAPI */

    PAPI_shutdown();

#endif
    return NULL;
}

void* BuildIndex(void *pThreadArgs)
{
    ThreadArgs* pThdArgs = (ThreadArgs*)pThreadArgs;
    int threadID = pThdArgs->threadID;
    BindToCore(threadID);

    uint64_t opCount = pThdArgs->opCount;
    KEY_TYPE* keyArray = pThdArgs->keyArray;
    VAL_TYPE* valArray = pThdArgs->valArray;
    construct_container(pThdArgs->container, keyArray, keyArray, opCount);    
    return NULL;
}



// load data from file
void RunExperimentYCSB(ExperimentConfig* pExpCfg)
{

    uint64_t *keyArray = (KEY_TYPE*)malloc(sizeof(uint64_t) * pExpCfg->numKeys);
    uint64_t *valArray = (VAL_TYPE*)malloc(sizeof(uint64_t) * pExpCfg->numKeys);
    int *rangeArray = (int*)malloc(sizeof(int) * pExpCfg->numKeys);
    OP_CODE* opArray = (OP_CODE*)malloc(sizeof(OP_CODE) * pExpCfg->numKeys);
    
    LoadInitDataFromFile(pExpCfg, keyArray, valArray, opArray);
    std::sort(valArray, valArray + pExpCfg->numKeys);

    pthread_t ftids[MAX_NUM_THREADS];
    ThreadArgs fthdArgs[MAX_NUM_THREADS];

    struct mallinfo mi_start, mi_end;

    mi_start = mallinfo();

    void* container = init_container(pExpCfg);
    
    {
        fthdArgs[0].threadID = 1;
        fthdArgs[0].container = container;
        fthdArgs[0].keyArray = keyArray;
        fthdArgs[0].valArray = valArray;
        fthdArgs[0].opArray = opArray;
        fthdArgs[0].opCount = pExpCfg->numKeys;
    }

    

    // Building Phase 
    struct timeval start, end;
    std::sort(keyArray, keyArray + pExpCfg->numKeys);
    uint64_t start_mem, end_mem;

    start_mem = GetMemoryUsage();
    gettimeofday(&start, NULL);
    
    // for(int i = 0; i < 1; i++)
    {
        pthread_create(&ftids[0], NULL, BuildIndex, &fthdArgs[0]);
    }

    // for(int i = 0; i < pExpCfg->numWriteThread; i++)
    {
        pthread_join(ftids[0], NULL);
    }

    gettimeofday(&end, NULL);
    
    uint64_t totalus = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    printf("Build=%lf\n", (double)totalus / 1000000);
    end_mem = GetMemoryUsage();



    printf("Memory=%ld\n", end_mem - start_mem);
    // printf("Per KV use %lf B\n", (end_mem - start_mem) * 1024.0 / (pExpCfg->numKeys));
    
    // Transaction Phase 
    pthread_t stids[MAX_NUM_THREADS];
    ThreadArgs sthdArgs[MAX_NUM_THREADS];


    LoadTxnDataFromFile(pExpCfg, keyArray, valArray, rangeArray, opArray);
    uint64_t opCountPerThread = pExpCfg->numTxns / pExpCfg->numTxnThread;
    for(int i = 0; i < pExpCfg->numTxnThread; i++)
    {
        sthdArgs[i].threadID = i + 1;
        sthdArgs[i].container = container;
        sthdArgs[i].keyArray = keyArray + i * opCountPerThread;
        sthdArgs[i].valArray = valArray + i * opCountPerThread;
        sthdArgs[i].rangeArray = rangeArray + i * opCountPerThread;
        sthdArgs[i].opArray = opArray + i * opCountPerThread;
        sthdArgs[i].opCount = opCountPerThread;
    }

    gettimeofday(&start, NULL);
    for(int i = 0; i < pExpCfg->numTxnThread; i++)
    {
        pthread_create(&stids[i], NULL, ExecuteWorkload, &sthdArgs[i]);
    }

    for(int i = 0; i < pExpCfg->numTxnThread; i++)
    {
        pthread_join(stids[i], NULL);
    }

    gettimeofday(&end, NULL);
    totalus = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    printf("Txn=%lf\n", pExpCfg->numTxns / (double)totalus);

}

// load data from file
void RunExperimentAblationMemory(ExperimentConfig* pExpCfg)
{
    uint64_t *keyArray = (KEY_TYPE*)malloc(sizeof(uint64_t) * pExpCfg->numKeys);
    OP_CODE* opArray = (OP_CODE*)malloc(sizeof(OP_CODE) * pExpCfg->numKeys);
    
    std::vector<uint64_t> data = load_data<uint64_t>(pExpCfg->initFileName);
    for(int i = 0; i < pExpCfg->numKeys; i++) keyArray[i] = data[i];
    for(int i = 0; i < pExpCfg->numKeys; i++) opArray[i] = OP_WRITE;

    pthread_t ftids[MAX_NUM_THREADS];
    ThreadArgs fthdArgs[MAX_NUM_THREADS];

    struct mallinfo mi_start, mi_end;

    mi_start = mallinfo();

    void* container = init_container(pExpCfg);
    {
        fthdArgs[0].threadID = 1;
        fthdArgs[0].container = container;
        fthdArgs[0].keyArray = keyArray;
        // fthdArgs[0].valArray = keyArray;
        fthdArgs[0].opArray = opArray;
        fthdArgs[0].opCount = pExpCfg->numKeys;
    }

    

    // Building Phase 
    struct timeval start, end;

    std::sort(keyArray, keyArray + pExpCfg->numKeys);
    uint64_t start_mem, end_mem;

    start_mem = GetMemoryUsage();
    gettimeofday(&start, NULL);
    {
        pthread_create(&ftids[0], NULL, BuildIndex, &fthdArgs[0]);
    }
    {
        pthread_join(ftids[0], NULL);
    }

    gettimeofday(&end, NULL);
    
    uint64_t totalus = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;

    printf("Build=%lf\n", (double)totalus / 1000000);
    end_mem = GetMemoryUsage();
    printf("Memory=%ld\n", end_mem - start_mem);
    free(keyArray);
    free(opArray);
}

// load data from file
void RunExperimentAblationOp(ExperimentConfig* pExpCfg)
{
    uint64_t *keyArray = (KEY_TYPE*)malloc(sizeof(uint64_t) * pExpCfg->numKeys);
    OP_CODE* opArray = (OP_CODE*)malloc(sizeof(OP_CODE) * pExpCfg->numKeys);
    
    std::vector<uint64_t> data = load_data<uint64_t>(pExpCfg->initFileName);
    for(int i = 0; i < pExpCfg->numKeys; i++) keyArray[i] = data[i];
    for(int i = 0; i < pExpCfg->numKeys; i++) opArray[i] = OP_WRITE;
    std::random_shuffle(keyArray, keyArray + pExpCfg->numKeys);

    pthread_t ftids[MAX_NUM_THREADS];
    ThreadArgs fthdArgs[MAX_NUM_THREADS];

    struct mallinfo mi_start, mi_end;

    mi_start = mallinfo();

    void* container = init_container(pExpCfg);
    {
        fthdArgs[0].threadID = 1;
        fthdArgs[0].container = container;
        fthdArgs[0].keyArray = keyArray;
        fthdArgs[0].opArray = opArray;
        fthdArgs[0].opCount = pExpCfg->numKeys;
    }

    

    // Building Phase 
    struct timeval start, end;
    gettimeofday(&start, NULL);
    {
        pthread_create(&ftids[0], NULL, BuildIndex, &fthdArgs[0]);
    }
    {
        pthread_join(ftids[0], NULL);
    }

    gettimeofday(&end, NULL);
    
    uint64_t totalus = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;

    printf("Insert=%lf\n", pExpCfg->numKeys / (double)totalus);
    
    // Transaction Phase 
    pthread_t stids[MAX_NUM_THREADS];
    ThreadArgs sthdArgs[MAX_NUM_THREADS];

    std::random_shuffle(keyArray, keyArray + pExpCfg->numKeys);
    
    for(int i = 0; i < pExpCfg->numTxns; i++) opArray[i] = OP_READ;
    uint64_t opCountPerThread = pExpCfg->numTxns / pExpCfg->numTxnThread;
    for(int i = 0; i < pExpCfg->numTxnThread; i++)
    {
        sthdArgs[i].threadID = i + 1;
        sthdArgs[i].container = container;
        sthdArgs[i].keyArray = keyArray + i * opCountPerThread;
        sthdArgs[i].opArray = opArray + i * opCountPerThread;
        sthdArgs[i].opCount = opCountPerThread;
    }

    gettimeofday(&start, NULL);
    for(int i = 0; i < pExpCfg->numTxnThread; i++)
    {
        pthread_create(&stids[i], NULL, ExecuteWorkload, &sthdArgs[i]);
    }

    for(int i = 0; i < pExpCfg->numTxnThread; i++)
    {
        pthread_join(stids[i], NULL);
    }

    gettimeofday(&end, NULL);
    totalus = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    printf("Read=%lf\n", pExpCfg->numTxns / (double)totalus);

}

void RunExperimentOthers(ExperimentConfig* pExpCfg)
{
    uint64_t *keyArray = (KEY_TYPE*)malloc(sizeof(uint64_t) * (pExpCfg->numKeys));
    OP_CODE* opArray = (OP_CODE*)malloc(sizeof(OP_CODE) * (pExpCfg->numKeys));
    
    std::vector<uint64_t> data = load_data<uint64_t>(pExpCfg->initFileName);
    std::random_shuffle(data.begin(), data.begin() + (pExpCfg->numKeys + pExpCfg->numTxns));
    

    for(int i = 0; i < pExpCfg->numKeys; i++) keyArray[i] = data[i];
    for(int i = 0; i < pExpCfg->numKeys; i++) opArray[i] = OP_WRITE;
    std::sort(keyArray, keyArray + pExpCfg->numKeys);
    
    pthread_t ftids[MAX_NUM_THREADS];
    ThreadArgs fthdArgs[MAX_NUM_THREADS];
    struct timeval start, end;
    uint64_t start_mem, end_mem;

    start_mem = GetMemoryUsage();
    void* container = init_container(pExpCfg);
    {
        fthdArgs[0].threadID = 1;
        fthdArgs[0].container = container;
        fthdArgs[0].keyArray = keyArray;
        // fthdArgs[0].valArray = keyArray;
        fthdArgs[0].opArray = opArray;
        fthdArgs[0].opCount = pExpCfg->numKeys;
    }

    // Building Phase 
    
    gettimeofday(&start, NULL);
    // for(int i = 0; i < 1; i++)
    {
        pthread_create(&ftids[0], NULL, BuildIndex, &fthdArgs[0]);
    }
    // for(int i = 0; i < pExpCfg->numWriteThread; i++)
    {
        pthread_join(ftids[0], NULL);
    }
    gettimeofday(&end, NULL);
    
    uint64_t totalus = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    printf("Build=%lf\n", (double)totalus / 1000000);

    end_mem = GetMemoryUsage();



    printf("Memory=%ld\n", end_mem - start_mem);

    std::mt19937 gen;
	std::uniform_real_distribution<> dis(0, 1);
	for(int i = 0; i < pExpCfg->numTxns; i++)
    {
        if(dis(gen) <= pExpCfg->ratio)
        {
            opArray[i] = OP_READ;
            keyArray[i] = data[i];
        }
        else
        {
            opArray[i] = OP_WRITE;
            keyArray[i] = data[pExpCfg->numKeys + i];
        }
    }



    pthread_t stids[MAX_NUM_THREADS];
    ThreadArgs sthdArgs[MAX_NUM_THREADS];
    
    // for(int i = 0; i < pExpCfg->numTxns; i++) opArray[i] = OP_READ;
    uint64_t opCountPerThread = pExpCfg->numTxns / pExpCfg->numTxnThread;
    for(int i = 0; i < pExpCfg->numTxnThread; i++)
    {
        sthdArgs[i].threadID = i + 1;
        sthdArgs[i].container = container;
        sthdArgs[i].keyArray = keyArray + i * opCountPerThread;
        sthdArgs[i].opArray = opArray + i * opCountPerThread;
        sthdArgs[i].opCount = opCountPerThread;
    }

    gettimeofday(&start, NULL);
    for(int i = 0; i < pExpCfg->numTxnThread; i++)
    {
        pthread_create(&stids[i], NULL, ExecuteWorkload, &sthdArgs[i]);
    }

    for(int i = 0; i < pExpCfg->numTxnThread; i++)
    {
        pthread_join(stids[i], NULL);
    }

    gettimeofday(&end, NULL);
    totalus = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    printf("Txn=%lf\n", pExpCfg->numTxns / (double)totalus);
}

// load configuration from config file
ExperimentConfig* ReadExperimentConfig(char* configFile)
{
    ExperimentConfig* pExpCfg = (ExperimentConfig*)calloc(sizeof(ExperimentConfig), 1);
    size_t read = 0;
    size_t len = 1024;
    char *pLine = (char*)calloc(len, 1);
    FILE *file = fopen(configFile, "r");
    if(file)
    {
        char *pKey, *pVal;
        while((read = getline(&pLine, &len, file)) != -1)
        {
            pKey = strtok(pLine, "=");
            pVal = strtok(NULL, "=\n");
            if (!strcmp(pKey, "experimentID"))
                pExpCfg->experimentID = atoi(pVal);
            else if(!strcmp(pKey, "numKeys"))
            {
                pExpCfg->numKeys = atoi(pVal) * CONST_MILLION;
            }    
            else if (!strcmp(pKey, "numTxns"))
                pExpCfg->numTxns = atol(pVal) * CONST_MILLION;
            else if (!strcmp(pKey, "numTxnThread"))
                pExpCfg->numTxnThread = atol(pVal);
            else if (!strcmp(pKey, "readRatio"))
                pExpCfg->ratio = atof(pVal);
            else if (!strcmp(pKey, "initFileName"))
            {
                int len = strlen(pVal);
                if(pVal[len - 1] == '\r' || pVal[len - 1] == '\n') len--;
                memcpy(pExpCfg->initFileName, pVal, len);
            }
            else if (!strcmp(pKey, "txnFileName"))
            {
                int len = strlen(pVal);
                if(pVal[len - 1] == '\r' || pVal[len - 1] == '\n') len--;
                memcpy(pExpCfg->txnFileName, pVal, len);
            }
            else
            {
                printf("Unrecognized argument:%s\n", pKey);
            }
        }
    }
    fclose(file);
    free(pLine);
    return pExpCfg;
}

// load key-value pairs from ycsb file
void LoadInitDataFromFile(ExperimentConfig* pExpCfg, uint64_t* keyArray, uint64_t* valArray, OP_CODE* opArray)
{
    size_t read = 0;
    size_t len = 1024;
    char *pLine = (char*)calloc(len, 1);
    size_t numKeys = 0;
    FILE *pInitFile = fopen(pExpCfg->initFileName, "r");
    if(pInitFile)
    {
        char *pKey, *pOpCode; 
        while((read = getline(&pLine, &len, pInitFile)) != -1)
        {
            pOpCode = strtok(pLine, " ");
            pKey = strtok(NULL, " ");
            if (!strcmp(pOpCode, "INSERT"))
            {
                keyArray[numKeys] = atoll(pKey);
                valArray[numKeys] = keyArray[numKeys];
                opArray[numKeys] = OP_WRITE;
                numKeys++;
            }
            else
            {
                printf("Illegal Opcode:%s\n", pOpCode);
            }
        }
    }
    fclose(pInitFile);
    pExpCfg->numKeys = numKeys;
    free(pLine);
}

void LoadTxnDataFromFile(ExperimentConfig* pExpCfg, uint64_t* keyArray, uint64_t* valArray, int* rangeArray, OP_CODE* opArray)
{
    size_t read = 0;
    size_t len = 1024;
    char *pLine = (char*)calloc(len, 1);
    size_t numTxns = 0;
    FILE *pTxnFile = fopen(pExpCfg->txnFileName, "r");
    if(pTxnFile)
    {
        char *pKey, *pOpCode, *pRange; 
        while((read = getline(&pLine, &len, pTxnFile)) != -1)
        {
            pOpCode = strtok(pLine, " ");
            pKey = strtok(NULL, " ");
            if (!strcmp(pOpCode, "INSERT"))
            {
                keyArray[numTxns] = atoll(pKey);
                // valArray[numTxns] = keyArray[numTxns];
                opArray[numTxns] = OP_WRITE;
                numTxns++;
            }
            else if (!strcmp(pOpCode, "READ"))
            {
                keyArray[numTxns] = atoll(pKey);
                opArray[numTxns] = OP_READ;
                numTxns++;
            }
            else if (!strcmp(pOpCode, "UPDATE"))
            {
                keyArray[numTxns] = atoll(pKey);
                // valArray[numTxns] = keyArray[numTxns];
                opArray[numTxns] = OP_WRITE;
                numTxns++;
            }
            else if (!strcmp(pOpCode, "SCAN"))
            {
                pRange = strtok(NULL, " ");
                keyArray[numTxns] = atoll(pKey);
                opArray[numTxns] = OP_SCAN;
                rangeArray[numTxns] = atoi(pRange);
                numTxns++;
            }
            else
            {
                printf("Illegal Opcode:%s\n", pOpCode);
            }
        }
    }
    fclose(pTxnFile);
    pExpCfg->numTxns = numTxns;
    free(pLine);
}

int main(int argc, char *argv[])
{
    BindToCore(0);
    if(argc == 1)
    {
        printf("./benchmark_xxx job.conf\n");
        assert(0);
    }
    ExperimentConfig *pExpCfg = ReadExperimentConfig(argv[1]);
    switch(pExpCfg->experimentID)
    {
        case 1:
            RunExperimentYCSB(pExpCfg);
            break;
        case 2:
            RunExperimentAblationMemory(pExpCfg);
            RunExperimentAblationOp(pExpCfg);
            break;
        case 3:
            RunExperimentOthers(pExpCfg);
            break;
        default:
            printf("Undefined experiment id %ld\n", pExpCfg->experimentID);
    }
    return 0;
}