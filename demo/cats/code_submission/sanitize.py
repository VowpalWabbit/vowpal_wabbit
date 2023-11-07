import sys
import getopt
import queue


class Sanitize:
    S_CODE = 0
    S_COMMENT = 1
    SANITIZE_WORDS = [
        "Copyright",
        "Microsoft",
        "Yahoo",
        "Maryam",
        "Majzoubi",
        "Chicheng",
        "Zhang",
        "Rajan",
        "Chari",
        "Akshay",
        "Krishnamurthy",
        "Alex",
        "Slivkins",
        "John",
        "Langford",
    ]

    # Constructor
    def __init__(self, source_file):
        self.source_file_name = source_file
        self.S_CURR = Sanitize.S_CODE
        self.file_line_queue = queue.Queue()
        self.comment_has_copyright = False

    # Main class method
    def process(self):
        source_file = open(self.source_file_name, "r")
        self.S_CURR = Sanitize.S_CODE
        source_line = ""

        for source_line in source_file:
            if self.S_CURR == Sanitize.S_CODE:
                self.process_input_S_CODE(source_line)
            elif self.S_CURR == Sanitize.S_COMMENT:
                self.process_input_S_COMMENT(source_line)

        while self.file_line_queue.qsize() >= 1:
            print(self.file_line_queue.get(), end="")

    # State machine is in CODE state.  i.e. in the middle of code segment
    def process_input_S_CODE(self, source_line):
        if "/*" in source_line:
            self.file_line_queue.put(source_line)
            self.S_CURR = Sanitize.S_COMMENT
            if any(x in source_line for x in Sanitize.SANITIZE_WORDS):
                self.comment_has_copyright = True
        else:
            print(source_line, end="")

    # State machine is in COMMENT state.  i.e. in the middle of comment segment
    def process_input_S_COMMENT(self, source_line):
        self.file_line_queue.put(source_line)

        if any(x in source_line for x in Sanitize.SANITIZE_WORDS):
            self.comment_has_copyright = True

        if "*/" in source_line:
            if self.comment_has_copyright:
                self.clear_queue()
            else:
                self.print_queue()
            self.comment_has_copyright = False
            self.S_CURR = Sanitize.S_CODE

    # Remove all items from the queue
    def clear_queue(self):
        self.file_line_queue = queue.Queue()

    #  Print and drain all items from the queue
    def print_queue(self):
        while self.file_line_queue.qsize() >= 1:
            print(self.file_line_queue.get(), end="")


# Main()
if __name__ == "__main__":
    source_file = "source.cc"

    # Parse options - get predict and data file names
    args = sys.argv[1:]
    opts, args = getopt.getopt(args, "s:", ["source="])
    for opt, arg in opts:
        if opt in ("-s", "--source"):
            source_file = arg

    # Print join lines to stdout
    fileJoiner = Sanitize(source_file)
    fileJoiner.process()
