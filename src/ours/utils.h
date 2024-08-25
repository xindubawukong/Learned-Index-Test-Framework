#ifndef UTILS_H_
#define UTILS_H_

#include <random>

#include "../benchmark/utils.h"
#include "jemalloc/jemalloc.h"
#include "parlay/primitives.h"

template <typename T = int>
auto UniformRandomEntries(size_t n) {
  if (n == 0) n = 200000000;
  parlay::sequence<uint64_t> keys(n * 1.1);
  std::mt19937_64 rng(0);
  for (size_t i = 0; i < n; i++) keys[i] = rng();
  parlay::sort_inplace(keys);
  keys = parlay::unique(keys);
  assert(keys.size() >= n);
  parlay::sequence<std::pair<uint64_t, uint64_t>> entries(n);
  for (size_t i = 0; i < n; i++) {
    entries[i].first = keys[i];
    entries[i].second = i;
  }
  return entries;
}

template <typename T = int>
auto LoadEntries(const std::string &path, size_t limit = 0) {
  std::cout << "Loading dataset from " << path << std::endl;
  uint64_t *data;
  size_t n = load_binary_data(data, -1, path);
  parlay::sort_inplace(parlay::make_slice(data, data + n));
  auto keys = parlay::unique(parlay::make_slice(data, data + n));
  if (limit > 0 && limit < n) {
    keys = parlay::random_shuffle(keys);
    keys.resize(limit);
    parlay::sort_inplace(keys);
  }
  n = keys.size();
  parlay::sequence<std::pair<uint64_t, uint64_t>> entries(n);
  parlay::parallel_for(0, n, [&](size_t i) {
    entries[i].first = keys[i];
    entries[i].second = i;
  });
  std::cout << "# of entries: " << entries.size() << std::endl;
  std::cout << "Dataset loading complete!" << std::endl;
  delete[] data;
  return entries;
}

template <typename T = int>
size_t GetJemallocAllocated() {
  size_t epoch = 1;
  size_t sz, allocated;
  sz = sizeof(size_t);
  mallctl("thread.tcache.flush", NULL, NULL, NULL, 0);
  mallctl("epoch", NULL, NULL, &epoch, sizeof(epoch));
  mallctl("stats.allocated", &allocated, &sz, NULL, 0);
  return allocated;
}

#endif  // UTILS_H_
