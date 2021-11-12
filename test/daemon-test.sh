#!/bin/bash
# -- vw daemon test
#
NAME='vw-daemon-test'

export PATH="vowpalwabbit:../build/vowpalwabbit:${PATH}"
# The VW under test
VW=`which vw`

MODEL=$NAME.model
TRAINSET=$NAME.train
PREDREF=$NAME.predref
PREDOUT=$NAME.predict
NETCAT_STATUS=$NAME.netcat-status
PORT=54248

while [ $# -gt 0 ]
do
    case "$1" in
        --foreground)
            Foreground="$1"
            ;;
        --json)
            JSON="$1 --chain_hash"
            ;;
        --port)
            PORT="$2"
            shift
            ;;
        --vw)
            VW="$2"
            shift
            ;;
        *)
            echo "$NAME: unknown argument $1"
            exit 1
            ;;
    esac
    shift
done

# -- make sure we can find vw first
if [ -x "$VW" ]; then
    : cool found vw at: $VW
else
    echo "$NAME: can not find 'vw' in $PATH - sorry"
    exit 1
fi

# -- and netcat
NETCAT=`which netcat`
if [ -x "$NETCAT" ]; then
    : cool found netcat at: $NETCAT
else
    NETCAT=`which nc`
    if [ -x "$NETCAT" ]; then
        : "no netcat but found 'nc' at: $NETCAT"
    else
        echo "$NAME: can not find 'netcat' not 'nc' in $PATH - sorry"
        exit 1
    fi
fi

# -- and pkill
PKILL=`which pkill`
if [ -x "$PKILL" ]; then
    : cool found pkill at: $PKILL
else
    echo "$NAME: can not find 'pkill' in $PATH - sorry"
    exit 1
fi

# A command (+pattern) that is unlikely to match anything but our own test
DaemonCmd="$VW -t -i $MODEL --daemon $Foreground --num_children 1 --quiet --port $PORT $JSON"
# libtool may wrap vw with '.libs/lt-vw' so we need to be flexible
# on the exact process pattern we try to kill.
DaemonPat=`echo $DaemonCmd | sed 's/^[^ ]*vw /.*vw /'`

stop_daemon() {
    # Make sure we are not running. May ignore 'error' that we're not
    $PKILL -9 -f "$DaemonPat" 2>&1 | grep -q 'no process found'

    # relinquish CPU by forcing some context switches to be safe
    # (let existing vw daemon procs die)
    if echo "$DaemonPat" | grep -q -v -- --foreground; then
        wait
    else
        sleep 0.1
    fi
}

start_daemon() {
    # echo starting daemon
    $DaemonCmd </dev/null >/dev/null &
    PID=$!
    # give it time to be ready
    if echo "$DaemonCmd" | grep -q -v -- --foreground; then
        wait; wait; wait
    else
        sleep 0.1
    fi
    echo "$PID"
}

cleanup() {
    /bin/rm -f $MODEL $TRAINSET $PREDREF $PREDOUT $NETCAT_STATUS
    stop_daemon
}

# -- main
cleanup

txt_dataset() {
# prepare training set
cat > $TRAINSET <<EOF
0.55 1 '1| a
0.99 1 '2| b c
EOF
}

json_dataset() {
# prepare training set json
cat > $TRAINSET <<EOF
{"_label":{"Weight":1, "Label":0.55}, "_tag":"1", "a":true}
{"_label":{"Weight":1, "Label":0.99}, "_tag":"2", "b":true, "c":true}
EOF
}

if [ -n "$JSON" ]; then
    json_dataset
else
    txt_dataset
fi

# prepare expected predict output
cat > $PREDREF <<EOF
0.553585 1
0.733882 2
EOF

# Train
$VW -b 10 --quiet -d $TRAINSET -f $MODEL $JSON

DaemonPid=`start_daemon`

# Test --foreground argument
PidsAreEqual=false
for ProcessPid in $(pgrep -f "$DaemonPat" 2>&1)
do
    if [ $DaemonPid -eq $ProcessPid ]; then
        PidsAreEqual=true
    fi
done
if [ $Foreground ]; then
    if ! $PidsAreEqual ; then
        echo "$NAME FAILED: --foreground, but vw has run in the background"
        stop_daemon
        exit 1
    fi
else
    if $PidsAreEqual ; then
        echo "$NAME FAILED: vw has not run in the background"
        stop_daemon
        exit 1
    fi
fi

# Test on train-set
# OpenBSD netcat quits immediately after stdin EOF
# nc.traditional does not, so let's use -w 1 which will silently close the connection if the connection
# or stdin are silent for more than 1 second
DELAY_OPT="-w 1"

$NETCAT $DELAY_OPT localhost $PORT < $TRAINSET > $PREDOUT

$PKILL -9 $NETCAT

# We should ignore small (< $Epsilon) floating-point differences (fuzzy compare)
diff <(cut -c-5 $PREDREF) <(cut -c-5 $PREDOUT)
case $? in
    0)  echo "$NAME: OK"
        cleanup
        exit 0
        ;;
    1)  echo "$NAME FAILED: see $PREDREF vs $PREDOUT"
        stop_daemon
        exit 1
        ;;
    *)  echo "$NAME: diff failed - something is fishy"
        stop_daemon
        exit 2
        ;;
esac
