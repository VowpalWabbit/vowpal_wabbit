import sys
import getopt
import math
import queue

class Sanitize:

  S_CODE = 0
  S_COMMENT = 1

  def __init__(self, source_file):
    self.source_file_name = source_file
    self.S_CURR = S_CODE
    self.file_line_queue = queue.Queue()
    self.comment_has_copyright = False

  def process_input_S_CODE(self, source_line):

    if "/*" in source_line:
      file_line_queue.put(source_line)
      self.S_CURR = S_COMMENT
      if "Copyright" in source_line:
        self.comment_has_copyright = True
    else:
        print(file_line_queue.get(),end='')

  def clear_queue(self):
    self.file_line_queue = queue.Queue()

  def print_queue(self):
    while file_line_queue.qsize() > 0:
      print(file_line_queue.get(),end='')

  def process_input_S_COMMENT(self, source_line):

    if "Copyright" in source_line:
      self.comment_has_copyright = True

    if "*/" in source_line:
      file_line_queue.put(source_line)
      if(self.comment_has_copyright):
        self.clear_queue()
      else:
        self.print_queue()
      self.comment_has_copyright = False
      self.S_CURR = S_CODE

  def process(self):
    source_file = open(self.source_file_name,"r")
    self.S_CURR = S_CODE
    source_line = ""

    for (source_line) in source_file:
      if(self.S_CURR == S_CODE):
        self.process_input_S_CODE(source_line)
      elif(self.S_CURR == S_COMMENT):
        self.process_input_S_COMMENT(source_line)

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
