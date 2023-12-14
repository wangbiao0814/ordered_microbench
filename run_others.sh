#!/bin/bash


for run_bench in run_bench_ALEX run_bench_vIndex run_bench_cuckooHT
do 
    for dataset in wiki fb osm randint
    do
        for ratio in 0.0 0.2 0.4 0.6 0.8 1.0 
        do
	        echo ${run_bench}_${dataset}_${ratio}.conf
            ./${run_bench} ./experiment/others/${dataset}_${ratio}.conf >> ./experiment/others/${run_bench}.txt
        done
    done
done