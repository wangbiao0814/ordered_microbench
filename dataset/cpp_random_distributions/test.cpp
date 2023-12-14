#include <random>
#include "selfsimilar_int_distribution.h"
#include <iostream>
#include <climits>
#include <cstdint>

int main() {
    std::default_random_engine generator;
    generator.seed(1729);
    int count[11] = {0};
    selfsimilar_int_distribution<uint64_t> distribution(1, 10, 0.4);
    for(int j = 0; j < 1000; j++)
    {
        uint64_t i = distribution(generator);
        count[i]++;
        // std::cout << distribution(generator) << std::endl;
    }
    for(int i = 0; i < 11; i++)
        std::cout << i << " " << count[i] << std::endl;
    return 0; 
    
}