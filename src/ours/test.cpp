#include "../competitor/competitor.h"
#include "gflags/gflags.h"
#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "utils.h"

using namespace std;

DEFINE_string(test_type, "ro", "ro/rw");
DEFINE_string(index, "fh_index_ro", "index name");
DEFINE_string(dataset, "/data/xding9001/li/libio",
              "path to dataset, or uniform");
DEFINE_int32(round, 4, "# of rounds");
DEFINE_uint64(num_keys, 0, "# of keys");

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
  timer.stop();
  parlay::parallel_for(0, entries.size(), [&](size_t i) {
    uint64_t val;
    bool ok = index->get(entries[i].first, val);
    assert(ok && val == entries[i].second);
  });
  timer.start();
  parlay::parallel_for(0, e2.size(), [&](int i) {
    bool ok = index->remove(e2[i].first);
    assert(ok);
  });
  timer.stop();
  double duration = timer.total_time();
  cout << "Insert and Delete duration: " << duration << endl;
  double mops = (double)n / duration / 1e6;
  cout << "Mops: " << mops << endl;
  cout << "All good!" << endl;
  delete index;
}

/*
alexol, lippol, xindex: result not correct
finedex: stuck on scan queries
*/
void TestRead(parlay::sequence<pair<uint64_t, uint64_t>> &entries) {
  auto n = entries.size();
  cout << "Start bulk_load" << endl;
  auto memory_before_bulk_load = GetJemallocAllocated();
  auto index = get_index<uint64_t, uint64_t>(FLAGS_index);
  index->bulk_load(entries.data(), n);
  auto memory_after_bulk_load = GetJemallocAllocated();
  cout << "End Bulk_load" << endl;
  cout << "Index memory usage: "
       << memory_after_bulk_load - memory_before_bulk_load << endl;

  // if (FLAGS_index == "fh_index_rw") {
  //   parlay::parlay_unordered_map<uint64_t, uint64_t, fh_index::internal::hash>
  //       hash_table(200000000);

  //   parlay::sequence<uint64_t> a(n);
  //   parlay::parallel_for(0, n, [&](size_t i) { a[i] = parlay::hash64_2(i); });
  //   // parlay::sort_inplace(a);

  //   parlay::parallel_for(0, n, [&](size_t i) { hash_table.Insert(a[i], i); });

  //   parlay::internal::timer tm;
  //   parlay::parallel_for(0, n,
  //                        [&](size_t i) { assert(hash_table.Find(a[i]) == i); });
  //   auto duration = tm.stop();

  //   cout << "Duration: " << duration << endl;
  //   double mops = (double)n / duration / 1e6;
  //   cout << "Hash Mops: " << mops << endl;
  // }

  double total_mops = 0;
  bool good = true;
  vector<int> res(n, 1);
  for (int r = 0; r < FLAGS_round; r++) {
    cout << "\nRound: " << r << endl;
    auto ids = parlay::random_permutation(n);
    // parlay::sequence<size_t> ids = {n/2};

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
  cout << "Single Read Average Mops: " << avg_mops << endl;

  return;

  vector<size_t> scan_size_list = {10, 50, 100};
  for (auto scan_size : scan_size_list) {
    double total_mops = 0;
    bool good = true;
    auto queries = entries;
    queries.resize(n - scan_size);
    parlay::sequence tmp(parlay::num_workers(),
                         parlay::sequence<pair<uint64_t, uint64_t>>(scan_size));
    vector<int> res(n, 1);
    bool check = false;
    for (int r = 0; r < FLAGS_round; r++) {
      cout << "\nRound: " << r << endl;
      queries = parlay::random_shuffle(queries);
      parlay::internal::timer timer;
      parlay::parallel_for(0, queries.size(), [&](size_t i) {
        uint64_t key_low_bound;
        i = queries[i].second;
        if (i == 0) {
          key_low_bound = entries[i].first / 2;
        } else {
          if (entries[i].first > entries[i].first + 1) {
            key_low_bound = (entries[i].first + entries[i].first) / 2;
          } else {
            key_low_bound = entries[i].first;
          }
        }
        auto worker_id = parlay::worker_id();
        index->scan(key_low_bound, scan_size, tmp[worker_id].data());
        if (check) {
          auto &result = tmp[worker_id];
          bool ok = true;
          for (size_t j = 0; j < scan_size; j++) {
            ok &= entries[i + j] == result[j];
          }
          res[i] = ok;
        }
      });
      double duration = timer.stop();
      cout << "Duration: " << duration << endl;
      double mops = (double)(queries.size()) / duration / 1e6;
      cout << "Mops: " << mops << endl;
      if (r > 0) {
        total_mops += mops;
      }
      if (parlay::any_of(res, [&](int x) { return x == 0; })) {
        good = false;
      }
    }
    cout << "good: " << (good ? "true" : "false") << endl;
    double avg_mops = total_mops / (FLAGS_round - 1);
    cout << "Scan(" << scan_size << ") Average Mops: " << avg_mops << endl;
  }
  delete index;
}

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  cout << "\nStart new benchmark test" << endl;
  cout << "Test Type: " << FLAGS_test_type << endl;
  cout << "Index: " << FLAGS_index << endl;
  cout << "Dataset: " << FLAGS_dataset << endl;

  parlay::sequence<pair<uint64_t, uint64_t>> entries;
  if (FLAGS_dataset == "uniform") {
    entries = UniformRandomEntries(FLAGS_num_keys);
  } else {
    entries = LoadEntries(FLAGS_dataset, FLAGS_num_keys);
  }

  parlay::parallel_for(1, entries.size(), [&](size_t i) {
    assert(entries[i].first > entries[i - 1].first);
  });

  if (FLAGS_test_type == "ro") {
    TestRead(entries);
  } else if (FLAGS_test_type == "rw") {
    TestReadWrite(entries);
  } else {
    assert(0);
  }

  return 0;
}
