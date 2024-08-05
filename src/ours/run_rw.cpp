#include "../competitor/competitor.h"
#include "gflags/gflags.h"
#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "utils.h"

using namespace std;

DEFINE_string(index, "fh_index_ro", "index name");
DEFINE_string(dataset, "", "path to dataset");
DEFINE_double(num, 0, "num");

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  cout << "\nStart new benchmark test" << endl;
  cout << "Index: " << FLAGS_index << endl;
  cout << "Dataset: " << FLAGS_dataset << endl;

  auto entries = LoadEntries(FLAGS_dataset, FLAGS_num);
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

  return 0;
}
