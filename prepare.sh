#!/bin/sh
#
# Copyright 2016 by Idiap Research Institute, http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, January 2016
#

# Download Kiss FFT (whether or not it's needed)
export KISSDIR=kiss_fft130
if [ ! -e $KISSDIR ]
then
    kisssrc=http://sourceforge.net/projects/kissfft/files/kissfft/v1_3_0
    curl -LO $kisssrc/$KISSDIR.tar.gz
    tar zxf $KISSDIR.tar.gz
    ln -sf $KISSDIR kissfft
fi

# CMake finders
if [ ! -e cmake/FindLibSSP.cmake ]
then
    curl \
        -L https://github.com/Idiap/libssp/raw/master/cmake/FindLibSSP.cmake \
        -o cmake/FindLibSSP.cmake
fi

if [ ! -e cmake/FindLibUBE.cmake ]
then
    curl \
        -L https://github.com/pgarner/libube/raw/master/cmake/FindLibUBE.cmake \
        -o cmake/FindLibUBE.cmake
fi
