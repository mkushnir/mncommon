AC_INIT(mrkcommon, 0.1, markiyan.kushnir@gmail.com, mrkcommon, https://github.com/mkushnir)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.ac-aux])
AC_CONFIG_FILES([Makefile src/Makefile test/Makefile man/Makefile port/Makefile])
AC_CONFIG_HEADERS(config.h)

AS_IF([test -n "$USE_GCC"],
[
    probe="gcc${USE_GCC}"
    AS_IF([which $probe >/dev/null],
    [
        CC="gcc${USE_GCC}"
        CXX="c++${USE_GCC}"
        CPP="cpp"
    ],
    [
        AS_IF([which gcc >/dev/null],
        [
            CC="gcc"
            CXX="g++"
            CPP="cpp"
        ],
        [
            echo "Cannot find gcc"
            exit 1
        ])
    ])
    AC_SUBST(CC_DEBUG, ['-ftrapv'])
],
[
    AS_IF([which clang >/dev/null],
    [
        CC="clang -Qunused-arguments -fcolor-diagnostics"
        CXX="clang++ -Qunused-arguments -fcolor-diagnostics"
        CPP="clang-cpp"
        clang_version=`clang --version | grep version | awk '{print $4;}'`
        clang_version_mjr=`echo $clang_version | awk -F. '{print $1;}'`
        clang_version_mnr=`echo $clang_version | awk -F. '{print $2;}'`
        AS_IF([test "$clang_version_mjr" = "3" -a "$clang_version_mnr" -lt 3],
            [AC_SUBST(CC_DEBUG, ['-ftrapv -fcatch-undefined-behavior'])],
            [AC_SUBST(CC_DEBUG, ['-fsanitize=undefined-trap -fsanitize-undefined-trap-on-error -fno-sanitize=alignment'])])
        AC_SUBST(CC_LTO, ['-flto'])
        AC_SUBST(LD_LTO, ['-Wl,--lto-O3'])
    ],
    [
        CC="cc"
        CXX="c++"
        CPP="cpp"
        AC_SUBST(CC_DEBUG, ['-ftrapv'])
    ])
])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AS_CASE(["$target_os"],
    [*linux*], [AC_SUBST(_GNU_SOURCE_MACRO, ['-D_GNU_SOURCE'])],
    [AC_SUBST(_GNU_SOURCE_MACRO)]
)

AS_CASE(["$target_os"],
    [*darwin*], [AC_SUBST(_XOPEN_SOURCE_MACRO, ['-D_XOPEN_SOURCE'])],
    [AC_SUBST(_XOPEN_SOURCE_MACRO)]
)

AC_LANG(C)

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
AC_PROG_INSTALL

AM_INIT_AUTOMAKE([foreign subdir-objects])
LT_INIT
AM_SILENT_RULES([yes])

dnl AM_MAINTAINER_MODE

AC_CHECK_HEADERS([byteswap.h])
AC_CHECK_HEADERS([endian.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([limits.h])
AC_CHECK_HEADERS([malloc.h])
AC_CHECK_HEADERS([malloc_np.h])
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([sys/endian.h])
AC_CHECK_HEADERS([sys/limits.h])
AC_CHECK_HEADERS([sys/param.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([syslog.h])
AC_CHECK_HEADERS([float.h])
AC_CHECK_HEADER_STDBOOL
# FreeBSD
AC_CHECK_HEADERS([sys/tree.h])


AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

AC_FUNC_ERROR_AT_LINE
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD

AC_CHECK_FUNCS([clock_gettime])
AC_CHECK_FUNCS([flsl])
AC_CHECK_FUNCS([getpagesize])
AC_CHECK_FUNCS([malloc_usable_size])
AC_CHECK_FUNCS([memmove])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strncasecmp])
AC_CHECK_FUNCS([strndup])
AC_CHECK_FUNCS([strstr])
AC_CHECK_FUNCS([strtol])
# FreeBSD specific
AC_CHECK_FUNCS([reallocf])
AC_CHECK_FUNCS([sysctlbyname])
AC_CHECK_FUNCS([abort2])





AC_PRESERVE_HELP_ORDER

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug],
                             [Enable debug build (default=no)]),
              [AC_MSG_NOTICE([Will enable debug])],
              [AC_MSG_NOTICE([Will not enable debug])])
AM_CONDITIONAL([DEBUG], [test "$enable_debug" = "yes"])

AC_ARG_ENABLE(allstatic,
              AC_HELP_STRING([--enable-allstatic],
                             [Enable all static build (default=no)]),
              [AC_MSG_NOTICE([Will build statically])],
              [AC_MSG_NOTICE([Will not build statically])])

AM_CONDITIONAL([ALLSTATIC], [test "$enable_allstatic" = "yes"])

