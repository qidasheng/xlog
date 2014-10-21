#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([xlog], [2.0], [qsf.zzia@foxmail.com])
AM_INIT_AUTOMAKE(xlog,2.0)
AC_CONFIG_SRCDIR([proc_title.c])
AC_CONFIG_HEADERS([config.h])
#AC_CONFIG_FILES([Makefile]
# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h malloc.h netinet/in.h stdlib.h string.h sys/socket.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_CHECK_FUNCS([bzero memset socket strchr strdup strerror strstr])

AC_OUTPUT(Makefile)
