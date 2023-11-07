import sys
import getopt
import math


class SRM:
    def __init__(self, data_file_name, predict_file_name, min_val, max_val, k, h):
        self.data_file_name = data_file_name
        self.predict_file_name = predict_file_name
        self.min_val = min_val
        self.max_val = max_val
        self.hh = h * (max_val - min_val) / k

    def calc(self):
        data_file = open(self.data_file_name, "r")
        predict_file = open(self.predict_file_name, "r")

        N = 0
        _loss_acc = 0.0

        for data_line, predict_line in zip(data_file, predict_file):
            # Get data
            pred = self.get_predict_action(predict_line)
            logd, L_s, P_s = self.get_logged_data(data_line)
            #       L_s = 1
            # Compute losses
            if math.isclose(pred, logd, abs_tol=self.hh):
                H_new = min(self.max_val, logd + self.hh) - max(
                    self.min_val, logd - self.hh
                )
                _loss_acc += L_s / (P_s * H_new)
            N += 1

        _loss = _loss_acc / float(N)

        # print("_loss=",_loss,", _loss_acc=",_loss_acc,", N=",N,)

        return _loss, N

    def get_logged_data(self, data_line):
        sep1 = data_line.find(":")
        a = data_line[:sep1]
        sep2 = data_line.find(":", sep1 + 1)
        l = data_line[sep1 + 1 : sep2]
        p = data_line[sep2 + 1 :]
        return (float(a), float(l), float(p))

    def get_predict_action(self, pred_line):
        separator_position = pred_line.find(",")
        return float(pred_line[:separator_position])


if __name__ == "__main__":
    # Example:
    usage = "\nUsage: python3 srm.py --p <prediction_file> --d <action_cost_probability_file> -m <max_action> -i <min_action> -k <partitions> --bandwidth <bandwidth>"
    usage += "\nExample: python3 srm.py --p results/friday_2_2048_4.ap --d results/friday_2.acp -m 23959 -i 185 -k 2048 --bandwidth 4"

    predict_file = "predict.txt"
    data_file = "data.txt"
    max_val = 100.0
    min_val = 0.0
    k = 4.0
    h = 1.0

    # Parse options - get predict and data file names
    args = sys.argv[1:]
    try:
        opts, args = getopt.getopt(
            args,
            "p:d:m:i:k:h:?",
            [
                "predict_file=",
                "data_file=",
                "max=",
                "min=",
                "number_actions=",
                "bandwidth=",
                "help",
            ],
        )
    except getopt.GetoptError:
        print(usage)
        sys.exit()

    for opt, arg in opts:
        if opt in ("-p", "--predict_file"):
            predict_file = arg
        elif opt in ("-d", "--data_file"):
            data_file = arg
        elif opt in ("-m", "--max"):
            max_val = float(arg)
        elif opt in ("-i", "--min"):
            min_val = float(arg)
        elif opt in ("-k", "--number_actions"):
            k = float(arg)
        elif opt in ("-h", "--bandwidth"):
            h = float(arg)
        elif opt in ("-?", "--help"):
            print(usage)
            sys.exit()

    srm_ = SRM(data_file, predict_file, min_val, max_val, k, h)
    try:
        l, n = srm_.calc()
        print("srm =", l)
    except Exception as err:
        print(usage)
        print()
        print(type(err).__name__, err, "\n")
