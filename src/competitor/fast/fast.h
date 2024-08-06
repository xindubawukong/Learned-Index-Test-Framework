#include "../indexInterface.h"
#include "parlay/primitives.h"
#include <immintrin.h>
#include <iostream>

static __inline__ __m256i_u
    __attribute__((__always_inline__, __nodebug__, __target__("avx2"),
                   __min_vector_width__(256)))
    _mm256_cmpgt_epu32(__m256i_u __a, __m256i_u __b) {
  return (__m256i_u)((__v8su)__a > (__v8su)__b);
}

static inline __m256i_u
    __attribute__((__always_inline__, __nodebug__, __target__("avx2"),
                   __min_vector_width__(256)))
    _mm256_cmpgt_epu64(__m256i_u __a, __m256i_u __b) {
  return (__m256i_u)((__v4du)__a > (__v4du)__b);
}

#include "./src/fast.h"

template <class KEY_TYPE, class PAYLOAD_TYPE>
class FastInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
public:
  void init(Param *param = nullptr) {}

  void bulk_load(std::pair<KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num,
                 Param *param = nullptr) {
    kvs.resize(num);
    parlay::parallel_for(0, num, [&](size_t i) { kvs[i] = key_value[i]; });
    auto keys =
        parlay::tabulate(num, [&](size_t i) { return key_value[i].first; });
    index.buildFAST(keys.begin(), num);
  }

  bool get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param = nullptr) {
    size_t lb = index.lower_bound(key);
    if (kvs[lb].first == key) {
      val = kvs[lb].second;
      return true;
    } else {
      return false;
    }
  }

  bool put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr) {
    return false;
  }

  bool update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr) {}

  bool remove(KEY_TYPE key, Param *param = nullptr) {}

  size_t scan(KEY_TYPE key_low_bound, size_t key_num,
              std::pair<KEY_TYPE, PAYLOAD_TYPE> *result,
              Param *param = nullptr) {
    size_t lb = index.lower_bound(key_low_bound);
    size_t cnt = 0;
    while (lb < kvs.size() && cnt < key_num) {
      result[cnt++] = kvs[lb++];
    }
    return cnt;
  }

  long long memory_consumption() {}

private:
  fast::FAST<KEY_TYPE> index;
  std::vector<std::pair<KEY_TYPE, PAYLOAD_TYPE>> kvs;
};