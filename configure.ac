#
# Copyright 2007 by IDIAP Research Institute
#                   http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT([Tracter], [0.1.0])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/creature.cpp])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

# libsamplerate, from the FAQ at http://www.mega-nerd.com/SRC/faq.html
PKG_CHECK_MODULES(
    SAMPLERATE, samplerate >= 0.1.2, ac_cv_samplerate=1, ac_cv_samplerate=0
)
AC_DEFINE_UNQUOTED(
    [HAVE_SAMPLERATE],${ac_cv_samplerate},
    [Set to 1 if you have libsamplerate.]
)
AC_SUBST(SAMPLERATE_CFLAGS)
AC_SUBST(SAMPLERATE_LIBS)

# ALSA
PKG_CHECK_MODULES(
    ALSA, alsa >= 1.0.13, ac_cv_alsa=1, ac_cv_alsa=0
)
AC_DEFINE_UNQUOTED(
    [HAVE_ALSA],${ac_cv_alsa},
    [Set to 1 if you have ALSA.]
)
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LIBS)

# fftw3f
PKG_CHECK_MODULES(
    FFTW3F, fftw3f >= 3.1.2, ac_cv_fftw3f=1, ac_cv_fftw3f=0
)
AC_DEFINE_UNQUOTED(
    [HAVE_FFTW3F],${ac_cv_fftw3f},
    [Set to 1 if you have libfftw3f.]
)
AC_SUBST(FFTW3F_CFLAGS)
AC_SUBST(FFTW3F_LIBS)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([munmap])

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 doc/Makefile])
AC_OUTPUT
