# configure.ac
AC_PREREQ([2.69])   # Minimum version of autoconf
AC_INIT([xbuild], [1.0], [rongsong.shen@gmail.com])  # Project info
AM_INIT_AUTOMAKE([foreign])  # Use Automake in 'foreign' mode (no strict requirements)

AC_ARG_VAR([libexecdir], [directory for internal executables])
AC_SUBST([libexecdir], [$libexecdir])

# Check for necessary tools (optional)
AC_PROG_CC
AC_PROG_INSTALL  # Ensure 'install' command is available

if test "x$libexecdir" = "x"; then
   libexecdir='${prefix}/libexec'
fi

# Output the configuration for the Makefile
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

