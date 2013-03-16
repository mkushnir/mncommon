#include <assert.h>
#include <stdlib.h>

#include "mrkcommon/dumpm.h"
#include "mrkcommon/list.h"
#include "diag.h"


static int
list_data_init(list_t *lst)
{
    unsigned i;
    struct _dlist_entry *tmp;

    if (lst->elnum > 0) {
        if ((lst->index = malloc(sizeof(struct _dlist_entry *) * lst->elnum)) == NULL) {
            TRRET(DLIST_INIT + 1);
        }

        if ((lst->data.head = malloc(sizeof(struct _dlist_entry) + lst->elsz)) == NULL) {
            TRRET(DLIST_INIT + 2);
        }

        lst->data.head->prev = NULL;
        lst->data.head->next = NULL;
        if (lst->init != NULL) {
            if (lst->init(DLE_DATA(lst->data.head)) != 0) {
                TRRET(DLIST_INIT + 3);
            }
        }
        lst->data.tail = lst->data.head;
        lst->index[0] = lst->data.head;

        if (lst->elnum > 1) {
            tmp = lst->data.tail;
            for (i = 1; i < lst->elnum; ++i) {
                if ((lst->data.tail = malloc(sizeof(struct _dlist_entry) + lst->elsz)) == NULL) {
                    TRRET(DLIST_INIT + 3);
                }
                lst->data.tail->next = NULL;
                tmp->next = lst->data.tail;
                lst->data.tail->prev = tmp;
                if (lst->init != NULL) {
                    if (lst->init(DLE_DATA(lst->data.tail)) != 0) {
                        TRRET(DLIST_INIT + 4);
                    }
                }
                lst->index[i] = lst->data.tail;
                tmp = lst->data.tail;
            }
        }

    } else {
        lst->index = NULL;
        lst->data.head = NULL;
        lst->data.tail = NULL;
    }
    return 0;
}

static int
list_data_fini(list_t *lst)
{
    struct _dlist_entry *tmp;

    lst->data.tail = NULL;

    if (lst->fini != NULL) {
        for (;
             lst->data.head != NULL;
             lst->data.head = tmp) {

            tmp = lst->data.head->next;
            lst->fini(DLE_DATA(lst->data.head));
            free(lst->data.head);
            lst->data.head = tmp;
        }
    } else {
        for (;
             lst->data.head != NULL;
             lst->data.head = tmp) {

            tmp = lst->data.head->next;
            free(lst->data.head);
            lst->data.head = tmp;
        }
    }
    free(lst->index);
    lst->index = NULL;
    lst->elnum = 0;
    return 0;
}


/*
 * Initialize list structure.
 *
 */
int
list_init (list_t *lst, size_t elsz, size_t elnum,
            list_initializer_t init,
            list_finalizer_t fini)
{
    assert(elsz > 0);
    lst->elsz = elsz;
    lst->elnum = elnum;
    lst->init = init;
    lst->fini = fini;
    lst->compar = NULL;
    if (list_data_init(lst) != 0) {
        TRRET(LIST_INIT_ + 1);
    }
    return 0;
}

/*
 * Make a full copy of the src into dst, and clean up src.
 */
int
list_move(list_t *dst, list_t *src)
{
    list_fini(dst);

    dst->elsz = src->elsz;
    src->elsz = 0;
    dst->elnum = src->elnum;
    src->elnum = 0;
    dst->data = src->data; /* pass a reference*/
    src->data.head = NULL;
    src->data.tail = NULL;
    dst->index = src->index; /* pass a reference*/
    src->index = NULL;
    dst->init = src->init;
    src->init = NULL;
    dst->fini = src->fini;
    src->fini = NULL;
    dst->compar = src->compar;
    src->compar = NULL;
    return 0;
}

