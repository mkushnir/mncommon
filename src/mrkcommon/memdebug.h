#ifndef MRKCOMMON_MEMDEBUG_H
#define MRKCOMMON_MEMDEBUG_H

#ifdef MEMDEBUG_SIMPLE

void *memdebug_malloc(size_t);
void *memdebug_calloc(size_t, size_t);
void *memdebug_realloc(void *, size_t);
void *memdebug_reallocf(void *, size_t);
void memdebug_free(void *);
size_t memdebug_nallocated(void);
char *memdebug_strdup(const char *);
char *memdebug_strndup(const char *, size_t);

#define MEMDEBUG_DECLARE(n)

#define MEMDEBUG_REGISTER(n)

#define malloc memdebug_malloc
#define calloc memdebug_calloc
#define realloc memdebug_realloc
#define reallocf memdebug_reallocf
#define free memdebug_free
#define strdup memdebug_strdup
#define strndup memdebug_strndup

#else /* MEMDEBUG_SIMPLE */

int memdebug_register(const char *);
void *memdebug_malloc_named(int, size_t);
void *memdebug_calloc_named(int, size_t, size_t);
void *memdebug_realloc_named(int, void *, size_t);
void *memdebug_reallocf_named(int, void *, size_t);
void memdebug_free_named(int, void *);
size_t memdebug_nallocated_named(int n);
char *memdebug_strdup_named(int, const char *);
char *memdebug_strndup_named(int, const char *, size_t);

#define CONCAT(a, b) a##b

#define _DECLARE_MALLOC(n) \
static void *__malloc(size_t sz) { return memdebug_malloc_named(CONCAT(__memdebug_id_, n), sz); }

#define _DECLARE_CALLOC(n) \
static void *__calloc(size_t e, size_t sz) { return memdebug_calloc_named(CONCAT(__memdebug_id_,n), e, sz); }

#define _DECLARE_REALLOC(n) \
static void *__realloc(void *ptr, size_t sz) { return memdebug_realloc_named(CONCAT(__memdebug_id_,n), ptr, sz); }

#define _DECLARE_REALLOCF(n) \
static void *__reallocf(void *ptr, size_t sz) { return memdebug_reallocf_named(CONCAT(__memdebug_id_,n), ptr, sz); }

#define _DECLARE_FREE(n) \
static void __free(void *ptr) { return memdebug_free_named(CONCAT(__memdebug_id_,n), ptr); }

#define _DECLARE_STRDUP(n) \
static char *__strdup(const char *str) { return memdebug_strdup_named(CONCAT(__memdebug_id_,n), str); }

#define _DECLARE_STRNDUP(n) \
static char *__strndup(const char *str, size_t len) { return memdebug_strndup_named(CONCAT(__memdebug_id_,n), str, len); }

#define _DECLARE_NALLOCATED(n) \
static size_t __memdebug_nallocated(void) { return memdebug_nallocated_named(CONCAT(__memdebug_id_,n)); }


#define MEMDEBUG_DECLARE(n) \
static int CONCAT(__memdebug_id_, n) = 0; \
_DECLARE_MALLOC(n); \
_DECLARE_CALLOC(n); \
_DECLARE_REALLOC(n); \
_DECLARE_REALLOCF(n); \
_DECLARE_FREE(n); \
_DECLARE_STRDUP(n); \
_DECLARE_STRNDUP(n); \
_DECLARE_NALLOCATED(n);



#define MEMDEBUG_REGISTER(n) \
    CONCAT(__memdebug_id_, n) = memdebug_register(#n)

#define malloc __malloc
#define calloc __calloc
#define realloc __realloc
#define reallocf __reallocf
#define free __free
#define strdup __strdup
#define strndup __strndup
#define memdebug_nallocated __memdebug_nallocated

#endif /* !MEMDEBUG_SIMPLE */

#endif
