import sys
import getopt
import math
from confidence_interval import ConfidenceInterval


def nextword(target, source):
    for i, w in enumerate(source):
        if w == target:
            return source[i + 1]


class LossStructOff:
    def __init__(
        self,
        n,
        h,
        srm,
        srm_penalty,
        test_loss,
        max_loss,
        nb_examples,
        ci_lower,
        ci_upper,
    ):
        self.n = n
        self.h = h
        self.srm = srm
        self.srm_penalty = srm_penalty
        self.test_loss = test_loss
        self.max_loss = max_loss
        self.nb_examples = nb_examples
        self.ci_lower = ci_lower
        self.ci_upper = ci_upper


class EvaluatorOffline:
    def __init__(self, srm_file_name, test_file_name, delta, alpha, quiet):
        self.srm_file_name = srm_file_name
        self.test_file_name = test_file_name
        self.costs = []
        self.initial = LossStructOff(0, 0, 0, sys.float_info.max, 0, 0, 0, 0, 0)
        self.optimized = LossStructOff(0, 0, 0, sys.float_info.max, 0, 0, 0, 0, 0)
        self.delta = delta
        self.conf_alpha = alpha
        self.quiet = quiet
        self.pmin = 0.05  # epsilon = 5%

    def eval(self):
        data_file = open(self.srm_file_name, "r")
        line = data_file.readline()

        count = 0
        while line:
            # Get data
            if line.find("CATS-offline") != -1:
                self.costs.append(LossStructOff(0, 0, 0, 0, 0, 0, 0, 0, 0))
                count += 1
            elif line.find("n = ") != -1:
                separator_position = len("n = ")
                separator_position_end = line.find("\n")
                self.costs[len(self.costs) - 1].n = float(
                    line[separator_position:separator_position_end]
                )

            elif line.find("h = ") != -1:
                separator_position = len("h = ")
                separator_position_end = line.find("\n")
                self.costs[len(self.costs) - 1].h = float(
                    line[separator_position:separator_position_end]
                )

            elif line.find("srm") != -1:
                s1 = line.split()
                self.costs[len(self.costs) - 1].srm = float(nextword("=", s1))

            line = data_file.readline()

        data_file = open(self.test_file_name, "r")
        line = data_file.readline()

        count = 0
        while line:
            # Get data
            if line.find("CATS-offline") != -1:
                count += 1

            elif line.find("n = ") != -1:
                separator_position = len("n = ")
                separator_position_end = line.find("\n")
                if self.costs[count - 1].n != float(
                    line[separator_position:separator_position_end]
                ):
                    print("error: n is not matched")

            elif line.find("h = ") != -1:
                separator_position = len("h = ")
                separator_position_end = line.find("\n")
                if self.costs[count - 1].h != float(
                    line[separator_position:separator_position_end]
                ):
                    print("error: h is not matched")

            elif line.find("test_loss") != -1:
                s1 = line.split()
                self.costs[count - 1].test_loss = float(nextword("=", s1))

            elif line.find("max_loss") != -1:
                s1 = line.split()
                self.costs[count - 1].max_loss = float(nextword("=", s1))

            elif line.find("nb_examples") != -1:
                s1 = line.split()
                self.costs[count - 1].nb_examples = float(nextword("=", s1))

            line = data_file.readline()

        self.calc_srm_penalty()
        self.get_optimized()

        self.saveConfidenceIntervals(self.initial)
        self.saveConfidenceIntervals(self.optimized)

        if not self.quiet:
            self.printAllResults()
            print("\ninitial model:")
            self.printResults(self.initial)
            print("optimized model:")
            self.printResults(self.optimized)

    def return_loss(self, model):
        if model == "init":
            return self.initial.test_loss, self.initial.ci_lower, self.initial.ci_upper
        elif model == "opt":
            return (
                self.optimized.test_loss,
                self.optimized.ci_lower,
                self.optimized.ci_upper,
            )

    def calc_srm_penalty(self):
        for c in self.costs:
            # c.srm_penalty = c.srm + math.sqrt(c.n * self.delta /(c.h * self.pmin * c.nb_examples)) # todo: fix
            c.srm_penalty = (
                c.srm
                + math.sqrt(
                    c.n * self.delta * c.srm / (c.h * self.pmin * c.nb_examples)
                )
                + (c.n * self.delta / (c.h * self.pmin * c.nb_examples))
            )  # todo: fix

    def get_optimized(self):
        if (
            self.costs[0].n != 4 or self.costs[0].h != 1
        ):  # todo: if changed to depth parameters
            print("error in finding initial model")
        self.initial = self.costs[0]
        for c in self.costs:
            if c.srm_penalty < self.optimized.srm_penalty:
                self.optimized = c

    def saveConfidenceIntervals(self, cost):
        cost.ci_lower, cost.ci_upper = ConfidenceInterval.calculate(
            cost.nb_examples, cost.test_loss, cost.max_loss, self.conf_alpha
        )

    def printAllResults(self):
        for cost in self.costs:
            print(
                "n, h, srm, srm_penalty, test_loss = {0}, {1}, {2}, {3}, {4}".format(
                    cost.n, cost.h, cost.srm, cost.srm_penalty, cost.test_loss
                )
            )
            print("C.I. = {0}, {1}".format(cost.ci_lower, cost.ci_upper))

    def printResults(self, cost):
        print(
            "n, h, srm, srm_penalty, test_loss = \n {0}, {1}, {2}, {3}, {4}".format(
                cost.n, cost.h, cost.srm, cost.srm_penalty, cost.test_loss
            )
        )
        print("C.I. = {0}, {1}".format(cost.ci_lower, cost.ci_upper))


if __name__ == "__main__":
    srm_file = "../../results/black_friday_offline_srm.txt"
    test_file = "../../results/black_friday_offline_test.txt"
    delta = 1
    alpha = 0.05
    model = "init"
    quiet = False

    # Parse options - get predict and data file names
    args = sys.argv[1:]
    opts, args = getopt.getopt(
        args,
        "d:p:c:a:r:q",
        ["srm_file=", "test_file", "delta=", "alpha=", "return_model=", "quiet"],
    )
    for opt, arg in opts:
        if opt in ("-d", "--srm_file"):
            srm_file = arg
        if opt in ("-p", "--test_file"):
            test_file = arg
        elif opt in ("-c", "--delta"):
            delta = float(arg)
        elif opt in ("-a", "--alpha"):
            alpha = float(arg)
        elif opt in ("-r", "--return_model"):
            model = arg
        elif opt in ("-q", "--quiet"):
            quiet = True

    # Print join lines to stdout
    fileJoiner = EvaluatorOffline(srm_file, test_file, delta, alpha, quiet)
    returnValue = fileJoiner.eval()
    print(fileJoiner.return_loss(model))
