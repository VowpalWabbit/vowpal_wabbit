import sys
import getopt
import os
import csv
import pandas as pd
import math

class PreProcessCSVData:

  def get_file_root(self, csv_file_name):
    return os.path.splitext(csv_file_name)[0]

  def process(self, csv_file_name):
    print('csv_file_name = ', csv_file_name)
    dat_file_name = self.get_file_root(csv_file_name)+'.dat'
    print('dat_file_name = ', dat_file_name)
    with open(dat_file_name, mode='w') as dat_file:
      dat_writer = csv.writer(dat_file, delimiter=' ', quotechar='"', quoting=csv.QUOTE_MINIMAL)
      with open(csv_file_name) as csvfile:
        readCSV = csv.reader(csvfile, delimiter=',')
        minv = sys.float_info.max
        maxv = sys.float_info.min
        next(readCSV)  # Skip header
        row_num = 0
        for row in readCSV:
          row.insert(1,'|') # add feature seperator as a column for vw
          if ('zurich' in csv_file_name):
            del row[7] # if this is the zurich dataset, remove 7th column
          else: # for all other datasets, swap first and last columns
            tmp = row[len(row)-1]
            row[len(row) - 1] = row[0]
            row[0] = tmp
          if (row[0] == '|'):
            print(row_num)
            break
          dat_writer.writerow(row)
          minv = min(minv, float(row[0]))
          maxv = max(maxv, float(row[0]))
          row_num += 1
    print('row_num = ', row_num)
    print('min_value = ', minv)
    print('max_value = ', minv)
    percent = 0.80
    df = pd.read_csv(dat_file_name, header=None)
    ds = df.sample(frac=1)
    ds_train = ds[0:math.floor(df.size*percent)]
    ds_test = ds[math.floor(df.size*percent):]
    ds.to_csv(dat_file_name, header = False, index=False)
    ds_train.to_csv(self.get_file_root(csv_file_name)+'_train.dat', header = False, index=False)
    ds_test.to_csv(self.get_file_root(csv_file_name)+'_test.dat', header = False, index=False)

if __name__ == "__main__":
  csv_file_name = "data.csv"
  # Parse options - get predict and data file names
  args = sys.argv[1:]
  opts, args = getopt.getopt(args, "c:",["csv_file="])
  for opt, arg in opts:
    if opt in ('-c', '--csv_file'):
      csv_file_name = arg

  # Print join lines to stdout
  processor = PreProcessCSVData()
  processor.process(csv_file_name)
