#!/bin/sh

set -eux

ME="`readlink -f "$0"`"
DIR="`dirname "$ME"`"
NAME="`basename "$DIR"`"

docker build -t "$NAME" .

exec docker run \
   -i \
   -t \
   --rm \
   --net=host \
   -v "${DIR}:/usr/src/${NAME}" \
   "$NAME" bash -l
