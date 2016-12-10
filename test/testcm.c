#include <assert.h>
#define _WITH_GETLINE
#include <stdio.h>
#include <string.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/fasthash.h>
#include <mrkcommon/bytes.h>
#include <mrkcommon/hash.h>
#include <mrkcommon/util.h>

#include <mrkcommon/cm.h>

#include "diag.h"
#include "unittest.h"

#ifndef NDEBUG
const char *_malloc_options = "AJ";
#endif

static uint64_t
myhash(cmhash_t *cmh)
{
    if (cmh->h == 0) {
        cmh->h = fasthash(0,
                          (unsigned char *)cmh->v,
                          cmh->w * sizeof(*cmh->v));
    }
    return cmh->h;
}

static int
mycmp(cmhash_t *a, cmhash_t *b)
{
    return memcmp(a->v, b->v, a->w * sizeof(*a->v));
}


static int
myfini(cmhash_t *key, UNUSED void *value)
{
    if (key != NULL) {
        if (key->o != NULL) {
            free(key->o);
            key->o = NULL;
        }
        cmhash_destroy(&key);
    }
    return 0;
}

static void
test0(void)
{
    mnhash_t d;
    struct {
        long rnd;
        char *s;
    } data[] = {
        {0, "This"},
        {0, "is"},
        {0, "the"},
        {0, "test"},
    };
    UNITTEST_PROLOG_RAND;


    hash_init(&d, 101,
              (hash_hashfn_t)myhash,
              (hash_item_comparator_t)mycmp,
              (hash_item_finalizer_t)myfini);

    FOREACHDATA {
        mnhash_item_t *dit;
        cmhash_t *cmh;

        cmh = cmhash_new(10, 20);
        cmhash_hash(cmh, strdup(CDATA.s), strlen(CDATA.s));

        if ((dit = hash_get_item(&d, cmh)) == NULL) {
            //TRACE("setting");
            //cmhash_dump(cmh);
            hash_set_item(&d, cmh, NULL);
        } else {
            TRACE("collision");
            cmhash_dump(cmh);
            cmhash_dump(dit->key);
            cmhash_destroy(&cmh);
        }
    }

    hash_fini(&d);
}


