import os
import subprocess

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))

def run_index(index, dataset):
  print(f'Running {index} {dataset}')
  command = f'./build/main -dataset={dataset} -index={index}'
  subprocess.call(f'/usr/bin/time -v numactl -i all {command} 1>> log/temp.txt 2>> log/temp.txt', shell=True)

def get_datasets():
  datasets = [
    'books',
    # 'covid',
    # 'fb',
    # 'genome',
    # 'history',
    # 'libio',
    # 'osm',
    # 'planet',
    # 'stack',
    # 'wise',
  ]
  for dataset in datasets:
    yield f'/colddata/xding9001/li/{dataset}'

def get_index_names():
  return [
    'empty',
    'fh_index_ro',
    'lipp',
    'finedex',
    'xindex',
    'sali',
    'artolc',
    'btreeolc',
    'fast',
  ]

def main():
  subprocess.call('mkdir -p log', shell=True)
  for index in get_index_names():
    for dataset in get_datasets():
      run_index(index, dataset)

if __name__ == '__main__':
  main()
