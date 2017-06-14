#!/bin/bash

NAME='cubic-test'

DataSet=/tmp/${NAME}.train
IH=/tmp/${NAME}.ih


VW=vw
Errors=0

warn() {
    echo "$@" 1>&2
    Errors=$(($Errors+1))
}


die() {
    warn "$@"
    exit 1
}

verify_cubic_ABC() {
    vw="$1"
    ih="$IH"

    Errors=0

    # Run vw
    rm -f "$DataSet" "$ih"
    echo "1 'cubic_test|A 1 |B 2 3 |C 4 5 6" > $DataSet
    $vw --quiet --cubic ABC -d "$DataSet" --invert_hash "$ih"

    # Verify single features existence
    for f1 in 'A^1' 'B^2' 'B^3' 'C^4' 'C^5' 'C^6'; do
        grep -q "^$f1:" "$ih" || \
            warn "$vw: single feature '$f1' missing in $ih"
    done

    # Verify no quadratic features existence
    grep -q "^[ABC]\^[1-9].[ABC][1-9]:" "$ih" >/dev/null
    case $? in
        (0) warn "$vw: quadratic feature '$f1' APPEARING in $ih"
            ;;
        (1) : cool ;;
    esac

    # Verify cubic features existence
    f1='A^1'
    for f2 in 'B^2' 'B^3'; do
        for f3 in 'C^4' 'C^5' 'C^6'; do
            cubic_feature="$f1.$f2.$f3"
            grep -q "^$cubic_feature:" "$ih" || \
                warn "$vw: cubic feature '$cubic_feature' missing in $ih"
        done
    done
    case $Errors in
        (0) : ;;
        (*) warn "$vw: $Errors errors"
            ;;
    esac
}




#
# main
#
case "$#" in
    (0) die "Usage: $0 <vw_executable> ..."
        ;;
    (1) verify_cubic_ABC "$1"
        ;;
    (*)
        for vw in "$@"; do
            echo === Testing $vw
            verify_cubic_ABC "$vw"
        done
        ;;
esac

