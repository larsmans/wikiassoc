#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([wikiassoc], [0.0], [larsmans@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([config.h])

m4_include([m4/ax_boost_base.m4])
m4_include([m4/ax_boost_iostreams.m4])

# Checks for programs.
AC_PROG_CXX
AC_LANG([C++])
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([bz2], [BZ2_bzRead])
AC_CHECK_LIB([z], [gzread])

AX_BOOST_BASE([1.37])
AX_BOOST_IOSTREAMS

AC_ARG_WITH([google-sparsehash],
            [AS_HELP_STRING([--with-google-sparsehash[=DIR]],
              [Use Google sparsehash library for smaller memory footprint
               (enabled by default if detected)])],
            [],
            [with_google_sparsehash=check])
AS_IF([test "x$with_google_sparsehash" != xno],
      [AS_IF([test "x$with_google_sparsehash" != xcheck],
             [CPPFLAGS="${CPPFLAGS} -I${with_google_sparsehash}/include"])]
      [AC_CHECK_HEADERS([google/sparse_hash_map])])

# Checks for typedefs, structures, and compiler characteristics.
AC_OPENMP


AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
