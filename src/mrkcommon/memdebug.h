#ifndef MRKCOMMON_MEMDEBUG_H
#define MRKCOMMON_MEMDEBUG_H

void *memdebug_malloc(size_t);
void *memdebug_calloc(size_t, size_t);
void *memdebug_realloc(void *, size_t);
void *memdebug_reallocf(void *, size_t);
void memdebug_free(void *);
size_t memdebug_nallocated(void);
char * memdebug_strdup(const char *);
char * memdebug_strndup(const char *, size_t);

#define malloc memdebug_malloc
#define calloc memdebug_calloc
#define realloc memdebug_realloc
#define reallocf memdebug_reallocf
#define free memdebug_free
#define strdup memdebug_strdup
#define strndup memdebug_strndup

#endif
