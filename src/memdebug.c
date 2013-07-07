#include <stdlib.h>
#include <string.h>
#include <malloc_np.h>

static size_t nallocated = 0;

void *
memdebug_malloc(size_t sz)
{
    void *res;

    res = malloc(sz);

    if (res != NULL) {
        nallocated += malloc_usable_size(res);
    }

    return res;
}

void *
memdebug_calloc(size_t n, size_t sz)
{
    void *res;

    res = calloc(n, sz);

    if (res != NULL) {
        nallocated += malloc_usable_size(res);
    }

    return res;
}

void *
memdebug_realloc(void * ptr, size_t sz)
{
    void *res;

    if (ptr != NULL) {
        nallocated -= malloc_usable_size(ptr);
    }

    res = realloc(ptr, sz);

    if (res != NULL) {
        nallocated += malloc_usable_size(res);
    }

    return res;
}

void *
memdebug_reallocf(void * ptr, size_t sz)
{
    void *res;

    if (ptr != NULL) {
        nallocated -= malloc_usable_size(ptr);
    }

    res = reallocf(ptr, sz);

    if (res != NULL) {
        nallocated += malloc_usable_size(res);
    }

    return res;
}

void
memdebug_free(void *ptr)
{
    if (ptr != NULL) {
        nallocated -= malloc_usable_size(ptr);
    }

    free(ptr);
}

char *
memdebug_strdup(const char *str)
{
    char *res;

    res = strdup(str);

    if (res != NULL) {
        nallocated += malloc_usable_size(res);
    }

    return res;
}

char *
memdebug_strndup(const char *str, size_t len)
{
    char *res;

    res = strndup(str, len);

    if (res != NULL) {
        nallocated += malloc_usable_size(res);
    }

    return res;
}

size_t
memdebug_nallocated(void)
{
    return nallocated;
}
