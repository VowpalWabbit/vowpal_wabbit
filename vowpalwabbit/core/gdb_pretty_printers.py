import itertools
import gdb


class VArrayPrinter:
    "Print a v_array<T>"

    class _iterator:
        def __init__(self, begin_array, end_array):
            self.item = begin_array
            self.end_array = end_array
            self.count = 0

        def __iter__(self):
            return self

        def __next__(self):
            count = self.count
            self.count = self.count + 1

            if self.item == self.end_array:
                raise StopIteration
            element = self.item.dereference()
            self.item = self.item + 1
            return ("[{}]".format(count), element)

    def __init__(self, value):
        self.value = value
        self.begin_array = self.value["_begin"]
        self.end_array = self.value["_end"]
        self.end_buffer = self.value["_end_array"]
        self.erase_count = self.value["_erase_count"]
        self.size = int(self.end_array - self.begin_array)
        self.capacity = int(self.end_buffer - self.begin_array)

    def children(self):
        object_info = [
            ("size", self.size),
            ("capacity", self.capacity),
            ("erase count", self.erase_count),
        ]
        children_item_iterator = self._iterator(self.begin_array, self.end_array)
        return itertools.chain(iter(object_info), children_item_iterator)

    def to_string(self):
        return "size={}, capacity={}, erase count={}".format(
            self.size, self.capacity, self.erase_count
        )

    @staticmethod
    def display_hint():
        return "array"


def build_pretty_printer():
    pretty_printer = gdb.printing.RegexpCollectionPrettyPrinter("vowpalwabbit")
    pretty_printer.add_printer("v_array", ".*v_array.*", VArrayPrinter)
    return pretty_printer


gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer())
