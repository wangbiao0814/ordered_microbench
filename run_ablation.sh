#!/bin/bash


for run_bench in run_bench_vEB run_bench_vIndex run_bench_art run_bench_stxbtree
do 
    for dataset in wiki fb osm randint
    do
        echo ${run_bench}_${dataset}.conf
        echo ${dataset}.conf >> ./experiment/ablation/${run_bench}.txt
        ./${run_bench} ./ablation/${dataset}.conf >> ./experiment/ablation/${run_bench}.txt
    done
done

