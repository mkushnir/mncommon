AC_INIT(mrkcommon, 0.1, markiyan.kushnir@gmail.com)
if test -n "$USE_GCC"
then
    if test "$USE_GCC" = "46" && which gcc46
    then
        CC="gcc46"
        CXX="c++46"
        CPP="cpp"
    else
        if which gcc
        then
            CC="gcc"
            CXX="g++"
            CPP="cpp"
        else
            echo "Cannot find gcc"
            exit 1
        fi
    fi
else
    if which clang >/dev/null
    then
        CC="clang -Qunused-arguments -fcolor-diagnostics"
        CXX="clang++ -Qunused-arguments -fcolor-diagnostics"
        CPP="clang-cpp"
        clang_version=`clang --version | grep version | awk '{print $4;}'`
        clang_version_mjr=`echo $clang_version | awk -F. '{print $1;}'`
        clang_version_mnr=`echo $clang_version | awk -F. '{print $2;}'`
        if test "$clang_version_mjr" = "3" -a "$clang_version_mnr" -lt 3
        then
            AC_SUBST(CLANG_DEBUG, ['-ftrapv -fcatch-undefined-behavior'])
        else
            # we assume all others are 3.3+
            AC_SUBST(CLANG_DEBUG, ['-fsanitize=undefined-trap -fsanitize-undefined-trap-on-error -fno-sanitize=alignment'])
        fi
    else
        CC="cc"
        CXX="c++"
        CPP="cpp"
    fi
fi

AC_CANONICAL_HOST
AC_CANONICAL_TARGET
case "$target_os" in
    *linux*)
        AC_SUBST(_GNU_SOURCE_MACRO, ['-D_GNU_SOURCE'])
    ;;
    *)
        AC_SUBST(_GNU_SOURCE_MACRO)
    ;;
esac

AC_CONFIG_MACRO_DIR([m4])

AC_LANG(C)

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_INSTALL

LT_INIT
AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

dnl AM_MAINTAINER_MODE

AC_CONFIG_FILES([Makefile src/Makefile test/Makefile man/Makefile])
AC_CONFIG_HEADERS(config.h)

AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/param.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([limits.h])
AC_CHECK_HEADERS([sys/limits.h])
AC_CHECK_HEADERS([malloc_np.h])
# FreeBSD
AC_CHECK_HEADERS([sys/tree.h])


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
AC_FUNC_STRTOD
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strndup])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([strtol])
AC_CHECK_FUNCS([flsl])
AC_CHECK_FUNCS([malloc_usable_size])
# FreeBSD specific
AC_CHECK_FUNCS([reallocf])
AC_CHECK_FUNCS([sysctlbyname])



AC_PRESERVE_HELP_ORDER

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug],
                             [Enable debug build (default=no)]),
              [AM_CONDITIONAL([DEBUG], [enable_debug=yes])],
              [AM_CONDITIONAL([DEBUG], [test "$enable_debug" = "yes"])]
             )

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

AC_ARG_ENABLE(dict,
              AC_HELP_STRING([--enable-dict],
                             [Enable simple dictionary (default=yes)]),
              [AM_CONDITIONAL([DICT], [test "$enable_dict" = "yes"])],
              [AM_CONDITIONAL([DICT], [enable_dict=yes])],
             )

AC_ARG_ENABLE(fasthash,
              AC_HELP_STRING([--enable-fasthash],
                             [Enable fasthash (default=yes)]),
              [AM_CONDITIONAL([FASTHASH], [test "$enable_fasthash" = "yes"])],
              [AM_CONDITIONAL([FASTHASH], [enable_fasthash=yes])],
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

AC_ARG_ENABLE(json,
              AC_HELP_STRING([--enable-json],
                             [Enable json (default=yes)]),
              [AM_CONDITIONAL([JSON], [test "$enable_json" = "yes"])],
              [AM_CONDITIONAL([JSON], [enable_json=yes])],
             )

AC_ARG_ENABLE(stqueue,
              AC_HELP_STRING([--enable-stqueue],
                             [Enable stqueue (default=yes)]),
              [AM_CONDITIONAL([STQUEUE], [test "$enable_stqueue" = "yes"])],
              [AM_CONDITIONAL([STQUEUE], [enable_stqueue=yes])],
             )

AC_ARG_ENABLE(dtqueue,
              AC_HELP_STRING([--enable-dtqueue],
                             [Enable dtqueue (default=yes)]),
              [AM_CONDITIONAL([DTQUEUE], [test "$enable_dtqueue" = "yes"])],
              [AM_CONDITIONAL([DTQUEUE], [enable_dtqueue=yes])],
             )

AC_ARG_ENABLE(memdebug,
              AC_HELP_STRING([--enable-memdebug],
                             [Enable memdebug (default=yes)]),
              [AM_CONDITIONAL([MEMDEBUG], [test "$enable_memdebug" = "yes"])],
              [AM_CONDITIONAL([MEMDEBUG], [enable_memdebug=yes])],
             )

AC_ARG_ENABLE(mpool,
              AC_HELP_STRING([--enable-mpool],
                             [Enable mpool (default=yes)]),
              [AM_CONDITIONAL([MPOOL], [test "$enable_mpool" = "yes"])],
              [AM_CONDITIONAL([MPOOL], [enable_mpool=yes])],
             )

AC_ARG_ENABLE(bytes,
              AC_HELP_STRING([--enable-bytes],
                             [Enable bytes (default=yes)]),
              [AM_CONDITIONAL([BYTES], [test "$enable_bytes" = "yes"])],
              [AM_CONDITIONAL([BYTES], [enable_bytes=yes])],
             )

AC_ARG_ENABLE(cm,
              AC_HELP_STRING([--enable-cm],
                             [Enable count-min (default=yes)]),
              [AM_CONDITIONAL([CM], [test "$enable_cm" = "yes"])],
              [AM_CONDITIONAL([CM], [enable_cm=yes])],
             )

AC_ARG_ENABLE(heap,
              AC_HELP_STRING([--enable-heap],
                             [Enable heap (default=yes)]),
              [AM_CONDITIONAL([HEAP], [test "$enable_heap" = "yes"])],
              [AM_CONDITIONAL([HEAP], [enable_heap=yes])],
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

if test "$enable_cm" = "yes" -a \( "$enable_dict" != "yes" -o "$enable_fasthash" != "yes" \)
then
    echo "ERROR: cannot disable the dict feature because the cm feature requires it."
    exit 1
fi

AC_OUTPUT
