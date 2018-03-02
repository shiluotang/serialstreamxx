#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([socketstream], [0.1], [grafterman@googlemail.com])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([src/socketstream.hpp])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Checks for programs.
AC_PROG_CXX
m4_pattern_allow([AX_CXX_COMPILE_STDCXX])
AX_CXX_COMPILE_STDCXX([11], [ext], [optional])
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR
LT_INIT

# Checks for libraries.
AC_CHECK_LIB([ws2_32],[main])
AC_CHECK_LIB([mswsock],[main])
AC_CHECK_LIB([wsock32],[main])

# Checks for header files.
AC_CHECK_HEADERS([netdb.h netinet/in.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UINT16_T

# Checks for library functions.
AC_CHECK_FUNCS([gethostbyname socket])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
