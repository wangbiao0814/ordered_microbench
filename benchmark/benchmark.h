#ifndef _BENCHMARK_H_
#define _BENCHMARK_H_

#include <stdint.h>
#include <pthread.h>
#include "wrapper.h"

// benchmark suites

void LoadInitDataFromFile(ExperimentConfig* pExpCfg, uint64_t* keyArray, uint64_t* valArray, OP_CODE* opArray);
void LoadTxnDataFromFile(ExperimentConfig* pExpCfg, uint64_t* keyArray, uint64_t* valArray, int* rangeArray, OP_CODE* opArray);
#endif