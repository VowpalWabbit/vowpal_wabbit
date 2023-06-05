import sys
import getopt


class PredictDataJoiner_acp:
    def join(self, data_file_name, predict_file_name):
        data_file = open(data_file_name, "r")
        predict_file = open(predict_file_name, "r")

        for data_line, predict_line in zip(data_file, predict_file):
            data_line = self.strip_label(data_line)
            print("ca", predict_line.strip(), "|", data_line.strip())

    def strip_label(self, data_line):
        separator_position = data_line.find("|")
        return data_line[separator_position + 1 :]


if __name__ == "__main__":
    predict_file = "predict.txt"
    data_file = "data.txt"

    # Parse options - get predict and data file names
    args = sys.argv[1:]
    opts, args = getopt.getopt(args, "p:d", ["predict_file=", "data_file="])
    for opt, arg in opts:
        if opt in ("-p", "--predict_file"):
            predict_file = arg
        elif opt in ("-d", "--data_file"):
            data_file = arg

    # Print join lines to stdout
    fileJoiner = PredictDataJoiner_acp()
    fileJoiner.join(data_file, predict_file)
