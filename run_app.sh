#!/bin/bash

# Fail on error
set -e


if [ ! -z "$1" ]
then
    COMPILE_TARGET=./build/data-fetcher
    if [ ! -f ${COMPILE_TARGET} ]
    then
        bash compile_app.sh
    fi

    # Run the application locally
    set -o allexport
    source .env
    set +o allexport
    ${COMPILE_TARGET} $1

else
    echo "Usage: $0 <DATE>"
    exit 1
fi
