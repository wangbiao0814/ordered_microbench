# ordered_microbench
A micro-benchmark for ordered indexes

## vIndex
vIndex is an adaptive stratified tree inspired by van Boas Emde tree(vEB tree) and Compact Sparse Row(CSR), it adaptively store key-value in CSR or vEB tree according to the density of keys(sparse or dense). It offers the following benefits:
* The lookup is efficient, each lookup operation need only $O(\log\log u)$ time, where $u$ is the universe of keys' set.
* Exceptional range query performance, key-value pairs are stored in the sorted array.
* Memory-efficient: the space complexity is $O(n)$, the common prefixes of keys are compressed and the suffixes of keys are stored in low-level compact bitmaps.
  
Current implementation only support 64-bit intergers.
## Build

### Cuckoo Hash
    $ cmake -DCMAKE_INSTALL_PREFIX=../install -DBUILD_EXAMPLES=1 -DBUILD_TESTS=1 ..
    $ make all
    $ make install
### Wormhole
    $ make CCC=gcc
### ART
    $ ./configure
    $ make && make install
STX-Btree and ALEX both provide the head-only way to compile.

## Dataset

The used dataset can be download from https://www.kaggle.com/datasets/wang284751873/real-world-datasets/

## Generate Workload
    $ python generate.py all_workload

## Benchmark
    $ make && ./run_ablation.sh && ./run_ycsb.sh && ./run_others.sh
