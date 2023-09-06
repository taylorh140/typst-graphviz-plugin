/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include	<sfio/sfhdr.h>
#include	<stddef.h>

/*	Swap two streams. If the second argument is NULL,
**	a new stream will be created. Always return the second argument
**	or the new stream. Note that this function will always work
**	unless streams are locked by SF_PUSH.
**
**	Written by Kiem-Phong Vo.
*/

Sfio_t *sfswap(Sfio_t * f1, Sfio_t * f2)
{
    Sfio_t tmp;
    int f1pool, f2pool, f1flags, f2flags;
    unsigned f2mode;

    if (!f1 || (f1->mode & SF_AVAIL)
	|| (SFFROZEN(f1) && (f1->mode & SF_PUSH)))
	return NULL;
    if (f2 && SFFROZEN(f2) && (f2->mode & SF_PUSH))
	return NULL;
    if (f1 == f2)
	return f2;

    unsigned f1mode = f1->mode;
    SFLOCK(f1, 0);
    f1->mode |= SF_PUSH;	/* make sure there is no recursion on f1 */

    if (f2) {
	f2mode = f2->mode;
	SFLOCK(f2, 0);
	f2->mode |= SF_PUSH;	/* make sure there is no recursion on f2 */
    } else {
	f2 = f1->file == 0 ? sfstdin :
	    f1->file == 1 ? sfstdout :
	    f1->file == 2 ? sfstderr : NULL;
	if ((!f2 || !(f2->mode & SF_AVAIL))) {
	    if (!(f2 = malloc(sizeof(Sfio_t)))) {
		f1->mode = f1mode;
		SFOPEN(f1, 0);
		return NULL;
	    }

	    SFCLEAR(f2);
	}
	f2->mode = SF_AVAIL | SF_LOCK;
	f2mode = SF_AVAIL;
    }

    if (!f1->pool)
	f1pool = -1;
    else
	for (f1pool = f1->pool->n_sf - 1; f1pool >= 0; --f1pool)
	    if (f1->pool->sf[f1pool] == f1)
		break;
    if (!f2->pool)
	f2pool = -1;
    else
	for (f2pool = f2->pool->n_sf - 1; f2pool >= 0; --f2pool)
	    if (f2->pool->sf[f2pool] == f2)
		break;

    f1flags = f1->flags;
    f2flags = f2->flags;

    /* swap image and pool entries */
    tmp = *f1;
    *f1 = *f2;
    *f2 = tmp;
    if (f2pool >= 0)
	f1->pool->sf[f2pool] = f1;
    if (f1pool >= 0)
	f2->pool->sf[f1pool] = f2;

    if (f2flags & SF_STATIC)
	f2->flags |= SF_STATIC;
    else
	f2->flags &= (unsigned short)~SF_STATIC;

    if (f1flags & SF_STATIC)
	f1->flags |= SF_STATIC;
    else
	f1->flags &= (unsigned short)~SF_STATIC;

    if (f2mode & SF_AVAIL) {	/* swapping to a closed stream */
	if (!(f1->flags & SF_STATIC))
	    free(f1);
    } else {
	f1->mode = f2mode;
	SFOPEN(f1, 0);
    }

    f2->mode = f1mode;
    SFOPEN(f2, 0);
    return f2;
}
