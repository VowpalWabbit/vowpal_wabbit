import sys
import getopt
import math


class PredictDataJoiner_ap:
    def __init__(
        self,
        data_file_name,
        predict_file_name,
        min_val,
        max_val,
        zero_one_width,
        loss_type_return,
    ):
        self.data_file_name = data_file_name
        self.predict_file_name = predict_file_name
        self.min_val = min_val
        self.max_val = max_val
        self.zero_one_width = zero_one_width
        self.loss_type_return = loss_type_return

    def join(self):
        data_file = open(self.data_file_name, "r")
        predict_file = open(self.predict_file_name, "r")

        range_val = self.max_val - self.min_val
        zero_one_h = abs(range_val) * self.zero_one_width
        N = 0
        abs_loss_acc = 0.0
        sqr_loss_acc = 0.0
        zero_one_loss_acc = 0.0
        abs_loss_max = float("-inf")
        sqr_loss_max = float("-inf")
        max_found = float("-inf")
        min_found = float("inf")
        for data_line, predict_line in zip(data_file, predict_file):
            # Get data
            act = self.get_action(predict_line)
            reg = self.get_regression_val(data_line)
            # Compute losses
            abss = abs((reg - act) / range_val)
            if abss > abs_loss_max:
                abs_loss_max = abss
            abs_loss_acc += abss
            sqrr = ((reg - act) / range_val) ** 2
            if sqrr > sqr_loss_max:
                sqr_loss_max = sqrr
            sqr_loss_acc += sqrr
            if not math.isclose(act, reg, abs_tol=zero_one_h):
                zero_one_loss_acc += 1.0
            N += 1
            if reg > max_found:
                max_found = reg
            if reg < min_found:
                min_found = reg

        abs_loss = abs_loss_acc / float(N)
        sqr_loss = sqr_loss_acc / float(N)
        zero_one_loss = zero_one_loss_acc / float(N)

        if self.loss_type_return == "abs":
            return abs_loss, abs_loss_max, N
        elif self.loss_type_return == "sqr":
            return sqr_loss, sqr_loss_max, N
        elif self.loss_type_return == "zerone":
            return zero_one_loss, 1, N

    def get_regression_val(self, data_line):
        data_line = data_line.lstrip()
        ca_end_position = 0
        if data_line.startswith("ca"):
            ca_end_position = 2
        separator_position = data_line.find("|")
        return float(data_line[ca_end_position:separator_position])

    def get_action(self, pred_line):
        separator_position = pred_line.find(",")
        return float(pred_line[:separator_position])


if __name__ == "__main__":
    predict_file = "predict.txt"
    data_file = "data.txt"
    max_val = 100.0
    min_val = 0.0
    zero_one_width = 0.1
    loss_return = "abs"

    # Parse options - get predict and data file names
    args = sys.argv[1:]
    opts, args = getopt.getopt(
        args,
        "p:d:m:i:z:l",
        ["predict_file=", "data_file=", "max=", "min=", "zero_one=", "loss_return="],
    )
    for opt, arg in opts:
        if opt in ("-p", "--predict_file"):
            predict_file = arg
        elif opt in ("-d", "--data_file"):
            data_file = arg
        elif opt in ("-m", "--max"):
            max_val = float(arg)
        elif opt in ("-i", "--min"):
            min_val = float(arg)
        elif opt in ("-z", "--zero_one"):
            zero_one_width = float(arg)
        elif opt in ("-l", "--loss_return"):
            loss_return = arg

    # Print join lines to stdout
    fileJoiner = PredictDataJoiner_ap(
        data_file, predict_file, min_val, max_val, zero_one_width, loss_return
    )
    loss, max_l, nb_ex = fileJoiner.join()
    print("test_loss =", loss)
    print("max_loss =", max_l)
    print("nb_examples =", nb_ex)
