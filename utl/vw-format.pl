#!/usr/bin/perl -w
#
# Convert TSV/CSV to VW training set format
#
# Supports:
#   - Optionally use header line for feature names ('-h' option)
#   - If no  '-h' is used, will number features as 1..k based on
#     column number
#   - Multiclass labels will be auto-converted to 1..k if they are
#     non-numeric e.g. Species: {setosa, versicolor, virginica}
#   - Categorical features are auto-converted to vw boolean name=value`
#   - Numerical features will use name:value
#   - Ability to specify label, tag and data-set splitting columns both
#     by name or index (positive and negative).
#   - Numeric command-line arg allows specifiying the label column
#     number (also: negative numbers conveniently support the
#     "from end of line" perl convention (e.g: -1 is last column)
#   - Allows specifying (and overriding) the input separator as perl
#     regexp ('-s <regexp>' option), by default using command/tab
#   - Allows specifying exclude regex pattern to omit certain columns
#   - Allows grouping of columns in namespaces based on a regex patter
#     given.
#
# Ivan Georgiev,October'2015
#
# Based on csv2vw script by:
# Ariel Faigon, May 2015
#

use Getopt::Long;
use Scalar::Util qw(looks_like_number);
use List::Util qw(first);

our ($o_verbose, $o_help, $o_header, $o_namespace, $o_tag, $o_prefix, $o_label, $o_exclude, $o_split, $o_dense, $o_output);

my $FieldSep = qr{[,\t]};
my $ExcludePat = qr/^\s+$/;    # Match all empty feature names
my $LabelCol = 0;            # Label column to use - after processing user input
my $TagCol = undef;            # Tag column to use - after processing user input
my $SplitCol = undef;        # Split column for examples splitting in datasets
my $LineNo = 0;                # Could be used instead of tag, if none is provided

sub verbose {
    return unless $o_verbose;
    if (@_ == 1) {
        print STDERR @_;
    } else {
        printf STDERR @_;
    }
}

sub usage {
    die "Usage: $0 [options] [file(s)...]
    Options:
        --verbose                verbose (on stderr)
        --help                   first line is header
        --header [<filename>]    header for the columns in the input.
        --namespace <regexp>     sed-like pattern for building namespaces, based
                                 on coulumn names
        --tag <tag_column>       column providing the example identifier
        --label <label_column>   column providing the label (class) for
                                 the example
        --tag-prefix <str>       prefix all tags with given string
        --exclude <regexp>       pattern for matching column names to be excluded
        --split <column>         column name (or index) to be used as a dataset
                                 splitting factor
        --dense                  puts all values, even zeroes. Off by default.
        --output <filename>      output files used with conjunction of --split.
        --delimiter <delimiter>  explicitly specify field separator
                                 (perl-regexp). Default is: '$FieldSep'

    Args:
        Input file(s) are read sequentially considering they come one after another
        in terms of rows. Providing '--header' option automatically skips the furst
        line of every file.
    
        Both <tag_column> and <label_column> can be strings, in which case this is
        column's name or integers, in which case they are treated as column indeces
        (starting from 0). If the value is negative - it is treated as a column
        position from the end (i.e. -1 is the last column).

        If a labels read aren't numeric, they will be assumed to be a multi-class
        labels and be converted to an integer [1..k] (vw multiclass-representation)

    Examples:
        $0 --header --label -1 iris.csv
            Use 1st line as header, last column as label

        $0 2 data.tsv
            Use 1..k as column/feature names, use 3rd column
            as the label column (base index is 0) - no header
            assumed in input

        $0 --header --tag Tag --namespace \(\\w\\d\).+data.tsv
            Use 1st line as a header, idetifying examples by the
            content of column names 'Tag' and grouping other
            columns in namespaces which match the <letter><digit>
            pattern given.
            
";
}

sub init {
    GetOptions( 'verbose' => \$o_verbose,
                'help' => \$o_help,
                'header:s' => \$o_header,
                'namespace=s' => \$o_namespace,
                'tag=s' => \$o_tag,
                'tag-prefix=s' => \$o_prefix,
                'label=s' => \$o_label,
                'exclude=s' => \$o_exclude,
                'split=s' => \$o_split,
                'dense' => \$o_dense,
                'output=s' => \$o_output,
                'delimiter=s' => \$FieldSep
    ) or usage();

    usage() if (defined $o_help);

    verbose("Header line is taken from: %s\n", $o_header) if (defined $o_header);
    verbose("Namespace pattern is: %s\n", qr/$o_namespace/) if (defined $o_namespace);
    verbose("Tag's column is: %s\n", $o_tag) if (defined $o_tag);
    verbose("Tag's prefix: %s\n", $o_prefix) if (defined $o_prefix);
    verbose("Label's column is: %s\n", $o_label) if (defined $o_label);

    $ExcludePat = qr/$o_exclude/ if (defined $o_exclude);
    verbose("Exclude columns matching pattern: %s\n", $ExcludePat);
    
    verbose("Split output based on column: %s\n", $o_split) if (defined $o_split);
    verbose("Outputing dense matrix\n") if (defined $o_dense);
    verbose("Output file(s) name: %s\n", $o_output) if (defined $o_output);
    verbose("Column separator: %s\n", $FieldSep);
    
    die ("Splitting asked, but no output is provided!\n") if ((defined $o_split and !defined($o_output)) or (!defined($o_split) and defined($o_output)));
}

