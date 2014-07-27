#!/bin/sh
# -- vw daemon test
#

export PATH="wowpalwabbit:../vowpalwabbit:${PATH}"
# The VW under test
VW=`which vw`
#
# VW=vw-7.20140627    Good
# VW=vw-7.20140709    Bad
#
#   7e138ac19bb3e4be88201d521249d87f52e378f3    BAD
#   cad00a0dd558a34f210b712b34da26e31374b8b9    GOOD
#

NAME=vw-daemon-test

MODEL=$NAME.model
TRAINSET=$NAME.train
PREDREF=$NAME.predref
PREDOUT=$NAME.predict
PORT=32223

# -- make sure we can find vw first
if [ -x "$VW" ]; then
    : cool found vw at: $VW
else
    $NAME: can not find vw under $PATH - sorry
    exit 1
fi

# A command and pattern that will unlikely to match anything but our own test
DaemonCmd="$VW -t -i $MODEL --daemon --quiet --port 32223"

stop_daemon() {
    # make sure we are not running, may ignore 'error' that we're not
    # echo stopping daemon
    pkill -9 -f "$DaemonCmd" 2>&1 | grep -qv ': no process found'
    # relinquish CPU by forcing some conext switches to be safe
    # (let existing vw daemon procs die)
    wait
}

start_daemon() {
    # echo starting daemon
    $DaemonCmd
    # give it time to be ready
    wait
}

cleanup() {
    /bin/rm -f $MODEL $TRAINSET $PREDREF $PREDOUT
    stop_daemon
}

# -- main
cleanup

# prepare training set
cat > $TRAINSET <<EOF
0.55 1 '1| a
0.99 1 '2| b c
EOF

# prepare expected predict output
cat > $PREDREF <<EOF
0.553585 1
0.733882 2
EOF


# Train
$VW --quiet -d $TRAINSET -f $MODEL

start_daemon

# Test on train-set
nc localhost $PORT < $TRAINSET > $PREDOUT
diff $PREDREF $PREDOUT

case $? in
    0)  echo $NAME: OK
        cleanup
        exit 0
        ;;
    1)  echo "$NAME FAILED: see $PREDREF vs $PREDOUT "
        stop_daemon
        exit 1
        ;;
    *)  echo $NAME: diff failed - something is fishy
        stop_daemon
        exit 2
        ;;
esac

