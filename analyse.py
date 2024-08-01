
def get_attr(lines, attr):
  for line in lines:
    line = line.strip()
    if line.startswith(attr):
      return line.split(' ')[-1]

def main():
  f = open('./log/temp.txt', 'r')
  all_lines = f.readlines()
  f.close()
  ids = []
  for i in range(len(all_lines)):
    if all_lines[i].startswith('Start new benchmark test'):
      ids.append(i)
  ids.append(len(all_lines))
  res = {}
  datasets = ['']
  for i in range(len(ids) - 1):
    lines = all_lines[ids[i]:ids[i+1]]

    index = get_attr(lines, 'Index:')
    dataset = get_attr(lines, 'Dataset:').split('/')[-1]
    print(index, dataset)
    mops = float(get_attr(lines, 'Average Mops:'))
    mem = float(get_attr(lines, 'Maximum resident set size (kbytes):')) / 1000000
    print(mops, mem)
    num_fgds = 0
    if index == 'fh_index_ro':
      for line in lines:
        if line.startswith('Start key:'):
          num_fgds += 1

    if index not in res:
      res[index] = [index]
    res[index].append(str(mem))
    if dataset not in datasets:
      datasets.append(dataset)
  print(','.join(datasets))
  for k,v in res.items():
    print(','.join(v))


if __name__ == '__main__':
  main()
