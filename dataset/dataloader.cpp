#include "dataloader.h"
#include <unordered_set>
#include <fstream>
#include <random>
#include <climits>
#include <cstring>
#define N 64000000


void LoadInitDataFromFile(char* fileName, uint64_t* keyArray)
{
    size_t read = 0;
    size_t len = 1024;
    char *pLine = (char*)calloc(len, 1);
    size_t numKeys = 0;
    FILE *pInitFile = fopen(fileName, "r");
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
                numKeys++;
            }
            else
            {
                printf("Illegal Opcode:%s\n", pOpCode);
            }
        }
    }
    fclose(pInitFile);
    // pExpCfg->numKeys = numKeys;
    free(pLine);
}

void LoadTxnDataFromFile(char* fileName, uint64_t* keyArray)
{
    size_t read = 0;
    size_t len = 1024;
    char *pLine = (char*)calloc(len, 1);
    size_t numTxns = 0;
    FILE *pTxnFile = fopen(fileName, "r");
    if(pTxnFile)
    {
        char *pKey, *pOpCode, *pRange; 
        while((read = getline(&pLine, &len, pTxnFile)) != -1)
        {
            pOpCode = strtok(pLine, " ");
            pKey = strtok(NULL, " ");
            if (!strcmp(pOpCode, "READ"))
            {
                keyArray[numTxns] = atoll(pKey);
                numTxns++;
            }

        }
    }
    fclose(pTxnFile);
    free(pLine);
}

int main()
{

#ifdef GENERATE_TXT_FILE_FOR_REAL_WORLD_DATASETS
    std::vector<uint64_t> wiki = load_data<uint64_t>("fb_64M_uint64");
    write_data_txt<uint64_t>(wiki, "fb_64M_uint64.txt");

    std::vector<uint64_t> fb = load_data<uint64_t>("wiki_ts_64M_uint64");
    write_data_txt<uint64_t>(fb, "wiki_ts_64M_uint64.txt");

    std::vector<uint64_t> osm = load_data<uint64_t>("osm_cellids_64M_uint64");
    write_data_txt<uint64_t>(osm, "osm_cellids_64M_uint64.txt");
#endif


#ifdef GENERATE_SYSTHETIC_DATASETS_AFTER_YCSB
    uint64_t* keyArray = new uint64_t[N];
    LoadInitDataFromFile("../workloads/workloadc_randint_load.dat", keyArray);
    std::vector<uint64_t> data;
    for(int i = 0; i < N; i++) data.push_back(keyArray[i]);
    std::sort(data.begin(), data.end());
    write_data<uint64_t>(data, "uniform_64M_uint64");
#endif

    

    return 0;
}