static int
list_data_reinit(list_t *lst)
{
    struct _dlist_entry *tmp;

    if (lst->fini != NULL && lst->init == NULL) {
        for (tmp = lst->data.head;
             tmp != NULL;
             tmp = tmp->next) {
            lst->fini(DLE_DATA(tmp));
        }
    } else if (lst->fini != NULL && lst->init != NULL) {
        for (tmp = lst->data.head;
             tmp != NULL;
             tmp = tmp->next) {
            lst->fini(DLE_DATA(tmp));
            if (lst->init(DLE_DATA(tmp)) != 0) {
                TRRET(LIST_REINIT + 1);
            }
        }
    } else if (lst->fini == NULL && lst->init != NULL) {
        for (tmp = lst->data.head;
             tmp != NULL;
             tmp = tmp->next) {
            if (lst->init(DLE_DATA(tmp)) != 0) {
                TRRET(LIST_REINIT + 2);
            }
        }
    }
    return 0;
}

/*
 * Make sure list is at least len long.
 */
int
list_ensure_len(list_t *lst, size_t newelnum, unsigned int flags)
{
    struct _dlist_entry *tmp;
    unsigned i, adjust;
    void *vtmp;

    if (newelnum > lst->elnum) {
        /* re-init the existing entries */
        if (!(flags & LIST_FLAG_SAVE)) {
            if (list_data_reinit(lst) != 0) {
                TRRET(LIST_ENSURE_LEN + 1);
            }
        }

        /* append new entries */
        if ((vtmp = realloc(lst->index,
                                 sizeof(struct _elist_entry *) * newelnum))
                                 == NULL) {
            TRRET(LIST_ENSURE_LEN + 2);
        }
        lst->index = vtmp;

        if (lst->elnum == 0) {
            if ((lst->data.head = malloc(sizeof(struct _dlist_entry) +
                                        lst->elsz)) == NULL) {
                TRRET(LIST_ENSURE_LEN + 3);
            }
            lst->data.head->prev = NULL;
            lst->data.head->next = NULL;
            lst->data.tail = lst->data.head;

            if (lst->init != NULL) {
                if (lst->init(DLE_DATA(lst->data.head)) != 0) {
                    TRRET(LIST_ENSURE_LEN + 4);
                }
            }
            lst->index[0] = lst->data.head;

            adjust = 1;

        } else {
            adjust = 0;
        }

        tmp = lst->data.tail;
        assert(tmp != NULL);

        if (lst->init != NULL) {
            for (i = lst->elnum + adjust; i < newelnum; ++i) {

                if ((lst->data.tail = malloc(sizeof(struct _dlist_entry) +
                                            lst->elsz)) == NULL) {
                    TRRET(LIST_ENSURE_LEN + 5);
                }

                lst->data.tail->next = NULL;
                lst->data.tail->prev = tmp;
                tmp->next = lst->data.tail;

                if (lst->init(DLE_DATA(lst->data.tail)) != 0) {
                    TRRET(LIST_ENSURE_LEN + 6);
                }

                lst->index[i] = lst->data.tail;
                tmp = lst->data.tail;
            }
        } else {
            for (i = lst->elnum + adjust; i < newelnum; ++i) {

                if ((lst->data.tail = malloc(sizeof(struct _dlist_entry) +
                                            lst->elsz)) == NULL) {
                    TRRET(LIST_ENSURE_LEN + 7);
                }

                lst->data.tail->next = NULL;
                lst->data.tail->prev = tmp;
                tmp->next = lst->data.tail;

                lst->index[i] = lst->data.tail;
                tmp = lst->data.tail;
            }
        }

        lst->elnum = newelnum;

    } else {
        /* remove excess entries */

        if (newelnum < lst->elnum) {
            if (newelnum > 0) {
                struct _dlist_entry *tmp_next;

                lst->data.tail = lst->index[newelnum]->prev;
                lst->data.tail->next = NULL;

                if (lst->fini != NULL) {

                    for (tmp = lst->index[newelnum];
                         tmp != NULL;
                         tmp = tmp_next) {

                        tmp_next = tmp->next;
                        lst->fini(DLE_DATA(tmp));
                        free(tmp);
                    }
                } else {
                    for (tmp = lst->index[newelnum];
                         tmp != NULL;
                         tmp = tmp_next) {

                        tmp_next = tmp->next;
                        free(tmp);
                    }
                }

                if ((vtmp = realloc(lst->index,
                                         sizeof(struct _dlist_entry *) *
                                         newelnum)) == NULL) {
                    TRRET(LIST_ENSURE_LEN + 8);
                }
                lst->index = vtmp;

                lst->elnum = newelnum;

                /* re-init the existing entries */
                if (!(flags & LIST_FLAG_SAVE)) {
                    if (list_data_reinit(lst) != 0) {
                        TRRET(LIST_ENSURE_LEN + 9);
                    }
                }


            } else {
                /* newelnum == 0 */
                list_data_fini(lst);
            }

        } else {
            /* newelnum == lst->elnum */

            /* re-init the existing entries */
            if (!(flags & LIST_FLAG_SAVE)) {
                if (list_data_reinit(lst) != 0) {
                    TRRET(LIST_ENSURE_LEN + 10);
                }
            }
        }
    }


    return 0;
}