my %Label2KMap;
my $MaxK = 0;

sub label2k($) {
    my $label = shift;
    return $Label2KMap{$label} if (exists $Label2KMap{$label});
    
    $MaxK++;
    $Label2KMap{$label} = $MaxK;
    
    verbose("New multi-class added: %s\n", $label);
    
    $MaxK;
}

my @RowFeatures = ();    # Temporary read (first) line

sub columnIdx($) {
    my $col = shift;
    
    if (looks_like_number($col)) {
        if ($col < 0) {
            $col = $#RowFeatures + 1 + $col;
        }
        
        unless (0 <= $col and $col <= $#RowFeatures) {
            die "Label Column: '$col' is out of range for [0 .. $#RowFeatures]\n";
        }
    }
    elsif (defined($o_header)) {
        my $name = $col;
        $col = first { $RowFeatures[$_] eq $name } 0 .. $#RowFeatures;
        die ("Given column name '$name' not found!") unless(defined $col);
    }
    else {
        die ("Specified column name: '$col' with no headers mode.");
    }
    
    $col;
}

my %Namespaces;            # Namespace mapping
my @FeatureNames = ();    # Names of the features, based on the column index

sub buildFeatures() {
    unless (defined $o_namespace) {
        $Namespaces{'f'} = [ () ]; # i.e. create the default namespace
        verbose ("Creating default namespace: 'f'\n");
    }
    
    foreach my $i (0 .. $#FeatureNames) {
        my $ns = '';
        my $feature = $FeatureNames[$i];
        
        next if ($feature =~ $ExcludePat);
        if (defined $o_namespace) {
            ($ns) = ($feature =~ qr/$o_namespace/);
            $ns = 'f' unless (defined $ns);
            verbose ("Creating new namespace: '%s'\n", $ns) unless (exists $Namespaces{$ns});
        }
        else {
            $ns = 'f';
        }
        
        push (@{$Namespaces{$ns}}, $i);
    }
}

my %SplitFiles;

sub outputHandle($) {
    my $split = shift;
    
    if (defined $split) {
        $split =~ s/^\s+|\s+$//g; # i.e. - trim
    }
    else {
        $split = "_";
    }
    
    return $SplitFiles{$split} if (exists $SplitFiles{$split});
    
    my $fname = $o_output . '.' . $split . '.vw';
    open (my $ff, ">", $fname) or die ("Failed to open split-output: $fname");
    verbose("New dataset output openned: %s\n", $split);
    $SplitFiles{$split} = $ff;
    
    $ff;
}

sub closeOutputs() {
    foreach my $split (keys %SplitFiles) {
        close $SplitFiles{$split};
    }
}

#
# -- main
#
init();

while (<>) {
    chomp;
    @RowFeatures = split($FieldSep);
    
    #deal with the first line - header ot not, we have things to do.
    if ($. == 1) {
        my @feature_indexes = (0 .. $#RowFeatures);
        
        $LabelCol = defined $o_label ? columnIdx($o_label) : 0;
        verbose("Actual label column index is: %d (out of %d)\n", $LabelCol, $#RowFeatures);

        $TagCol = defined $o_tag ? columnIdx($o_tag) : undef;
        verbose("Actual tag column index is: %d (out of %d)\n", $TagCol, $#RowFeatures) if (defined $TagCol);

        $SplitCol = defined $o_split ? columnIdx($o_split) : undef;
        verbose("Actual split column idex is: %d (out of %d)\n", $SplitCol, $#RowFeatures) if (defined $SplitCol);

        if (defined $o_header) {
            @FeatureNames = @RowFeatures[@feature_indexes];
            buildFeatures();
            next;
        } else {
            @FeatureNames = @feature_indexes;
            buildFeatures();
        }
    }

    $LineNo++;
    
    my $fh = defined $SplitCol ? outputHandle($RowFeatures[$SplitCol]) : STDOUT;

    my $label = $RowFeatures[$LabelCol] || 'undef';
    $label = label2k($label) unless (looks_like_number($label));
    
    my $tag = defined $TagCol ? $RowFeatures[$TagCol] : $LineNo;
    $tag = $o_prefix . $tag if (defined $o_prefix);
    
    printf $fh "%s %s", $label, $tag;
    foreach my $ns (keys %Namespaces) {
        my $nsput = 0;
        my $sep = undef;
        
        foreach my $i (@{$Namespaces{$ns}}) {
            my $val = $RowFeatures[$i];
            next unless (defined $val);
            next if ($i == $LabelCol);
            next if (defined($TagCol) and $i == $TagCol);
            next if (defined($SplitCol) and $i == $SplitCol);
            
            unless (looks_like_number($val)) {
                $sep = '=';
            }
            elsif (defined($o_dense) or $val != 0) {
                $sep = ':';
            }
            else {
                next;
            }
            
            unless ($nsput) {
                printf $fh "|%s ", $ns;
                $nsput = 1;
            }
            
            printf $fh "%s%s%s ", $FeatureNames[$i], $sep, $val;
        }
    }
    
    print $fh "\n";
}

verbose("Lines processed: %d\n", $LineNo);
closeOutputs();
