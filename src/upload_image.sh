#!/bin/sh

if [ "$#" -ne 1 ] || ! [ -f "$1" ]; then
    echo "Usage: $0 FILE" >&2
    exit 1
else
    FILENAME=$1
fi

curl -X POST http://localhost:8042/instances --data-binary @${FILENAME}
