import sys
import getopt
from scipy.stats import beta

class ConfidenceInterval:

  def calculate(N, avg_loss, max_weighted_cost, alpha=0.05):
    """
    input:
      N = number of total data points
      avg_loss = average cost of the batch
      max_weighted_cost = max cost during the batch
    """
    sum_loss = avg_loss * N
    success_count = sum_loss / max_weighted_cost
    n = N / max_weighted_cost
    lower = beta.ppf(alpha/2, success_count, n - success_count + 1)
    upper = beta.ppf(1 - alpha/2, success_count + 1, n - success_count)
    return lower,upper

if __name__ == "__main__":
  # Parse options - get predict and data file names
  args = sys.argv[1:]
  opts, args = getopt.getopt(args, "N:l:m,a",["num_samples=", "loss_avg=", "max_weighted_cost=", "alpha="])
  for opt, arg in opts:
    if opt in ('-N', '--num_samples'):
      N = float(arg)
    elif opt in ('-l', '--loss_avg'):
      avg_loss = float(arg)
    elif opt in ('-m', '--max_weighted_cost'):
      max_weighted_cost = float(arg)
    elif opt in ('-a', '--alpha'):
      alpha = float(arg)

  print("Args: N=",N,", avg_loss=", avg_loss, ", max_weighted_cost=", max_weighted_cost, ", alpha=", alpha)
  print("Returns: ", ConfidenceInterval.calculate(N,avg_loss,max_weighted_cost,alpha))

"""
var success_count = num / max_weighted_cost;
var n = N / max_weighted_cost;
return {l: jStat.beta.inv(alpha/2, success_count, n - success_count + 1),
 u: jStat.beta.inv(1 - alpha/2, success_count + 1, n - success_count)};

where:

N = total number of data
num =  (avg. loss)*N = summation of all loss
max_weighted_cost  =  max abs of loss
alpha = 0.05
"""