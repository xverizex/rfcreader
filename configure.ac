#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([rfcreader], [0.4], [xverizex@xverizex.asuscomm.com])
AC_CONFIG_SRCDIR([settings.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([curses],[newwin])
AC_CHECK_LIB([menu],[new_menu])
AC_CHECK_LIB([ssl],[SSL_library_init])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h locale.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h termios.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([gethostbyname memset mkdir setlocale socket strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
