import sys
import getopt
from scipy.stats import beta


class ConfidenceInterval:
    @staticmethod
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
        return beta(success_count + 1, n - success_count + 1).interval(
            1 - alpha
        )  # simpler than above


if __name__ == "__main__":
    # Parse options - get predict and data file names
    args = sys.argv[1:]
    opts, args = getopt.getopt(
        args, "N:l:m:a:", ["num_samples=", "avg_loss=", "max_weighted_cost=", "alpha="]
    )
    for opt, arg in opts:
        if opt in ("-N", "--num_samples"):
            N = float(arg)
        elif opt in ("-l", "--avg_loss"):
            avg_loss = float(arg)
        elif opt in ("-m", "--max_weighted_cost"):
            max_weighted_cost = float(arg)
        elif opt in ("-a", "--alpha"):
            alpha = float(arg)

    print(
        "Args: N=",
        N,
        ", avg_loss=",
        avg_loss,
        ", max_weighted_cost=",
        max_weighted_cost,
        ", alpha=",
        alpha,
    )
    print(
        "Returns: ", ConfidenceInterval.calculate(N, avg_loss, max_weighted_cost, alpha)
    )

"""
  Example:  python confidence_interval.py -N 999999 -l 0.044278 --max_weighted_cost=0.796388 --alpha=0.05

  VW Output:
  number of examples = 999999
  weighted example sum = 999999.000000
  weighted label sum = 12765158078.839020
  average loss = 0.044278
  best constant = 12765.170898
  total feature number = 15999984
  Max Cost=0.796388
"""
