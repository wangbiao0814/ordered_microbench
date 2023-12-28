CPP = g++ -D U64_KEY_U64_VALUE 
CFLAGS = -O3 -std=c++17
CFLAGS_ALEX = -mpopcnt -mlzcnt -march=native 
LIBS = -lm -lpthread 
LIBS_WORMHOLE = -L./competitor/wormhole -lwh 
LIBS_ART = -L./competitor/libart/src -lart

INCS = -I./competitor/ -I./dataset/ -I./benchmark/ -I./dataset/cpp_random_distributions

INCS_CUCKOOHT = -I./competitor/libcuckoo/install/include
INCS_WORMHOLE = -I./competitor/wormhole
INCS_STXBTREE = -I./competitor/stx
INCS_ART = -I./competitor/libart/src
INCS_VEB = -I./competitor/vEB -I./competitor/vIndex/gtl-1.1.8/include
INCS_VINDEX = -I./competitor/vIndex -I./competitor/vIndex/gtl-1.1.8/include
INCS_ALEX = -I./competitor/ALEX/src


CUCKOOHASH_BENCH_SRC = ./benchmark/benchmark.cpp ./competitor/wrapper_cuckoohash.cpp
WORMHOLE_BENCH_SRC = ./benchmark/benchmark.cpp ./competitor/wrapper_wormhole.cpp
STXBTREE_BENCH_SRC = ./benchmark/benchmark.cpp ./competitor/wrapper_stxbtree.cpp
ART_BENCH_SRC = ./benchmark/benchmark.cpp ./competitor/wrapper_art.cpp
VINDEX_BENCH_SRC = ./benchmark/benchmark.cpp ./competitor/wrapper_vIndex.cpp ./competitor/vIndex/vIndexNode.cc ./competitor/vIndex/CSRNode.cc ./competitor/vIndex/DeltaLeafNode.cc ./competitor/vIndex/BitmapLeafNode.cc ./competitor/vIndex/vIndex.cc
VEB_BENCH_SRC = ./benchmark/benchmark.cpp ./competitor/wrapper_vEB.cpp ./competitor/vEB/ASTree.cc ./competitor/vEB/CSR.cc
CUCKOOTRIE_BENCH_SRC = ./benchmark/benchmark.cpp ./competitor/wrapper_cuckootrie.cpp 

ALEX_BENCH_SRC = ./benchmark/benchmark.cpp ./competitor/wrapper_alex.cpp

all: run_bench_vEB run_bench_wormhole run_bench_stxbtree run_bench_art run_bench_vIndex_flat run_bench_ALEX

run_bench_vEB:
	$(CPP) $(CFLAGS) $(INCS) $(INCS_VEB) $(VEB_BENCH_SRC) -o $@ $(LIBS) -DVEB
run_bench_cuckooHT:
	$(CPP) $(CFLAGS) $(INCS) $(INCS_CUCKOOHT) $(CUCKOOHASH_BENCH_SRC) -o $@ $(LIBS) -DCUCKOOHASH
run_bench_wormhole:
	$(CPP) $(CFLAGS) $(INCS) $(INCS_WORMHOLE) $(WORMHOLE_BENCH_SRC) -o $@ $(LIBS) $(LIBS_WORMHOLE) -DWORMHOLE
run_bench_stxbtree:
	$(CPP) $(CFLAGS) $(INCS) $(INCS_STXBTREE) $(STXBTREE_BENCH_SRC) -o $@ $(LIBS) -DSTXBTREE
run_bench_art:
	$(CPP) $(CFLAGS) $(INCS) $(INCS_ART) $(ART_BENCH_SRC) -o $@ $(LIBS) $(LIBS_ART) -DART
run_bench_vIndex:
	$(CPP) $(CFLAGS) $(INCS) $(INCS_VINDEX) $(VINDEX_BENCH_SRC) -o $@ $(LIBS) -DVINDEX
run_bench_ALEX:
	$(CPP) $(CFLAGS) $(CFLAGS_ALEX) $(INCS) $(INCS_ALEX) $(ALEX_BENCH_SRC) -o $@ $(LIBS) -DALEX

clean:
	rm -rf *.o run_bench_*