UNUSED static void
test1(const char *fname)
{
    mnhash_t d;
    FILE *f = NULL;
    char *s;
    size_t sz0;
    ssize_t sz1;
    size_t nlines;

    hash_init(&d, 1046527,
              (hash_hashfn_t)myhash,
              (hash_item_comparator_t)mycmp,
              (hash_item_finalizer_t)myfini);

    if ((f = fopen(fname, "r")) == NULL) {
        FAIL("open");
    }

    for (s = NULL, nlines = 0;
         (sz1 = getline(&s, &sz0, f)) > 0;
         s = NULL, sz0 = 0) {

        mnhash_item_t *dit;
        cmhash_t *cmh;

        if (sz1 < 64) {
            free(s);
            continue;
        }
        ++nlines;

        //cmh = cmhash_new(2, 1046527); /* collisions 0/645727 */

        //cmh = cmhash_new(2, 193939); /* collisions 5/645727 */
/* 
2/193939/0002d0f4000219a0/75e24957d6578043 137384 184564
2/193939/0002d0f4000219a0/90d64aba24944e50 137384 184564
sion
2/193939/00013ed100008d9d/694a929242739588 35989 81617
2/193939/00013ed100008d9d/6c9484379f5fd296 35989 81617
sion
2/193939/0000586d0002eee1/d53ddd00438999b1 192489 22637
2/193939/0000586d0002eee1/080a2207d2be93d7 192489 22637
sion
2/193939/0002e04900025132/3803e81431d9c297 151610 188489
2/193939/0002e04900025132/5d703f3fedd73808 151610 188489
sion
2/193939/0001f8280000be73/d59ccfa33da49aea 49019 129064
2/193939/0001f8280000be73/b43947e1d5b704b9 49019 129064
s=645727
 */
        //cmh = cmhash_new(3, 93563); /* collisions 0/645727 */

        //cmh = cmhash_new(4, 65537); /* collisions 4/645727 */
/* 

4/65537/00131625001c0080/3385c060b70406d5 56493 39485 46813 63480
4/65537/00131625001c0080/3385c064f80847d5 56493 39485 46813 63480

4/65537/0003db5b000bcbe0/3385c064f810c9d5 24228 7220 14548 31215
4/65537/0003db5b000bcbe0/3385c05c760847d5 24228 7220 14548 31215

4/65537/0002e6f9000bb386/3385c06939150ad5 24228 5206 16562 31215
4/65537/0002e6f9000bb386/3385c06d7a194bd5 24228 5206 16562 31215

4/65537/000f46a3001be729/3a2bb8dddf5d30f9 53327 30787 63766 50688
4/65537/000f46a3001be729/3a2bb8fedf7e30f9 53327 30787 63766 50688
 */
        //cmh = cmhash_new(3, 32771); /* collisions 0/645727 */

        //cmh = cmhash_new(3, 16411); /* collisions 0/645727 */

        //cmh = cmhash_new(4, 8209); /* collisions 1/645727 */
/* 
4/8209/0001e7f800019712/fbc8975b390d877e 2503 3948 4517 6932
4/8209/0001e7f800019712/c9c8a0f818d3c176 2503 3948 4517 6932
 */

        //cmh = cmhash_new(3, 8209); /* collisions 2/645727 */
/* 
3/8209/0001fceca51011ab/fbc8975b390d877e 2503 3948 4517
3/8209/0001fceca51011ab/c9c8a0f818d3c176 2503 3948 4517

3/8209/000070ec9d09a64e/ba3e7142399643af 5378 876 2973
3/8209/000070ec9d09a64e/376593325cef194b 5378 876 2973
 */

        //cmh = cmhash_new(4, 4099); /* collisions 2/645727 */
/* 
4/4099/0000861d000198f1/64394aaaac9417ea 3689 1112 872 2373
4/4099/0000861d000198f1/f29b31656e9417ea 3689 1112 872 2373

4/4099/0000f60f0000f59a/f28aceb4297072a4 1471 1964 2069 2851
4/4099/0000f60f0000f59a/6428e7f9677072a4 1471 1964 2069 2851
 */

        //cmh = cmhash_new(5, 2053); /* collisions 1/645727 */
/* 
5/2053/0020d090c301413c/1e833a499e1127bf 707 1936 651 1792 195
5/2053/0020d090c301413c/62155b499e1127bf 707 1936 651 1792 195
 */

        //cmh = cmhash_new(4, 1031); /* collisions 1/645727 */
/* 
4/1031/000014b7000073ab/8c6a235a9f07d57b 358 165 413 498
4/1031/000014b7000073ab/006a321550bfd377 358 165 413 498
 */

        cmh = cmhash_new(20, 521); /* collisions 7/645727 */
/* 
20/521/bf19eca080712683/4fa0e0309e1127bf 421 436 274 174 489 452 274 217 119 381 114 400 148 515 447 323 111 392 350 36
20/521/bf19eca080712683/51937d309e1127bf 421 436 274 174 489 452 274 217 119 381 114 400 148 515 447 323 111 392 350 36
sion
20/521/dec6e819fa86aed7/4a5d56e66df1d387 498 196 90 460 31 377 86 71 165 23 192 505 222 431 490 233 296 316 415 432
20/521/dec6e819fa86aed7/b22856e66df1d387 498 196 90 460 31 377 86 71 165 23 192 505 222 431 490 233 296 316 415 432
sion
20/521/c9583954220aa647/b18d41065c526683 59 394 226 190 401 92 459 66 497 190 144 93 489 44 364 231 204 400 508 131
20/521/c9583954220aa647/c9cca7cc5c526683 59 394 226 190 401 92 459 66 497 190 144 93 489 44 364 231 204 400 508 131
sion
20/521/a45502d70397beeb/7627ad788638c7dc 491 218 449 102 428 202 323 110 363 251 271 288 96 178 204 459 381 479 491 222
20/521/a45502d70397beeb/7e3ccfba8638c7dc 491 218 449 102 428 202 323 110 363 251 271 288 96 178 204 459 381 479 491 222
sion
20/521/5181eda74b7c64dc/c7e3fbb1468caad8 58 397 14 105 141 406 328 25 54 28 309 367 229 247 4 82 140 160 467 270
20/521/5181eda74b7c64dc/b3af260c468caad8 58 397 14 105 141 406 328 25 54 28 309 367 229 247 4 82 140 160 467 270
sion
20/521/af470a75f69d1350/6a4e283d979e9426 246 66 420 404 76 175 339 324 40 500 241 352 297 199 143 14 3 400 241 189
20/521/af470a75f69d1350/6643971c979e9426 246 66 420 404 76 175 339 324 40 500 241 352 297 199 143 14 3 400 241 189
sion
20/521/466f9f80d0598308/08c12b2cfa6b4bed 456 368 405 176 493 135 314 365 42 42 174 438 299 23 295 431 206 382 468 37
20/521/466f9f80d0598308/0ccbbc4dfa6b4bed 456 368 405 176 493 135 314 365 42 42 174 438 299 23 295 431 206 382 468 37
 */


        cmhash_hash(cmh, s, sz1);

        if ((dit = hash_get_item(&d, cmh)) == NULL) {
            //TRACE("setting");
            //cmhash_dump(cmh);
            hash_set_item(&d, cmh, NULL);
        } else {
            TRACE("collision");
            cmhash_dump(cmh);
            cmhash_dump(dit->key);
            cmhash_destroy(&cmh);
        }

    }

    fclose(f);
    hash_fini(&d);
    TRACE("nlines=%ld", nlines);
}



static uint64_t
pset_item_hash(pset_item_t *it)
{
    mnbytes_t *s;
    s = it->v;
    return bytes_hash(s);
}


static int
pset_item_cmp(pset_item_t *a, pset_item_t *b)
{
    mnbytes_t *sa, *sb;
    sa = a->v;
    sb = b->v;
    return bytes_cmp(sa, sb);
}


