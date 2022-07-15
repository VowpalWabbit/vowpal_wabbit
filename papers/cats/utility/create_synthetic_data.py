import numpy as np
import math
import gzip
import os
import pandas as pd
import math
import sys
import getopt


class SyntheticData:
    def __init__(self):
        self.n = 1000000  # number of samples
        self.d = 5  # dimensionality of data
        self.sig = 0.2  # (Gaussian) noise rate
        self.name = "ds_{}".format(self.n)  # ds_1000000
        self.fname = "ds_synthetic_{}_{}.vw.gz".format(
            self.n, self.d
        )  # ds_synthetic_1000000_5.vw.gz

    def create(self, save_to_path):
        """
        Synthetic dataset 1: w is roughly a unit vector, x_i's are standard normal
        (Y is in the range [-10, +10] with high probability)
        """
        w = np.random.randn(self.d) / math.sqrt(self.d)
        X = np.random.randn(self.n, self.d) + self.d
        Y = X.dot(w) + self.sig * np.random.randn(self.n)

        min_file = open(save_to_path + self.name + ".min", "w")
        min_file.write(str(Y.min()))
        print("min_value = ", Y.min(), flush=True)

        max_file = open(save_to_path + self.name + ".max", "w")
        max_file.write(str(Y.max()))
        print("max_value = ", Y.max(), flush=True)

        self.save_vw_reg_dataset(X, Y, save_to_path)
        percent = 0.80
        ds = pd.read_csv(save_to_path + self.fname, header=None)
        ds_train = ds[0 : math.floor(ds.size * percent)]
        ds_test = ds[math.floor(ds.size * percent) :]

        ds.to_csv(save_to_path + self.name + ".dat", header=False, index=False)
        ds_train.to_csv(
            save_to_path + self.name + "_train.dat", header=False, index=False
        )
        ds_test.to_csv(
            save_to_path + self.name + "_test.dat", header=False, index=False
        )

    """
  Saving the data matrix in vw format
  """

    def save_vw_reg_dataset(self, X, Y, ds_dir):
        self.n, self.d = np.shape(X)
        with gzip.open(os.path.join(ds_dir, self.fname), "w") as f:
            for i in range(self.n):
                st = "{} | {}\n".format(
                    Y[i],
                    " ".join(
                        "{}:{:.6f}".format(j, val)
                        for j, val in zip(range(1, self.d + 1), X[i].data)
                    ),
                )
                f.write(st.encode())


if __name__ == "__main__":
    save_to_path = "./test/train-sets/regression/"

    args = sys.argv[1:]
    opts, args = getopt.getopt(args, "p:", ["save_to_path="])
    for opt, arg in opts:
        if opt in ("-p", "--save_to_path"):
            save_to_path = arg

    synthetic = SyntheticData()
    synthetic.create(save_to_path)
