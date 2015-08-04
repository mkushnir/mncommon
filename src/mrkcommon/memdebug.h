#ifndef MRKCOMMON_MEMDEBUG_H
#define MRKCOMMON_MEMDEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _memdebug_stat {
    const char *name;
    size_t nallocated;
    size_t nitems;
} memdebug_stat_t;

int memdebug_register(const char *);
void memdebug_clear(void);
int memdebug_set_runtime_scope(int);
#define MEMDEBUG_SET_RUNTIME_SCOPE(n) \
    memdebug_set_runtime_scope(CONCAT(__memdebug_id_,n))

#define MEMDEBUG_CLEAR_RUNTIME_SCOPE() memdebug_set_runtime_scope(-1)
int memdebug_get_runtime_scope(void);
void *memdebug_malloc(int, size_t);
void *memdebug_calloc(int, size_t, size_t);
void *memdebug_realloc(int, void *, size_t);
void *memdebug_reallocf(int, void *, size_t);
void memdebug_free(int, void *);
char *memdebug_strdup(int, const char *);
char *memdebug_strndup(int, const char *, size_t);
void memdebug_stat(int, memdebug_stat_t *);
void memdebug_traverse_ctxes(int (*cb)(memdebug_stat_t *, void *), void *);
void memdebug_print_stats(void);
void memdebug_print_stats_oneline(void);

#define CONCAT(a, b) a##b

#ifndef UNUSED
#define UNUSED __attribute__ ((unused))
#endif

#define _DECLARE_MALLOC(n) \
UNUSED static void *__memdebug_malloc(size_t sz) { return memdebug_malloc(CONCAT(__memdebug_id_,n), sz); }

#define _DECLARE_CALLOC(n) \
UNUSED static void *__memdebug_calloc(size_t e, size_t sz) { return memdebug_calloc(CONCAT(__memdebug_id_,n), e, sz); }

#define _DECLARE_REALLOC(n) \
UNUSED static void *__memdebug_realloc(void *ptr, size_t sz) { return memdebug_realloc(CONCAT(__memdebug_id_,n), ptr, sz); }

#define _DECLARE_REALLOCF(n) \
UNUSED static void *__memdebug_reallocf(void *ptr, size_t sz) { return memdebug_reallocf(CONCAT(__memdebug_id_,n), ptr, sz); }

#define _DECLARE_FREE(n) \
UNUSED static void __memdebug_free(void *ptr) { return memdebug_free(CONCAT(__memdebug_id_,n), ptr); }

#define _DECLARE_STRDUP(n) \
UNUSED static char *__memdebug_strdup(const char *str) { return memdebug_strdup(CONCAT(__memdebug_id_,n), str); }

#define _DECLARE_STRNDUP(n) \
UNUSED static char *__memdebug_strndup(const char *str, size_t len) { return memdebug_strndup(CONCAT(__memdebug_id_,n), str, len); }

#define _DECLARE_NALLOCATED(n) \
UNUSED static size_t __memdebug_nallocated(void) { memdebug_stat_t st; memdebug_stat(CONCAT(__memdebug_id_,n), &st); return st.nallocated; }


#define MEMDEBUG_DEFINE_ID(n)          \
int CONCAT(__memdebug_id_, n) = 0;     \


#define MEMDEBUG_DECLARE(n)            \
MEMDEBUG_DEFINE_ID(n)                  \
_DECLARE_MALLOC(n);                    \
_DECLARE_CALLOC(n);                    \
_DECLARE_REALLOC(n);                   \
_DECLARE_REALLOCF(n);                  \
_DECLARE_FREE(n);                      \
_DECLARE_STRDUP(n);                    \
_DECLARE_STRNDUP(n);                   \
_DECLARE_NALLOCATED(n);                \


#define MEMDEBUG_REGISTER(n)                           \
    extern int CONCAT(__memdebug_id_, n);              \
    CONCAT(__memdebug_id_, n) = memdebug_register(#n)  \


#define malloc __memdebug_malloc
#define calloc __memdebug_calloc
#define realloc __memdebug_realloc
#define reallocf __memdebug_reallocf
#define free __memdebug_free
#ifdef strdup
#undef strdup
#endif
#define strdup __memdebug_strdup
#ifdef strndup
#undef strndup
#endif
#define strndup __memdebug_strndup
#define memdebug_nallocated __memdebug_nallocated

#ifdef __cplusplus
}
#endif

#endif
