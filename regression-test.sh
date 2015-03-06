#!/bin/bash

# Variables/constants
PID=$$
REPEATS=${REPEATS:-10}
VERBATIM=${VERBATIM:-./verbatim}

# Tools
TIME=${TIME:-/usr/bin/time}
PERF=${PERF:-/usr/bin/perf}
VALGRIND=${VALGRIND:-/usr/bin/valgrind}

$PERF stat -c -r $REPEATS -o ./perf-${PID}.log -- sh -c "exec $VERBATIM $@"

(
$VALGRIND $VERBATIM $@
$TIME -f 'RSS: %M\tMPF: %F' -- $VERBATIM $@
) > ./valgrind-${PID}.log 2>&1

echo "RAM usage summary:"
grep '^RSS: ' ./valgrind-${PID}.log | sort -g -k 2
