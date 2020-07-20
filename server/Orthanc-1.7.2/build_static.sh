#!/bin/bash

BUILDDIR="$PWD/Build"

if [ -d "${BUILDDIR}" ]; then
  # Control will enter here if $DIRECTORY exists.
    cd ${BUILDDIR}    
else
    mkdir -p ${BUILDDIR}
    cd ${BUILDDIR}
fi

cmake -DSTATIC_BUILD=ON -DCMAKE_BUILD_TYPE=Debug ../OrthancServer/
make
make doc
