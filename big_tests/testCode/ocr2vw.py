#!/usr/bin/env python
# convert letter.data to letter.vw


def read_letter_names(fn):
    ret = list()
    with open(fn) as ins:
        for line in ins:
            ret.append(line.rstrip())
    print("Read %d names from %s" % (len(ret), fn))
    return ret


def find_pixel_start(names):
    for i in range(len(names)):
        if names[i].startswith("p_"):
            return i
    raise ValueError("No pixel data", names)


def data2vw(ifn, train, test, names):
    lineno = 0
    trainN = 0
    testN = 0
    if ifn.endswith(".gz"):
        import gzip

        iopener = lambda f: gzip.open(f, "rt")
    else:
        iopener = open
    id_pos = names.index("id")
    letter_pos = names.index("letter")
    pixel_start = find_pixel_start(names)
    with iopener(ifn) as ins, open(train, "w") as trainS, open(test, "w") as testS:
        for line in ins:
            lineno += 1
            vals = line.rstrip().split("\t")
            if len(vals) != len(names):
                raise ValueError("Bad field count", len(vals), len(names), vals, names)
            char = vals[letter_pos]
            if len(char) != 1:
                raise ValueError("Bad letter", char)
            if lineno % 10 == 0:
                testN += 1
                outs = testS
            else:
                trainN += 1
                outs = trainS
            outs.write(
                "%d 1 %s-%s|Pixel" % (ord(char) - ord("a") + 1, char, vals[id_pos])
            )
            for i in range(pixel_start, len(names)):
                if vals[i] != "0":
                    outs.write(" %s:%s" % (names[i], vals[i]))
            outs.write("\n")
    print(
        "Read %d lines from %s; wrote %d lines into %s and %d lines into %s"
        % (lineno, ifn, trainN, train, testN, test)
    )


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Convert letters.data to VW format")
    parser.add_argument("input", help="path to letter.data[.gz]")
    parser.add_argument("names", help="path to letter.names")
    parser.add_argument("train", help="VW train file location (90%)")
    parser.add_argument("test", help="VW test file location (10%)")
    args = parser.parse_args()
    data2vw(args.input, args.train, args.test, read_letter_names(args.names))
