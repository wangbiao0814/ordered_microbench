#!/bin/bash


for run_bench in run_bench_stxbtree run_bench_art run_bench_wormhole run_bench_vIndex
do 
    for dataset in wiki face osm randint   
    do
        for workload in workloada workloadb workloadc workloadd workloade workloadf
        do
	      echo ${run_bench}_${dataset}_${workload}.conf
            echo ${dataset}_${workload}.conf >> ./experiment/ycsb/${run_bench}.txt
            ./${run_bench} ./experiment/ycsb/${dataset}_${workload}.conf >> ./experiment/ycsb/${run_bench}.txt
        done
    done
done




