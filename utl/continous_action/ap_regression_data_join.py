import sys
import getopt
import math

class PredictDataJoiner:

  def __init__(self, data_file_name, predict_file_name, min_val, max_val, loss_fn):
    self.data_file_name = data_file_name
    self.predict_file_name = predict_file_name
    self.min_val = min_val
    self.max_val = max_val
    self.loss_fn = loss_fn

  def join(self):

    data_file = open(self.data_file_name,"r")
    predict_file = open(self.predict_file_name,"r")

    range_val = self.max_val - self.min_val
    N = 0
    acc = 0.
    max_found = float("-inf")
    min_found = float("inf")
    for (data_line, predict_line) in zip(data_file, predict_file):
      act = self.get_action(predict_line)
      reg = self.get_regression_val(data_line)
      acc += abs(reg-act)/range_val
      N += 1
      if(N%10000 == 0):
        print('.',end='',flush=True)
      if(reg > max_found):
        max_found = reg
      if(reg < min_found):
        min_found = reg
    abs_loss = acc / float(N)
    print('.')
    print("abs_loss=",abs_loss,",max_param=",self.max_val,",min_param=",self.min_val,"acc=",acc,", N=",N,"max_found=",max_found,"min_found=",min_found)
    if(not math.isclose(self.max_val,max_found,rel_tol=.001)):
      print("ERR:Please check max param. ", self.max_val, "is not close to ", max_found)
    if(not math.isclose(self.min_val,min_found,rel_tol=.001)):
      print("ERR:Please check min param. ", self.min_val, "is not close to ", min_found)

  def get_regression_val(self, data_line):
    separator_position = data_line.find('|')
    return float(data_line[:separator_position - 1])

  def get_action(self, pred_line):
    separator_position = pred_line.find(':')
    return float(pred_line[:separator_position - 1 ])

if __name__ == "__main__":
  predict_file = "predict.txt"
  data_file = "data.txt"
  max_val = 100.0
  min_val = 0.0
  loss_fn = 0

  # Parse options - get predict and data file names
  args = sys.argv[1:]
  opts, args = getopt.getopt(args, "p:d:m:i:l",["predict_file=", "data_file=", "max=", "min=", "loss_fn="])
  for opt, arg in opts:
    if opt in ('-p', '--predict_file'):
      predict_file = arg
    elif opt in ('-d', '--data_file'):
      data_file = arg
    elif opt in ('-m', '--max'):
      max_val = float(arg)
    elif opt in ('-i', '--min'):
      min_val = float(arg)
    elif opt in ('-l', '--loss'):
      loss_fn = int(arg)


  # Print join lines to stdout
  fileJoiner = PredictDataJoiner(data_file, predict_file, min_val, max_val, loss_fn)
  fileJoiner.join()
