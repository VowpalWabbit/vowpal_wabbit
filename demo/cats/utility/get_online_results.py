import sys
import getopt
from confidence_interval import ConfidenceInterval


def nextword(target, source):
    for i, w in enumerate(source):
        if w == target:
            return source[i + 1]


class LossStructOn:
    def __init__(
        self, model, n, h, loss, time, max_cost, nb_examples, ci_lower, ci_upper
    ):
        self.model = model
        self.n = n
        self.h = h
        self.loss = loss
        self.time = time
        self.max_cost = max_cost
        self.nb_examples = nb_examples
        self.ci_lower = ci_lower
        self.ci_upper = ci_upper


class EvaluatorOnline:
    def __init__(self, file_name, alpha, quiet):
        self.file_name = file_name
        self.conf_alpha = alpha
        self.costs = []
        self.best_cats = LossStructOn("cats", 0, 0, sys.float_info.max, 0, 0, 0, 0, 0)
        self.best_disc_tree = LossStructOn(
            "disc_tree", 0, 0, sys.float_info.max, 0, 0, 0, 0, 0
        )
        self.best_disc_linear = LossStructOn(
            "disc_linear", 0, 0, sys.float_info.max, 0, 0, 0, 0, 0
        )
        self.max_time = 0.0
        self.quiet = quiet

    def eval(self):
        data_file = open(self.file_name, "r")
        line = data_file.readline()

        while line:
            # Get data
            if line.find("CATS-online") != -1:
                self.costs.append(
                    LossStructOn("cats", 0, 0, sys.float_info.max, 0, 0, 0, 0, 0)
                )
            elif line.find("Discretized-Tree-online") != -1:
                self.costs.append(
                    LossStructOn("disc_tree", 0, 0, sys.float_info.max, 0, 0, 0, 0, 0)
                )
            elif line.find("Discretized-Linear-online") != -1:
                self.costs.append(
                    LossStructOn("disc_linear", 0, 0, sys.float_info.max, 0, 0, 0, 0, 0)
                )

            elif line.find("timeout") != -1:
                s1 = line.split()
                self.max_time = float(nextword("timeout", s1))

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

            elif line.find("Max Cost=") != -1:
                separator_position = len("Max Cost=")
                self.costs[len(self.costs) - 1].max_cost = float(
                    line[separator_position:]
                )

            elif line.find("number of examples") != -1:
                s1 = line.split()
                self.costs[len(self.costs) - 1].nb_examples = int(nextword("=", s1))

            elif line.find("average loss") != -1:
                s1 = line.split()
                self.costs[len(self.costs) - 1].loss = float(nextword("=", s1))

            elif line.find("real") != -1:
                s1 = line.split()
                self.costs[len(self.costs) - 1].time = float(nextword("real", s1))

            line = data_file.readline()

        self.get_best_loss()

        self.saveConfidenceIntervals(self.best_cats)
        self.saveConfidenceIntervals(self.best_disc_tree)
        self.saveConfidenceIntervals(self.best_disc_linear)

        if not self.quiet:
            self.printAllResults()

            print("max_time = ", self.max_time)

            self.printBestResults(self.best_cats)
            self.printBestResults(self.best_disc_tree)
            self.printBestResults(self.best_disc_linear)

        self.find_error()

    def return_loss(self, model):
        if model == "cats":
            return self.best_cats.loss, self.best_cats.ci_lower, self.best_cats.ci_upper
        elif model == "disc_tree":
            return (
                self.best_disc_tree.loss,
                self.best_disc_tree.ci_lower,
                self.best_disc_tree.ci_upper,
            )
        elif model == "disc_linear":
            return (
                self.best_disc_linear.loss,
                self.best_disc_linear.ci_lower,
                self.best_disc_linear.ci_upper,
            )

    def return_all(self, model):
        n_ = []
        h_ = []
        loss_ = []
        time_ = []
        for c in self.costs:
            if c.model == model:
                if c.loss < 1:
                    loss_.append(c.loss)
                    time_.append(c.time)
                    n_.append(c.n)
                    h_.append(c.h)
        return loss_, time_, n_, h_

    def get_best_loss(self):
        for c in self.costs:
            if c.model == "cats":
                if c.loss < self.best_cats.loss:
                    self.best_cats = c
            elif c.model == "disc_tree":
                if c.loss < self.best_disc_tree.loss:
                    self.best_disc_tree = c
            elif c.model == "disc_linear":
                if c.loss < self.best_disc_linear.loss:
                    self.best_disc_linear = c

    def saveConfidenceIntervals(self, cost):
        if cost.max_cost != 0:
            cost.ci_lower, cost.ci_upper = ConfidenceInterval.calculate(
                cost.nb_examples, cost.loss, cost.max_cost, self.conf_alpha
            )

    def getTime(self, model, n, hp, h, mode):  # assumes costs is soreted wrt hp and n
        times = []
        if mode == "hp":
            n_ = []
            for c in self.costs:
                if c.model == model:
                    if c.h == hp:
                        times.append(c.time)
                        n_.append(c.n)
            return times, n_

        elif mode == "h":
            n_ = []
            for c in self.costs:
                if c.model == model:
                    if (c.h / c.n) == h:
                        times.append(c.time)
                        n_.append(c.n)
            return times, n_

        elif mode == "n":
            h_ = []
            for c in self.costs:
                if c.model == model:
                    if c.n == n:
                        times.append(c.time)
                        h_.append(c.h)
            return times, h_

    def printAllResults(self):
        for cost in self.costs:
            print(
                "model, n, h, loss, time = {0}, {1}, {2}, {3}, {4}".format(
                    cost.model, cost.n, cost.h, cost.loss, cost.time
                )
            )

    def printBestResults(self, cost):
        print(
            "model, n, h, loss, time = {0}, {1}, {2}, {3}, {4}".format(
                cost.model, cost.n, cost.h, cost.loss, cost.time
            )
        )
        print("C.I. = {0}, {1}".format(cost.ci_lower, cost.ci_upper))

    def find_error(self):
        for c in self.costs:
            if c.loss == sys.float_info.max:
                if c.time < self.max_time:
                    print("error in model={0}, n={1}, h={2}".format(c.model, c.n, c.h))


if __name__ == "__main__":
    namee = "BNG_cpu_act"
    data_file = "../../results/" + namee + "_online_validation.txt"
    alpha = 0.05
    model = "cats"
    quiet = False

    # Parse options - get predict and data file names
    args = sys.argv[1:]
    opts, args = getopt.getopt(
        args, "d:a:r:q", ["data_file=", "alpha=", "return_model=", "quiet"]
    )
    for opt, arg in opts:
        if opt in ("-d", "--data_file"):
            data_file = arg
        elif opt in ("-a", "--alpha"):
            alpha = float(arg)
        elif opt in ("-r", "--return_model"):
            model = arg
        elif opt in ("-q", "--quiet"):
            quiet = True

    # Print join lines to stdout
    fileJoiner = EvaluatorOnline(data_file, alpha, quiet)
    returnValue = fileJoiner.eval()
    print(fileJoiner.return_loss(model))
    print(fileJoiner.getTime("disc_linear", 0, 0, 0, "hp"))
