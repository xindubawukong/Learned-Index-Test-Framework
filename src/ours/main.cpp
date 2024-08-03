#include "../benchmark/utils.h"
#include "../competitor/competitor.h"
#include "gflags/gflags.h"
#include "parlay/parallel.h"
#include "parlay/primitives.h"

using namespace std;

DEFINE_string(index, "fh_index_ro", "index name");
DEFINE_string(dataset, "", "path to dataset");
DEFINE_int32(round, 5, "# of rounds");

vector<pair<uint64_t, uint64_t>> LoadEntries(const string &path) {
  cout << "Loading dataset from " << path << endl;
  vector<pair<uint64_t, uint64_t>> entries;
  uint64_t *keys;
  auto n = load_binary_data(keys, -1, path);
  parlay::sort_inplace(parlay::make_slice(keys, keys + n));
  n = unique(keys, keys + n) - keys;
  entries.resize(n);
  parlay::parallel_for(0, n, [&](size_t i) {
    entries[i].first = keys[i];
    entries[i].second = i;
  });
  cout << "Dataset loaded" << endl;
  cout << "# of entries: " << entries.size() << endl;
  delete[] keys;
  return entries;
}

const vector<string> index_names = {
    "empty",    "lipp",   "finedex",     "sali",
    "btreeolc", "artolc", "fh_index_ro", "xindex",
};

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  cout << "\nStart new benchmark test" << endl;
  cout << "Index: " << FLAGS_index << endl;
  cout << "Dataset: " << FLAGS_dataset << endl;
  assert(FLAGS_round > 1);

  vector<pair<uint64_t, uint64_t>> entries = LoadEntries(FLAGS_dataset);
  auto n = entries.size();

  cout << "Start bulk_load" << endl;
  auto index = get_index<uint64_t, uint64_t>(FLAGS_index);
  index->bulk_load(entries.data(), n);
  cout << "End Bulk_load" << endl;

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
    if (parlay::any_of(res, [&](int x) { return x == 0; }))
      good = false;

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

  return 0;
}
