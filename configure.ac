AC_INIT(mrkcommon, 0.1, markiyan.kushnir@gmail.com)
if test -z "$USE_GCC"
then
    CC="clang -Qunused-arguments -fcolor-diagnostics"
    CXX="clang++ -Qunused-arguments -fcolor-diagnostics"
    CPP="clang -E"
fi

AC_CONFIG_MACRO_DIR([m4])

AC_LANG(C)

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_INSTALL

LT_INIT
AM_INIT_AUTOMAKE

dnl AM_MAINTAINER_MODE

AC_CONFIG_FILES([Makefile src/Makefile src/test/Makefile man/Makefile])
AC_CONFIG_HEADERS(config.h)

AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/param.h])

AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_OFF_T
AC_TYPE_SSIZE_T

AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_ERROR_AT_LINE
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([memset])



AC_PRESERVE_HELP_ORDER

AC_ARG_ENABLE(array,
              AC_HELP_STRING([--enable-array],
                             [Enable resizable realloc(3)-based array
                              with iterators (default=yes)]),
              [AM_CONDITIONAL([ARRAY], [test "$enable_array" = "yes"])],
              [AM_CONDITIONAL([ARRAY], [enable_array=yes])],
             )
AC_ARG_ENABLE(list,
              AC_HELP_STRING([--enable-list],
                             [Enable linked list (default=yes)]),
              [AM_CONDITIONAL([LIST], [test "$enable_list" = "yes"])],
              [AM_CONDITIONAL([LIST], [enable_list=yes])],
             )
AC_ARG_ENABLE(logging,
              AC_HELP_STRING([--enable-logging],
                             [Enable logging (default=yes)]),
              [AM_CONDITIONAL([LOGGING], [test "$enable_logging" = "yes"])],
              [AM_CONDITIONAL([LOGGING], [enable_logging=yes])],
             )
AC_ARG_ENABLE(profile,
              AC_HELP_STRING([--enable-profile],
                             [Enable profile (default=yes). Requires --enable-list]),
              [AM_CONDITIONAL([PROFILE], [test "$enable_profile" = "yes"])],
              [AM_CONDITIONAL([PROFILE], [enable_profile=yes])],
             )

AC_ARG_ENABLE(conf,
              AC_HELP_STRING([--enable-conf],
                             [Enable conf (default=yes). Requires --enable-array]),
              [AM_CONDITIONAL([CONF], [test "$enable_conf" = "yes"])],
              [AM_CONDITIONAL([CONF], [enable_conf=yes])],
             )

AC_ARG_ENABLE(trie,
              AC_HELP_STRING([--enable-trie],
                             [Enable trie (default=yes)]),
              [AM_CONDITIONAL([TRIE], [test "$enable_trie" = "yes"])],
              [AM_CONDITIONAL([TRIE], [enable_trie=yes])],
             )

AC_ARG_ENABLE(rbt,
              AC_HELP_STRING([--enable-rbt],
                             [Enable rbt (default=yes)]),
              [AM_CONDITIONAL([RBT], [test "$enable_rbt" = "yes"])],
              [AM_CONDITIONAL([RBT], [enable_rbt=yes])],
             )

AC_ARG_ENABLE(traversedir,
              AC_HELP_STRING([--enable-traversedir],
                             [Enable traversedir (default=yes)]),
              [AM_CONDITIONAL([TRAVERSEDIR], [test "$enable_traversedir" = "yes"])],
              [AM_CONDITIONAL([TRAVERSEDIR], [enable_traversedir=yes])],
             )

AC_ARG_ENABLE(bytestream,
              AC_HELP_STRING([--enable-bytestream],
                             [Enable bytestream (default=yes)]),
              [AM_CONDITIONAL([BYTESTREAM], [test "$enable_bytestream" = "yes"])],
              [AM_CONDITIONAL([BYTESTREAM], [enable_bytestream=yes])],
             )

if test "$enable_profile" = "yes" -a "$enable_list" != "yes"
then
    echo "ERROR: cannot disable the list feature because the profile feature requires it."
    exit 1
fi

if test "$enable_conf" = "yes" -a "$enable_array" != "yes"
then
    echo "ERROR: cannot disable the array feature because the conf feature requires it."
    exit 1
fi

AC_OUTPUT
