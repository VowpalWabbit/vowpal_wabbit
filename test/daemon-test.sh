#!/bin/sh
# -- vw daemon test
#
NAME='vw-daemon-test'

# This is a ugly hack:
# Travis doesn't like this test, possibly because of firewall rules
# on the travis-ci env, so don't bother running it on travis machines.
HOSTNAME=`hostname`
case $HOSTNAME in
    *worker-linux*|*travis-ci.org)
        # Don't generate anything to STDERR or it'll fail
        : "travis host: $HOSTNAME detected, skipping test: $0"
        echo "$NAME: OK"
        exit 0
        ;;
esac

export PATH="vowpalwabbit:../vowpalwabbit:${PATH}"
# The VW under test
VW=`which vw`
#
# VW=vw-7.20140627    Good
# VW=vw-7.20140709    Bad
#
#   7e138ac19bb3e4be88201d521249d87f52e378f3    BAD
#   cad00a0dd558a34f210b712b34da26e31374b8b9    GOOD
#


MODEL=$NAME.model
TRAINSET=$NAME.train
PREDREF=$NAME.predref
PREDOUT=$NAME.predict
LOCALHOST=0
PORT=54248

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
    echo "$NAME: can not find 'netcat' in $PATH - sorry"
    exit 1
fi

# -- and pkill
PKILL=`which pkill`
if [ -x "$PKILL" ]; then
    : cool found pkill at: $PKILL
else
    echo "$NAME: can not find 'pkill' in $PATH - sorry"
    exit 1
fi

#
# fractional seconds sleep
#
mysleep() {
    case "$1" in
        *[0-9]*) Seconds="$1" ;;
        *) Seconds=0.2 ;;
    esac
    perl -e "select(undef,undef,undef,$Seconds)"
}

# A command and pattern that will unlikely to match anything but our own test
DaemonCmd="$VW -t -i $MODEL --daemon --num_children 1 --quiet --port $PORT"

stop_daemon() {
    # make sure we are not running, may ignore 'error' that we're not
    # echo stopping daemon
    $PKILL -9 -f "$DaemonCmd" 2>&1 | grep -q 'no process found'
    # relinquish CPU by forcing some conext switches to be safe
    # (to let existing vw daemon procs die)
    mysleep 0.1
}

start_daemon() {
    # echo starting daemon
    $DaemonCmd </dev/null >/dev/null &
    # give vw some time to load the model and be ready
    mysleep 0.1
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
$VW -b 10 --quiet -d $TRAINSET -f $MODEL

start_daemon

# Test on train-set, gnu netcat returns immediately, but OpenBSD netcat
# hangs unless we use '-q 0' (which is GNU netcat incompatible)
# Hacky solution is to start netcat in the background and wait for
# it to output two lines.
$NETCAT -n $LOCALHOST $PORT < $TRAINSET > $PREDOUT &

# Wait until we recieve a prediction from the vw daemon then kill netcat
until [ `wc -l < $PREDOUT` -eq 2 ]; do mysleep 0.1; done
$PKILL -9 $NETCAT

diff $PREDREF $PREDOUT
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

