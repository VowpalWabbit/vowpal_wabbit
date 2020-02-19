import sys
import getopt
import math
import queue

class Sanitize:

  def __init__(self, source_file):
    self.source_file_name = source_file

  def process(self):
    file_line_queue = queue.Queue()
    source_file = open(self.source_file_name,"r")

    source_line = ""
    comment_line_count = 0

    for (source_line) in source_file:

      file_line_queue.put(source_line)

      if file_line_queue.qsize() < 1:
        continue

      if "Copyright" in source_line:
        comment_line_count = 5

      if comment_line_count <= 0:
        print(file_line_queue.get(),end='')
      else:
        file_line_queue.get()
        comment_line_count -= 1

    while file_line_queue.qsize() > 0:
      print(file_line_queue.get(),end='')


if __name__ == "__main__":
  source_file = "source.cc"

  # Parse options - get predict and data file names
  args = sys.argv[1:]
  opts, args = getopt.getopt(args, "s:",["source="])
  for opt, arg in opts:
    if opt in ('-s', '--source'):
      source_file = arg

  # Print join lines to stdout
  fileJoiner = Sanitize(source_file)
  fileJoiner.process()
