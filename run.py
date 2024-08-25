import os
import subprocess

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))

def run_index(test_type, index, dataset):
  print(f'Running {test_type} {index} {dataset}')
  output_file = f'./log/0825-{test_type}.txt'
  command = f'./build/test -dataset={dataset} -index={index} -test_type={test_type} -round=5'
  subprocess.call(f'numactl -i all {command} >> {output_file}', shell=True)

def get_datasets():
  yield 'uniform'
  datasets = [
    'books',
    'covid',
    'fb',
    'genome',
    'history',
    'libio',
    'osm',
    'planet',
    'stack',
    'wise',
    'wiki_ts_200M_uint64',
    'books_800M_uint64',
    'osm_cellids_800M_uint64',
  ]
  for dataset in datasets:
    yield f'/data/xding9001/li/{dataset}'

def get_index_names():
  return [
    # 'naive',
    'fh_index_ro',
    'fh_index_rw',
    'pgm',
    'alex',
    'lipp',
    'xindex',
    'sali',
    'verlib_arttree',
    'verlib_btree',
    'fast',
  ]

def main():
  subprocess.call('mkdir -p log', shell=True)
  for test_type in ['ro']:
    for index in get_index_names():
      for dataset in get_datasets():
        run_index(test_type, index, dataset)

if __name__ == '__main__':
  main()
