all : generate_realworld_dataset generate_systhetic_dataset

generate_realworld_dataset:
	g++ -o generate_realworld_dataset -O2 dataloader.cpp -DGENERATE_TXT_FILE_FOR_REAL_WORLD_DATASETS
generate_systhetic_dataset:
	g++ -o generate_systhetic_dataset -O2 dataloader.cpp -DGENERATE_SYSTHETIC_DATASETS_AFTER_YCSB