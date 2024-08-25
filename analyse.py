
def get_attr(lines, attr, default='0'):
  for line in lines:
    line = line.strip()
    if line.startswith(attr):
      return line.split(' ')[-1]
  return default

def main():
  f = open('./log/0825-ro.txt', 'r')
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
    mops = float(get_attr(lines, 'Single Read Average Mops:'))
    mem = float(get_attr(lines, 'Index memory usage:')) / 1000000000
    # print(mops, mem)

    if index not in res:
      res[index] = [index]
    res[index].append(str(mops))
    if dataset not in datasets:
      datasets.append(dataset)
  print(','.join(datasets))
  for k,v in res.items():
    print(','.join(v))


if __name__ == '__main__':
  main()
