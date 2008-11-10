#
# Copyright 2007,2008 by IDIAP Research Institute
#                        http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([Tracter], [0.2.1])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/PluginObject.cpp])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

# libresample
AC_ARG_WITH(libresample-includes,
  [AS_HELP_STRING(
    [--with-libresample-includes=DIR],
    [Compiler flags to include libresample headers]
  )],
  [],
  [with_libresample_includes=""]
)
AC_ARG_WITH(libresample-libs,
  [AS_HELP_STRING(
    [--with-libresample-libs=DIR],
    [Linker flags to find libresample libraries]
  )],
  [],
  [with_libresample_libs=""]
)
AC_MSG_CHECKING([whether to use libresample])
AS_IF(
  [test "$with_libresample_includes" != ""],
  [AC_DEFINE([HAVE_LIBRESAMPLE], [1], [Define if you have libresample.])
   AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)]
)
AC_SUBST([LIBRESAMPLE_INCLUDES], [$with_libresample_includes])
AC_SUBST([LIBRESAMPLE_LIBS], [$with_libresample_libs])
AM_CONDITIONAL([WITH_LIBRESAMPLE], [test "$with_libresample_includes" != ""])

# HTK
AC_ARG_WITH(htk-includes,
  [AS_HELP_STRING(
    [--with-htk-includes=DIR],
    [Directory containing HTK headers]
  )],
  [],
  [with_htk_includes=""]
)
AC_ARG_WITH(htk-libs,
  [AS_HELP_STRING(
    [--with-htk-libs=DIR],
    [List of HTK libraries]
  )],
  [],
  [with_htk_libs=""]
)
AC_MSG_CHECKING([whether to use HTK])
AS_IF(
  [test "$with_htk_includes" != ""],
  [AC_DEFINE([HAVE_HTKLIB], [1], [Define if you have HTK.])
   AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)]
)
AC_SUBST([HTK_INCLUDES], [$with_htk_includes])
AC_SUBST([HTK_LIBS], [$with_htk_libs])
AM_CONDITIONAL([WITH_HTK], [test "$with_htk_includes" != ""])

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
AC_MSG_CHECKING([whether to use Kiss FFT])
AS_IF(
  [test "x$with_kiss_fft" != xno],
  [AS_IF(
    [test -d "$with_kiss_fft"],
    [AC_MSG_RESULT([$with_kiss_fft])
     AC_DEFINE([HAVE_KISS_FFT], [1], [Define if you have Kiss FFT.])],
    [AC_MSG_ERROR([Can't find directory $with_kiss_fft])]
  )],
  [AC_MSG_RESULT([no])]
)
AM_CONDITIONAL([WITH_KISS_FFT], [test "$with_kiss_fft" != no])
AC_SUBST([KISS_FFT_DIR], [$with_kiss_fft])


# ALSA
# 1.0.6 is the version on CentOS at Sheffield; our Debian's is higher
PKG_CHECK_MODULES(ALSA, alsa >= 1.0.6, ac_cv_alsa=1, ac_cv_alsa=0)
AS_IF(
  [test "$ac_cv_alsa" = 1],
  [AC_DEFINE([HAVE_ALSA], [1], [Define if you have ALSA.])]
)
AM_CONDITIONAL([WITH_ALSA], [test "$ac_cv_alsa" = 1])
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LIBS)


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([munmap])

AC_CONFIG_FILES(
	[Makefile
	 tracter.pc
	 Doxyfile
	 src/Makefile
	 doc/Makefile]
)
AC_OUTPUT
