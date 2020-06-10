This tool can be used to measure throughput of parsers in vw. It supports both text and dsjson formats.

## Options
```
-h [ --help ]         Produce help message
-d [ --data ] arg     Data file to read
-a [ --args ] arg     VW args to setup parser correctly
-t [ --type ] arg     Type of input format. [text, djson]
```

## Usage examples
```sh
# Parses the 'rcv1.test.vw' file as VW text format
./parser_throughput --type text --data rcv1.test.vw
# Uses the dsjson parser to read the file 'ccb_test.dat', --args is used to ensure the parser can read the CCB labels
./parser_throughput --type dsjson --data ccb_test.dat --args "--ccb_explore_adf"
# Same as previous but through the --args option the slates subparser is being used
./parser_throughput --type dsjson --data throughput_test.json --args "--slates"
# Same as previous but through the --args option cb parser is being specified
./parser_throughput --type dsjson --data cb_data.dsjson --args "--cb_explore_adf"
```

## Results
Results are dependent on many factors such as processor, contents of data, build flags. This also does not take into account the fact that generally the information density of these two formats is not the same.
- VW text: ~480 MB/s
- DSJSON: ~225 MB/s
