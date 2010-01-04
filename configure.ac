#
# Copyright 2007,2008 by IDIAP Research Institute
#                        http://www.idiap.ch
#
# See the file COPYING for the licence associated with this software.
#

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([Tracter], [0.5.0])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/TracterObject.cpp])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL


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


# libsndfile
PKG_CHECK_MODULES(SNDFILE, sndfile >= 1.0.17, ac_cv_sndfile=1, ac_cv_sndfile=0)
AS_IF(
  [test "$ac_cv_sndfile" = 1],
  [AC_DEFINE([HAVE_SNDFILE], [1], [Define if you have SNDFILE.])]
)
AM_CONDITIONAL([WITH_SNDFILE], [test "$ac_cv_sndfile" = 1])
AC_SUBST(SNDFILE_CFLAGS)
AC_SUBST(SNDFILE_LIBS)


# Check for Torch3 first using an explicit --with-torch3
AC_MSG_CHECKING([whether to use Torch3])
AC_ARG_WITH(torch3,
  [AS_HELP_STRING(
    [--with-torch3=DIR],
    [Use Torch3 in DIR]
  )],
  [],
  [with_torch3=yes]
)
AS_IF( [test "$with_torch3" = yes],

  # yes is default, but not informative; Try via pkgconfig (at Idiap)
  [PKG_CHECK_MODULES(TORCH, torch >= 3.1, pkg_torch3=1, pkg_torch3=0)
   AS_IF( [test "$pkg_torch3" = 1],
     [AC_DEFINE([HAVE_TORCH3], [1], [Define if you have TORCH3.])
      AC_SUBST([TORCH3_INCLUDES], [$TORCH_CFLAGS])
      AC_SUBST([TORCH3_LIBS], [$TORCH_LIBS])],
     [with_torch3=no]
   )],

  # There is an explicit --with-torch3=dir
  [AS_IF( [test "$with_torch3" != no],
    [AS_IF( [test -d "$with_torch3"],
      [AC_DEFINE([HAVE_TORCH3], [1], [Define if you have Torch3.])
       AC_SUBST(
         [TORCH3_INCLUDES],
	 ["-I$with_torch3/core \
	   -I$with_torch3/speech \
           -I$with_torch3/datasets \
           -I$with_torch3/distributions \
           -I$with_torch3/gradients"]
       )
       AC_SUBST(
         [TORCH3_LIBS],
	 ["-L$with_torch3/libs/Linux_opt_float -ltorch"]
       )],
      [AC_MSG_ERROR([Can't find directory $with_torch3])
       with_torch3=no]
     )]
   )
   AC_MSG_RESULT([$with_torch3])]
)
AM_CONDITIONAL([WITH_TORCH3], [test "$with_torch3" != no])


# BSAPI
AC_ARG_WITH(bsapi-includes,
  [AS_HELP_STRING(
    [--with-bsapi-includes=DIR],
    [Directory containing BSAPI headers]
  )],
  [],
  [with_bsapi_includes=""]
)
AC_ARG_WITH(bsapi-libs,
  [AS_HELP_STRING(
    [--with-bsapi-libs=DIR],
    [List of BSAPI libraries]
  )],
  [],
  [with_bsapi_libs=""]
)
AC_MSG_CHECKING([whether to use BSAPI])
AS_IF(
  [test "$with_bsapi_includes" != ""],
  [AC_DEFINE([HAVE_BSAPI], [1], [Define if you have BSAPI.])
   AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)]
)
AC_SUBST([BSAPI_INCLUDES], [$with_bsapi_includes])
AC_SUBST([BSAPI_LIBS], [$with_bsapi_libs])
AM_CONDITIONAL([WITH_BSAPI], [test "$with_bsapi_includes" != ""])

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

# SPTK
AC_ARG_WITH(sptk-includes,
  [AS_HELP_STRING(
    [--with-sptk-includes=DIR],
    [Directory containing SPTK headers]
  )],
  [],
  [with_sptk_includes=""]
)
AC_ARG_WITH(sptk-libs,
  [AS_HELP_STRING(
    [--with-sptk-libs=DIR],
    [List of SPTK libraries]
  )],
  [],
  [with_sptk_libs=""]
)
AC_MSG_CHECKING([whether to use SPTK])
AS_IF(
  [test "$with_sptk_includes" != ""],
  [AC_DEFINE([HAVE_SPTK], [1], [Define if you have SPTK.])
   AC_MSG_RESULT(yes)],
  [AC_MSG_RESULT(no)]
)
AC_SUBST([SPTK_INCLUDES], [$with_sptk_includes])
AC_SUBST([SPTK_LIBS], [$with_sptk_libs])
AM_CONDITIONAL([WITH_SPTK], [test "$with_sptk_includes" != ""])

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

AS_IF(
  [test "$with_kiss_fft" = "no"],
  [AC_ERROR([Required library Kiss FFT not found])]
)


# libresample
# As above, doesn't come as a proper package so we need a directory name
AC_ARG_WITH(libresample,
  [AS_HELP_STRING(
    [--with-libresample=DIR],
    [Use libresample for sample rate conversion]
  )],
  [],
  [with_libresample=no]
)
AC_MSG_CHECKING([whether to use libresample])
AS_IF(
  [test "x$with_libresample" != xno],
  [AS_IF(
    [test -d "$with_libresample"],
    [AC_MSG_RESULT([$with_libresample])
     AC_DEFINE([HAVE_LIBRESAMPLE], [1], [Define if you have libresample.])],
    [AC_MSG_ERROR([Can't find directory $with_libresample])]
  )],
  [AC_MSG_RESULT([no])]
)
AM_CONDITIONAL([WITH_LIBRESAMPLE], [test "$with_libresample" != no])
AC_SUBST([LIBRESAMPLE_DIR], [$with_libresample])


# RtAudio
# Although it comes as a package it's easier to compile it here
AC_ARG_WITH(rtaudio,
  [AS_HELP_STRING(
    [--with-rtaudio=DIR],
    [Use RtAudio to connect to sound card]
  )],
  [],
  [with_rtaudio=no]
)
AC_MSG_CHECKING([whether to use RtAudio])
AS_IF(
  [test "x$with_rtaudio" != xno],
  [AS_IF(
    [test -d "$with_rtaudio"],
    [AC_MSG_RESULT([$with_rtaudio])
     AC_DEFINE([HAVE_RTAUDIO], [1], [Define if you have RtAudio.])],
    [AC_MSG_ERROR([Can't find directory $with_rtaudio])]
  )],
  [AC_MSG_RESULT([no])]
)
AM_CONDITIONAL([WITH_RTAUDIO], [test "$with_rtaudio" != no])
AC_SUBST([RTAUDIO_DIR], [$with_rtaudio])

# Boost
# Just a flag to turn it on and off.
# ...and it actually does nothing now.  Maybe it will when 1.35 goes into
# Ubuntu with asio and hence sockets.
AC_ARG_ENABLE(boost,
  [AS_HELP_STRING(
    [--enable-boost],
    [Enable Boost]
  )],
  [],
  [enable_boost=yes]
)
AC_MSG_CHECKING([whether to enable Boost])
AS_IF(
  [test "x$enable_boost" != xno],
  [AC_MSG_RESULT([yes])
   AC_DEFINE([HAVE_BOOST], [1], [Define if you have Boost.])
  ],
  [AC_MSG_RESULT([no])]
)
AM_CONDITIONAL([WITH_BOOST], [test "$enable_boost" != no])


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
