import sys
import getopt
import math

class SRM:

  def __init__(self, data_file_name, predict_file_name, min_val, max_val, k, h):
    self.data_file_name = data_file_name
    self.predict_file_name = predict_file_name
    self.min_val = min_val
    self.max_val = max_val
    self.hh = (h * (max_val - min_val) / k)

  def calc(self):

    data_file = open(self.data_file_name,"r")
    predict_file = open(self.predict_file_name,"r")

    N = 0
    _loss_acc = 0.
    _loss_max = float("-inf")
    
    for (data_line, predict_line) in zip(data_file, predict_file):
      # Get data
      pred = self.get_predict_action(predict_line)
      logd, L_s, P_s = self.get_logged_data(data_line)
      # Compute losses
      if(math.isclose(pred,logd,abs_tol=self.hh)):
        H_new = min(self.max_val, logd + self.hh) - max(self.min_val, logd - self.hh)  
        _loss_acc += (L_s / (P_s * H_new))
      N += 1
      if(N%10000 == 0):
        print('.',end='',flush=True)
      
    _loss = _loss_acc / float(N)
    
    print('.')
    print("_loss=",_loss,", _loss_acc=",_loss_acc,", N=",N,)


  def get_logged_data(self, data_line):
    sep1 = data_line.find(':')
    a = data_line[:sep1]
    sep2 = data_line.find(':', sep1+1)
    l = data_line[sep1+1:sep2]
    p = data_line[sep2+1:]
    return (float(a), float(l), float(p))

  def get_predict_action(self, pred_line):
    separator_position = pred_line.find(':')
    #print(pred_line[:separator_position - 1 ])
    return float(pred_line[:separator_position])

if __name__ == "__main__":
  predict_file = "predict.txt"
  data_file = "data.txt"
  max_val = 100.0
  min_val = 0.0
  k = 4.0
  h = 1.0

  # Parse options - get predict and data file names
  args = sys.argv[1:]
  opts, args = getopt.getopt(args, "p:d:m:i:k:h",["predict_file=", "data_file=", "max=", "min=", "number_actions=", "bandwidth="])
  for opt, arg in opts:
    if opt in ('-p', '--predict_file'):
      predict_file = arg
    elif opt in ('-d', '--data_file'):
      data_file = arg
    elif opt in ('-m', '--max'):
      max_val = float(arg)
    elif opt in ('-i', '--min'):
      min_val = float(arg)
    elif opt in ('-k', '--number_actions'):
      k = float(arg)
    elif opt in ('-h', '--bandwidth'):
      h = float(arg)

  # Print join lines to stdout
  srm_ = SRM(data_file, predict_file, min_val, max_val, k, h)
  srm_.calc()
