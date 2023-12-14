#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>

#define ROW_WIDTH 1

//#define PRINT_ERRORS


static uint64_t timing(std::function<void()> fn) {
  const auto start = std::chrono::high_resolution_clock::now();
  fn();
  const auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
      .count();
}


// Loads values from binary file into vector.
template <typename T>
static std::vector<T> load_data(const std::string& filename,
                                bool print = true) {
  std::vector<T> data;
  const uint64_t ns = timing([&] {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
      std::cerr << "unable to open " << filename << std::endl;
      exit(EXIT_FAILURE);
    }
    // Read size.
    uint64_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
    data.resize(size);
    // Read values.
    in.read(reinterpret_cast<char*>(data.data()), size * sizeof(T));
    in.close();
  });
  const uint64_t ms = ns / 1e6;

  if (print) {
    std::cout << "read " << data.size() << " values from " << filename << " in "
              << ms << " ms (" << static_cast<double>(data.size()) / 1000 / ms
              << " M values/s)" << std::endl;
  }

  return data;
}

// Writes values from vector into binary file.
template <typename T>
static void write_data(const std::vector<T>& data, const std::string& filename,
                       const bool print = true) {
  const uint64_t ns = timing([&] {
    std::ofstream out(filename, std::ios_base::trunc | std::ios::binary);
    if (!out.is_open()) {
      std::cerr << "unable to open " << filename << std::endl;
      exit(EXIT_FAILURE);
    }
    // Write size.
    const uint64_t size = data.size();
    out.write(reinterpret_cast<const char*>(&size), sizeof(uint64_t));
    // Write values.
    out.write(reinterpret_cast<const char*>(data.data()), size * sizeof(T));
    out.close();
  });
  const uint64_t ms = ns / 1e6;
  if (print) {
    std::cout << "wrote " << data.size() << " values to " << filename << " in "
              << ms << " ms (" << static_cast<double>(data.size()) / 1000 / ms
              << " M values/s)" << std::endl;
  }
}

// Writes values from vector into txt file.
template <typename T>
static void write_data_txt(const std::vector<T>& data, const std::string& filename,
                       const bool print = true) {
  const uint64_t ns = timing([&] {
    std::ofstream out(filename, std::ios::out);
    if (!out.is_open()) {
      std::cerr << "unable to open " << filename << std::endl;
      exit(EXIT_FAILURE);
    }
    // Write size.
    const uint64_t size = data.size();
    // out.write(reinterpret_cast<const char*>(&size), sizeof(uint64_t));
    // Write values.
    for(uint64_t i = 0; i < size; i++)
    {
      out << data[i] << std::endl;
    }
    // out.write(reinterpret_cast<const char*>(data.data()), size * sizeof(T));
    out.close();
  });
  const uint64_t ms = ns / 1e6;
  if (print) {
    std::cout << "wrote " << data.size() << " values to " << filename << " in "
              << ms << " ms (" << static_cast<double>(data.size()) / 1000 / ms
              << " M values/s)" << std::endl;
  }
}