static int
pset_item_fini(pset_item_t *it)
{
    if (it != NULL) {
        mnbytes_t *s;
        s = it->v;
        //TRACE("decrefing %s", s->data);
        bytes_decref(&s);
        free(it);
    }
    return 0;
}


static pset_item_t *
pset_item_new(char *s)
{
    pset_item_t *it;

    if ((it = malloc(sizeof(pset_item_t))) == NULL) {
        FAIL("malloc");
    }
    it->v = bytes_new_from_str(s);

    return it;
}


int
mycb(pset_item_t *it, UNUSED void *v, void *udata)
{
    CMTY *aggr;
    mnbytes_t *s;

    s = it->v;
    aggr = udata;
    *aggr += it->cmprop;
    TRACE("%03ld %s", (long)it->cmprop, s->data);
    return 0;
}


UNUSED static void
test2(char *fname)
{
    FILE *f = NULL;
    char *s;
    size_t sz0;
    ssize_t sz1;
    size_t nlines;
    pset_t pset;
    uint64_t aggr;

    if ((f = fopen(fname, "r")) == NULL) {
        FAIL("open");
    }

    pset_init(&pset,
                80,
                (hash_hashfn_t)pset_item_hash,
                (hash_item_comparator_t)pset_item_cmp,
                (hash_item_finalizer_t)pset_item_fini,
                1000);

    for (s = NULL, nlines = 0;
         (sz1 = getline(&s, &sz0, f)) > 0;
         s = NULL, sz0 = 0) {

        pset_item_t *it0, *it1;

        if (sz1 < 0) {
            free(s);
            continue;
        }
        ++nlines;
        s[sz1 - 1] = '\0';
        //D8(s, sz1);
        it0 = pset_item_new(s);
        free(s);

        if ((it1 = pset_peek(&pset, it0)) == NULL) {
            if ((it1 = pset_push(&pset, it0)) != NULL) {
                UNUSED mnbytes_t *s;

                s = it1->v;
                //TRACE("deleting %s", s->data);
                pset_item_fini(it1);
            }
        } else {
            UNUSED mnbytes_t *s;

            s = it0->v;
            //TRACE("deleting dup %s", s->data);
            pset_item_fini(it0);
        }
    }

    aggr = 0;
    pset_traverse(&pset, (hash_traverser_t)mycb, &aggr);

    pset_fini(&pset);
    fclose(f);
    TRACE("nlines=%ld aggr=%ld", nlines, (long)aggr);
}


static void
pset_item_tell_cm(pset_item_t *it, cm_t *cm)
{
    mnbytes_t *s;

    s = it->v;
    cm_add(cm, s->data, s->sz, 1);
    it->cmprop = 0x7fffffff;
    cm_get(cm, s->data, s->sz, &it->cmprop);
}


UNUSED static void
pset_item_ask_cm(pset_item_t *it, cm_t *cm)
{
    mnbytes_t *s;

    s = it->v;
    it->cmprop = 0x7fffffff;
    cm_get(cm, s->data, s->sz, &it->cmprop);
}


UNUSED static void
test3(char *fname)
{
    FILE *f = NULL;
    char *s;
    size_t sz0;
    ssize_t sz1;
    size_t nlines;
    pset_t pset;
    cm_t cm;
    CMTY aggr;

    if ((f = fopen(fname, "r")) == NULL) {
        FAIL("open");
    }

    cm_init(&cm, 4, 1031);
    pset_init(&pset,
                10,
                (hash_hashfn_t)pset_item_hash,
                (hash_item_comparator_t)pset_item_cmp,
                (hash_item_finalizer_t)pset_item_fini,
                0);

    for (s = NULL, nlines = 0;
         (sz1 = getline(&s, &sz0, f)) > 0;
         s = NULL, sz0 = 0) {
        pset_item_t *it0, *it1;

        if (sz1 < 64) {
            free(s);
            continue;
        }
        ++nlines;
        s[sz1 - 1] = '\0';
        //D8(s, sz1);
        it0 = pset_item_new(s);
        free(s);

        pset_item_tell_cm(it0, &cm);

        if ((it1 = pset_peek(&pset, it0)) == NULL) {
            if ((it1 = pset_push(&pset, it0)) != NULL) {
                UNUSED mnbytes_t *s;

                //s = it1->v;
                //TRACE("deleting %s", s->data);
                pset_item_fini(it1);
            }
        } else {
            UNUSED mnbytes_t *s;

            //s = it0->v;
            //TRACE("deleting dup %s", s->data);
            pset_item_ask_cm(it1, &cm);
            pset_item_fini(it0);
        }
    }

    aggr = 0;
    pset_traverse(&pset, (hash_traverser_t)mycb, &aggr);

    pset_fini(&pset);
    cm_fini(&cm);
    fclose(f);
    TRACE("nlines=%ld aggr=%ld", nlines, (long)aggr);
}


int
main(int argc, char **argv)
{
    int i;

    test0();
    for (i = 1; i < argc; ++i) {
        //test1(argv[i]);
        //test2(argv[i]);
        test3(argv[i]);
    }
    return 0;
}
