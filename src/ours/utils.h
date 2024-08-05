#ifndef UTILS_H_
#define UTILS_H_

#include "../benchmark/utils.h"
#include "parlay/primitives.h"

template <typename T = int>
parlay::sequence<std::pair<uint64_t, uint64_t>>
LoadEntries(const std::string &path, size_t limit = 0) {
  std::cout << "Loading dataset from " << path << std::endl;
  uint64_t *data;
  auto n = load_binary_data(data, -1, path);
  parlay::sort_inplace(parlay::make_slice(data, data + n));
  auto keys = parlay::unique(parlay::make_slice(data, data + n));
  if (limit > 0) {
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

#endif // UTILS_H_
