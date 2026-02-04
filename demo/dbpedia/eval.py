#!/usr/bin/env python

from __future__ import print_function
import argparse
from sklearn.metrics import classification_report, confusion_matrix

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Evaluate Classification Results")
    parser.add_argument(
        "--truth", dest="truth", type=str, required=True, help="path to truth file"
    )
    parser.add_argument(
        "--pred", dest="pred", type=str, required=True, help="path to prediction file"
    )

    args = parser.parse_args()

    y_true = []
    with open(args.truth, "rb") as t:
        for line in t:
            y_true.append(int(line.split(" ", 1)[0]))

    y_pred = []
    with open(args.pred, "rb") as p:
        for line in p:
            y_pred.append(int(line.strip()))

    print(confusion_matrix(y_true, y_pred))
    print()
    print(classification_report(y_true, y_pred))
