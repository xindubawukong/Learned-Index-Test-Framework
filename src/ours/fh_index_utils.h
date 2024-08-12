#ifndef FH_INDEX_UTILS_H_
#define FH_INDEX_UTILS_H_

#include <cstddef>

// FHIndexRW
const size_t kSizeLowerLeaf = 32;
const size_t kSizeUpperLeaf = 64;
const size_t kFingerPerBlockLeaf = 8;

const size_t kSizeLowerInternal = 8;
const size_t kSizeUpperInternal = 16;
const size_t kFingerPerBlockInternal = 2;

// FHIndexRO
const size_t kBlockSize = 64;
const size_t kFingerDensity = 8;
const size_t kInternalBlockSize = 16;
const size_t kInternalFingerDensity = 2;

// Common
const size_t kTopLevelThreshold = 1024;

#endif  // FH_INDEX_UTILS_H_