AC_ARG_ENABLE(lto,
              AC_HELP_STRING([--enable-lto],
                             [Enable all static build (default=no)]),
              [AC_MSG_NOTICE([Will enable LTO])],
              [AC_MSG_NOTICE([Will not enable LTO])])

AM_CONDITIONAL([LTO], [test "$enable_lto" = "yes"])

AC_ARG_ENABLE(array,
              AC_HELP_STRING([--enable-array],
                             [Enable resizable realloc(3)-based array
                              with iterators (default=yes)]),
              [enable_array=$enableval],
              [enable_array=yes])
AM_CONDITIONAL([ARRAY], [test "$enable_array" = "yes"])
AS_IF([test "$enable_array" = "yes"],
      [AC_MSG_NOTICE([Will enable array])],
      [AC_MSG_NOTICE([Will not enable array])])

AC_ARG_ENABLE(hash,
              AC_HELP_STRING([--enable-hash],
                             [Enable simple hash tables (default=yes)]),
              [enable_hash=$enableval],
              [enable_hash=yes])
AM_CONDITIONAL([HASH], [test "$enable_hash" = "yes"])
AS_IF([test "$enable_hash" = "yes"],
      [AC_MSG_NOTICE([Will enable hash])],
      [AC_MSG_NOTICE([Will not enable hash])])

AC_ARG_ENABLE(fasthash,
              AC_HELP_STRING([--enable-fasthash],
                             [Enable fasthash (default=yes)]),
              [enable_fasthash=$enableval],
              [enable_fasthash=yes])
AM_CONDITIONAL([FASTHASH], [test "$enable_fasthash" = "yes"])
AS_IF([test "$enable_fasthash" = "yes"],
      [AC_MSG_NOTICE([Will enable fasthash])],
      [AC_MSG_NOTICE([Will not enable fasthash])])

AC_ARG_ENABLE(profile,
              AC_HELP_STRING([--enable-profile],
                             [Enable profile (default=yes). Requires --enable-array]),
              [enable_profile=$enableval],
              [enable_profile=yes])
AM_CONDITIONAL([PROFILE], [test "$enable_profile" = "yes"])
AS_IF([test "$enable_profile" = "yes"],
      [AC_MSG_NOTICE([Will enable profile])],
      [AC_MSG_NOTICE([Will not enable profile])])

AC_ARG_ENABLE(trie,
              AC_HELP_STRING([--enable-trie],
                             [Enable trie (default=yes)]),
              [enable_trie=$enableval],
              [enable_trie=yes])
AM_CONDITIONAL([TRIE], [test "$enable_trie" = "yes"])
AS_IF([test "$enable_trie" = "yes"],
      [AC_MSG_NOTICE([Will enable trie])],
      [AC_MSG_NOTICE([Will not enable trie])])

AC_ARG_ENABLE(rbt,
              AC_HELP_STRING([--enable-rbt],
                             [Enable rbt (default=yes)]),
              [enable_rbt=$enableval],
              [enable_rbt=yes])
AM_CONDITIONAL([RBT], [test "$enable_rbt" = "yes"])
AS_IF([test "$enable_rbt" = "yes"],
      [AC_MSG_NOTICE([Will enable rbt])],
      [AC_MSG_NOTICE([Will not enable rbt])])

AC_ARG_ENABLE(traversedir,
              AC_HELP_STRING([--enable-traversedir],
                             [Enable traversedir (default=yes)]),
              [enable_traversedir=$enableval],
              [enable_traversedir=yes])
AM_CONDITIONAL([TRAVERSEDIR], [test "$enable_traversedir" = "yes"])
AS_IF([test "$enable_traversedir" = "yes"],
      [AC_MSG_NOTICE([Will enable traversedir])],
      [AC_MSG_NOTICE([Will not enable traversedir])])

AC_ARG_ENABLE(bytestream,
              AC_HELP_STRING([--enable-bytestream],
                             [Enable bytestream (default=yes)]),
              [enable_bytestream=$enableval],
              [enable_bytestream=yes])
AM_CONDITIONAL([BYTESTREAM], [test "$enable_bytestream" = "yes"])
AS_IF([test "$enable_bytestream" = "yes"],
      [AC_MSG_NOTICE([Will enable bytestream])],
      [AC_MSG_NOTICE([Will not enable bytestream])])

AC_ARG_ENABLE(json,
              AC_HELP_STRING([--enable-json],
                             [Enable json (default=yes)]),
              [enable_json=$enableval],
              [enable_json=yes])
AM_CONDITIONAL([JSON], [test "$enable_json" = "yes"])
AS_IF([test "$enable_json" = "yes"],
      [AC_MSG_NOTICE([Will enable json])],
      [AC_MSG_NOTICE([Will not enable json])])

