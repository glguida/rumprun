#!/bin/sh
#
# Build rumprun for MH/MRG
#

usage() {
    echo "MH/MRG build for rump run"
    echo "Usage"
    echo "   $(basename "$0") <MHSRC>"
    echo
    echo "MHSRC points to the base directory of MH, already compiled."
    exit 1
}

if [ ! $# -eq 1 ]; then
    usage
fi

mrgdir=$1
shift

if [ ! -e buildrump.sh-orig ]; then
    git submodule update --init
    mv buildrump.sh buildrump.sh-orig
    ln -s ${mrgdir}/exts/buildrump-20170910 buildrump.sh
fi

PATH=${mrgdir}/toolchain/install/bin:$PATH CC=i686-elf-murgia-gcc ./build-rr.sh mrg build install
