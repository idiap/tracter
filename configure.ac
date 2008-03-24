#
# Copyright 2007,2008 by IDIAP Research Institute
#                        http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT([Tracter], [0.1.0])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/PluginObject.cpp])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB


# Kiss FFT
# Doesn't come as a proper package so we need a directory name
AC_ARG_WITH(kiss_fft,
  [AS_HELP_STRING(
    [--with-kiss-fft=DIR],
    [Use Kiss FFT in DIR for DFTs and DCTs]
  )],
  [],
  [with_kiss_fft=no]
)
AM_CONDITIONAL([WITH_KISS_FFT], [test "x$with_kiss_fft" != xno])
AC_MSG_CHECKING([whether to use Kiss FFT])
AS_IF(
  [test "x$with_kiss_fft" != xno],
  [AS_IF(
    [test -d "$with_kiss_fft"],
    [AC_MSG_RESULT([$with_kiss_fft])
     AC_SUBST([KISS_FFT_DIR], [$with_kiss_fft])
     AC_DEFINE_UNQUOTED(
       [HAVE_KISS_FFT], 1,
       [Define to 1 if you have Kiss FFT.]
     )],
    [AC_MSG_ERROR([Can't find directory $with_kiss_fft])]
  )],
  [AC_MSG_RESULT([no])]
)


# fftw3f
PKG_CHECK_MODULES(
    FFTW3F, fftw3f >= 3.1.2, ac_cv_fftw3f=1, ac_cv_fftw3f=0
)
AS_IF(
  [test "x$with_kiss_fft" != xno],
  [ac_cv_fftw3f=0
   AC_MSG_RESULT([FFTW3 overridden by Kiss FFT])],
  [AC_DEFINE_UNQUOTED(
     [HAVE_FFTW3F], ${ac_cv_fftw3f},
     [Define to 1 if you have libfftw3f.]
   )
   AC_SUBST(FFTW3F_CFLAGS)
   AC_SUBST(FFTW3F_LIBS)]
)
AM_CONDITIONAL([WITH_FFTW3F], [test $ac_cv_fftw3f == 1])


# libsamplerate, from the FAQ at http://www.mega-nerd.com/SRC/faq.html
PKG_CHECK_MODULES(
    SAMPLERATE, samplerate >= 0.1.2, ac_cv_samplerate=1, ac_cv_samplerate=0
)
AC_DEFINE_UNQUOTED(
    [HAVE_SAMPLERATE], ${ac_cv_samplerate},
    [Define to 1 if you have libsamplerate.]
)
AC_SUBST(SAMPLERATE_CFLAGS)
AC_SUBST(SAMPLERATE_LIBS)

# ALSA
PKG_CHECK_MODULES(
    ALSA, alsa >= 1.0.13, ac_cv_alsa=1, ac_cv_alsa=0
)
AC_DEFINE_UNQUOTED(
    [HAVE_ALSA], ${ac_cv_alsa},
    [Define to 1 if you have ALSA.]
)
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LIBS)

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
