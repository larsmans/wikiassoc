#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([wikiassoc], [0.0], [larsmans@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_LANG([C++])
AC_PROG_INSTALL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lboost_iostreams':
#AC_CHECK_LIB([boost_iostreams], [main])
AC_CHECK_LIB([bz2], [BZ2_bzRead])
AC_CHECK_LIB([z], [gzread])

# Checks for header files.
AC_CHECK_HEADER([boost/iostreams/filter/gzip.hpp],,
                [AC_MSG_ERROR([Boost.Iostreams library not found])])
AC_CHECK_HEADER([boost/unordered_map.hpp],,
                [AC_MSG_ERROR([boost/unordered_map.hpp not found])])
AC_CHECK_HEADER([boost/spirit/include/classic_core.hpp],,
                [AC_MSG_ERROR([Boost.Spirit 2.x library not found])])

# Checks for typedefs, structures, and compiler characteristics.
AC_OPENMP

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
