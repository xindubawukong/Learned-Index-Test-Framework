#include "../competitor/competitor.h"
#include "gflags/gflags.h"
#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "utils.h"

using namespace std;

DEFINE_string(test_type, "ro", "ro/rw/scan");
DEFINE_string(index, "fh_index_ro", "index name");
DEFINE_string(dataset, "/colddata/xding9001/li/libio", "path to dataset");
DEFINE_int32(round, 5, "# of rounds");
DEFINE_uint64(num_keys, 100000, "# of keys");
DEFINE_uint64(scan_size, 10, "scan size");

void TestReadOnly(parlay::sequence<pair<uint64_t, uint64_t>> &entries) {
  cout << "Test Read Only" << endl;
  auto n = entries.size();
  cout << "Start bulk_load" << endl;
  auto index = get_index<uint64_t, uint64_t>(FLAGS_index);
  index->bulk_load(entries.data(), n);
  cout << "End Bulk_load" << endl;
  ReportJemallocAllocated();

  double total_mops = 0;
  bool good = true;
  vector<int> res(n);
  for (int r = 0; r < FLAGS_round; r++) {
    cout << "\nRound: " << r << endl;
    auto ids = parlay::random_permutation(n);

    parlay::internal::timer timer;
    parlay::parallel_for(0, n, [&](int i) {
      uint64_t val;
      bool ok = index->get(entries[ids[i]].first, val);
      res[i] = (ok && val == ids[i]);
    });
    double duration = timer.stop();
    if (parlay::any_of(res, [&](int x) { return x == 0; })) good = false;

    cout << "Duration: " << duration << endl;
    double mops = (double)n / duration / 1e6;
    cout << "Mops: " << mops << endl;

    if (r > 0) {
      total_mops += mops;
    }
  }

  cout << "good: " << (good ? "true" : "false") << endl;
  double avg_mops = total_mops / (FLAGS_round - 1);
  cout << "Average Mops: " << avg_mops << endl;
  delete index;
}

void TestReadWrite(parlay::sequence<pair<uint64_t, uint64_t>> &entries0) {
  auto entries = entries0;
  entries = parlay::random_shuffle(entries);
  auto n = entries.size();
  auto e1 = entries.subseq(0, n / 2);
  auto e2 = entries.subseq(n / 2, n);
  parlay::sort_inplace(e1);
  parlay::sort_inplace(e2);

  cout << "Start bulk_load" << endl;
  auto index = get_index<uint64_t, uint64_t>(FLAGS_index);
  index->bulk_load(e1.data(), e1.size());
  cout << "End Bulk_load" << endl;

  parlay::internal::timer timer;
  parlay::parallel_for(0, e2.size(), [&](int i) {
    bool ok = index->put(e2[i].first, e2[i].second);
    assert(ok);
  });
  double duration = timer.stop();
  cout << "Insert duration: " << duration << endl;
  double mops = (double)n / duration / 1e6;
  cout << "Mops: " << mops << endl;

  parlay::parallel_for(0, entries.size(), [&](size_t i) {
    uint64_t val;
    bool ok = index->get(entries[i].first, val);
    assert(ok && val == entries[i].second);
  });

  cout << "All good!" << endl;

  delete index;
}

/*
alexol, lippol, xindex: result not correct
finedex: stuck on scan queries
*/
void TestScan(parlay::sequence<pair<uint64_t, uint64_t>> &entries) {
  auto n = entries.size();
  cout << "Start bulk_load" << endl;
  auto index = get_index<uint64_t, uint64_t>(FLAGS_index);
  index->bulk_load(entries.data(), n);
  cout << "End Bulk_load" << endl;

  double total_mops = 0;
  bool good = true;
  auto queries = entries;
  queries.resize(n - FLAGS_scan_size);
  queries = parlay::random_shuffle(queries);
  if (queries.size() > 10000000) {
    queries.resize(10000000);
  }
  size_t q = queries.size();
  cout << "q: " << q << endl;

  parlay::sequence tmp(
      q, parlay::sequence<pair<uint64_t, uint64_t>>(FLAGS_scan_size));
  vector<int> res(n, 1);
  for (int r = 0; r < FLAGS_round; r++) {
    cout << "\nRound: " << r << endl;

    parlay::internal::timer timer;
    parlay::parallel_for(0, q, [&](size_t i) {
      uint64_t key_low_bound;
      size_t id = queries[i].second;
      if (id == 0) {
        key_low_bound = entries[id].first / 2;
      } else {
        if (entries[id].first > entries[id].first + 1) {
          key_low_bound = (entries[id].first + entries[id].first) / 2;
        } else {
          key_low_bound = entries[id].first;
        }
      }
      index->scan(key_low_bound, FLAGS_scan_size, tmp[i].data());
    });
    double duration = timer.stop();
    cout << "Duration: " << duration << endl;
    double mops = (double)(queries.size()) / duration / 1e6;
    cout << "Mops: " << mops << endl;
    if (r > 0) {
      total_mops += mops;
    }

    parlay::parallel_for(0, q, [&](size_t i) {
      size_t id = queries[i].second;
      auto &result = tmp[i];
      bool ok = true;
      for (size_t j = 0; j < FLAGS_scan_size; j++) {
        ok &= entries[id + j] == result[j];
      }
      // if (!ok) {
      //   for (size_t j = 0; j < num; j++) {
      //     cout << entries[i + j].first << ' ' << entries[i + j].second << ' '
      //          << result[j].first << ' ' << result[j].second << endl;
      //   }
      //   exit(0);
      // }
      res[id] = ok;
    });
    if (parlay::any_of(res, [&](int x) { return x == 0; })) {
      good = false;
    }
  }

  cout << "good: " << (good ? "true" : "false") << endl;
  double avg_mops = total_mops / (FLAGS_round - 1);
  cout << "Average Mops: " << avg_mops << endl;
  delete index;
}

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  cout << "\nStart new benchmark test" << endl;
  cout << "Test Type: " << FLAGS_test_type << endl;
  cout << "Index: " << FLAGS_index << endl;
  cout << "Dataset: " << FLAGS_dataset << endl;

  auto entries = LoadEntries(FLAGS_dataset, FLAGS_num_keys);

  parlay::parallel_for(1, entries.size(), [&](size_t i) {
    assert(entries[i].first > entries[i - 1].first);
  });

  if (FLAGS_test_type == "ro") {
    TestReadOnly(entries);
  } else if (FLAGS_test_type == "rw") {
    TestReadWrite(entries);
  } else if (FLAGS_test_type == "scan") {
    TestScan(entries);
  } else {
    assert(0);
  }

  return 0;
}
