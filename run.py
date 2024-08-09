import os
import subprocess

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))

def run_index(index, dataset):
  print(f'Running {index} {dataset}')
  test_type = 'ro'
  num_keys = 0
  output_file = './log/scan-0808.txt'
  command = f'./build/test -dataset={dataset} -index={index} -test_type={test_type} -num_keys={num_keys}'
  subprocess.call(f'numactl -i all {command} >> {output_file}', shell=True)

def get_datasets():
  datasets = [
    'books',
    'covid',
    # 'fb',
    # 'genome',
    # 'history',
    # 'libio',
    # 'osm',
    # 'planet',
    # 'stack',
    # 'wise',
    # 'wiki_ts_200M_uint64',
    # 'books_800M_uint64',
    # 'osm_cellids_800M_uint64',
  ]
  for dataset in datasets:
    yield f'/colddata/xding9001/li/{dataset}'

def get_index_names():
  return [
    'empty',
    # 'naive',
    'fh_index_ro',
    # 'pgm',
    # 'alex',
    # 'lipp',
    # # 'finedex',
    # 'xindex',
    # 'sali',
    # 'verlib_arttree',
    # 'verlib_btree',
    # 'fast',
  ]

def main():
  subprocess.call('mkdir -p log', shell=True)
  for index in get_index_names():
    for dataset in get_datasets():
      run_index(index, dataset)

if __name__ == '__main__':
  main()