int
list_clear_item(list_t *lst, unsigned idx)
{
    //TRACE("idx=%d elnum=%ld", idx, lst->elnum);
    if (idx >= lst->elnum) {
        TRRET(LIST_CLEAR_ITEM + 1);
    }
    if (lst->fini != NULL) {
        return lst->fini(DLE_DATA(lst->index[idx]));
    }
    return 0;
}

void *
list_get(list_t *lst, unsigned idx)
{
    if (idx < lst->elnum) {
        return DLE_DATA(lst->index[idx]);
    }
    return NULL;
}

void *
list_get_iter(list_t *lst, list_iter_t *it)
{
    if (it->iter < lst->elnum) {
        return DLE_DATA(lst->index[it->iter]);
    }
    return NULL;
}

int
list_fini (list_t *lst)
{
    list_data_fini(lst);
    lst->init = NULL;
    lst->fini = NULL;
    lst->compar = NULL;
    return 0;
}

void *
list_first(list_t *lst, list_iter_t *iter)
{
    iter->iter = 0;
    return DLE_DATA(lst->data.head);
}

void *
list_last(list_t *lst, list_iter_t *iter)
{
    iter->iter = lst->elnum - 1;
    return DLE_DATA(lst->data.tail);
}

void *
list_next(list_t *lst, list_iter_t *iter)
{
    if (++iter->iter < lst->elnum) {
        return DLE_DATA(lst->index[iter->iter]);
    }
    return NULL;
}

void *
list_prev(list_t *lst, list_iter_t *iter)
{
    if (--iter->iter < lst->elnum) {
        return DLE_DATA(lst->index[iter->iter]);
    }
    return NULL;
}

void *
list_incr(list_t *lst)
{
    if (list_ensure_len(lst, lst->elnum + 1, LIST_FLAG_SAVE) != 0) {
        TRRETNULL(LIST_INCR + 1);
    }
    return list_get(lst, lst->elnum - 1);
}

void *
list_incr_iter(list_t *lst, list_iter_t *it)
{
    if (list_ensure_len(lst, lst->elnum + 1, LIST_FLAG_SAVE) != 0) {
        TRRETNULL(LIST_INCR + 1);
    }
    it->iter = lst->elnum - 1;
    return list_get(lst, lst->elnum - 1);
}

int
list_decr(list_t *lst)
{
    if (list_ensure_len(lst, lst->elnum - 1, LIST_FLAG_SAVE) != 0) {
        TRRET(LIST_DECR + 1);
    }
    return 0;
}

void *
list_find(list_t *lst, const void *key)
{
    if (lst->compar == NULL) {
        TRRETNULL(LIST_FIND + 1);
    }
    if (lst->elnum == 0) {
        TRRETNULL(LIST_FIND + 2);
    }
    return bsearch(key, lst->index, lst->elnum, sizeof(struct _dlist_entry), lst->compar);
}

int
list_traverse(list_t *lst, list_traverser_t tr, void *udata)
{
    unsigned i;
    int res;
    for (i = 0; i < lst->elnum; ++i) {
        if ((res = tr(DLE_DATA(lst->index[i]), udata)) != 0) {
            return res;
        }
    }
    return 0;
}

