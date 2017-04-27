#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([rfcreader], [0.3], [naidv88@gmail.com])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([locale.h stdlib.h string.h sys/ioctl.h termios.h])
AC_CHECK_LIB([curses],[newwin])
AC_CHECK_LIB([menu],[new_menu])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([memset setlocale strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