AC_ARG_ENABLE(stqueue,
              AC_HELP_STRING([--enable-stqueue],
                             [Enable stqueue (default=yes)]),
              [enable_stqueue=$enableval],
              [enable_stqueue=yes])
AM_CONDITIONAL([STQUEUE], [test "$enable_stqueue" = "yes"])
AS_IF([test "$enable_stqueue" = "yes"],
      [AC_MSG_NOTICE([Will enable stqueue])],
      [AC_MSG_NOTICE([Will not enable stqueue])])

AC_ARG_ENABLE(dtqueue,
              AC_HELP_STRING([--enable-dtqueue],
                             [Enable dtqueue (default=yes)]),
              [enable_dtqueue=$enableval],
              [enable_dtqueue=yes])
AM_CONDITIONAL([DTQUEUE], [test "$enable_dtqueue" = "yes"])
AS_IF([test "$enable_dtqueue" = "yes"],
      [AC_MSG_NOTICE([Will enable dtqueue])],
      [AC_MSG_NOTICE([Will not enable dtqueue])])

AC_ARG_ENABLE(memdebug,
              AC_HELP_STRING([--enable-memdebug],
                             [Enable memdebug (default=yes)]),
              [enable_memdebug=$enableval],
              [enable_memdebug=yes])
AM_CONDITIONAL([MEMDEBUG], [test "$enable_memdebug" = "yes"])
AS_IF([test "$enable_memdebug" = "yes"],
      [AC_MSG_NOTICE([Will enable memdebug])],
      [AC_MSG_NOTICE([Will not enable memdebug])])

AC_ARG_ENABLE(mpool,
              AC_HELP_STRING([--enable-mpool],
                             [Enable mpool (default=yes)]),
              [enable_mpool=$enableval],
              [enable_mpool=yes])
AM_CONDITIONAL([MPOOL], [test "$enable_mpool" = "yes"])
AS_IF([test "$enable_mpool" = "yes"],
      [AC_MSG_NOTICE([Will enable mpool])],
      [AC_MSG_NOTICE([Will not enable mpool])])

AC_ARG_ENABLE(bytes,
              AC_HELP_STRING([--enable-bytes],
                             [Enable bytes (default=yes)]),
              [enable_bytes=$enableval],
              [enable_bytes=yes])
AM_CONDITIONAL([BYTES], [test "$enable_bytes" = "yes"])
AS_IF([test "$enable_bytes" = "yes"],
      [AC_MSG_NOTICE([Will enable bytes])],
      [AC_MSG_NOTICE([Will not enable bytes])])

AC_ARG_ENABLE(cm,
              AC_HELP_STRING([--enable-cm],
                             [Enable count-min (default=yes)]),
              [enable_cm=$enableval],
              [enable_cm=yes])
AM_CONDITIONAL([CM], [test "$enable_cm" = "yes"])
AS_IF([test "$enable_cm" = "yes"],
      [AC_MSG_NOTICE([Will enable cm])],
      [AC_MSG_NOTICE([Will not enable cm])])

AC_ARG_ENABLE(heap,
              AC_HELP_STRING([--enable-heap],
                             [Enable heap (default=yes)]),
              [enable_heap=$enableval],
              [enable_heap=yes])
AM_CONDITIONAL([HEAP], [test "$enable_heap" = "yes"])
AS_IF([test "$enable_heap" = "yes"],
      [AC_MSG_NOTICE([Will enable heap])],
      [AC_MSG_NOTICE([Will not enable heap])])

AC_ARG_ENABLE(base64,
              AC_HELP_STRING([--enable-base64],
                             [Enable base64 (default=yes)]),
              [enable_base64=$enableval],
              [enable_base64=yes])
AM_CONDITIONAL([BASE64], [test "$enable_base64" = "yes"])
AS_IF([test "$enable_base64" = "yes"],
      [AC_MSG_NOTICE([Will enable base64])],
      [AC_MSG_NOTICE([Will not enable base64])])

AC_ARG_WITH(cmocka,
            AC_HELP_STRING([--with-cmocka],
            [Build CMocka tests (default=no)]),
            [AM_CONDITIONAL([CMOCKA], [with_cmocka=yes])],
            [AM_CONDITIONAL([CMOCKA], [test "\$with_cmocka" = "yes"])])

AS_IF([! test \( "$enable_hash" = "yes" -a "$enable_fasthash" = "yes" \) -o \
        \( "$enable_hash" != "yes" -a "$enable_fasthash" != "yes" \)],
      [AC_MSG_FAILURE(
        [Cannot disable either of hash/fasthash feature while the other one is enabled.])])

AS_IF([test "$enable_cm" = "yes" -a \( "$enable_hash" != "yes" -o "$enable_fasthash" != "yes" \)],
      [AC_MSG_FAILURE([Cannot disable the hash feature because the cm feature requires it.])])

AC_OUTPUT
