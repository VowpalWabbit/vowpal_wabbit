#define ARRAYS_OPTIMIZED

%rename("%(camelcase)s", notregexmatch$name="examples_writer_template|vector") "";

%rename("%(lowercamelcase)s", %$isfunction, %$ismember) "";
// TODO: rename methods seperately
