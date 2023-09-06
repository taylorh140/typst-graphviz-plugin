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
#include <stdint.h>

/*	Functions to set a given stream to some desired mode
**
**	Written by Kiem-Phong Vo.
**
**	Modifications:
**		06/27/1990 (first version)
**		01/06/1991
**		07/08/1991
**		06/18/1992
**		02/02/1993
**		05/25/1993
**		02/07/1994
**		05/21/1996
**		08/01/1997
**		08/01/1998 (extended formatting)
**		09/09/1999 (thread-safe)
**		02/01/2001 (adaptive buffering)
*/

/* done at exiting time */
static void _sfcleanup(void)
{
    Sfpool_t *p;
    Sfio_t *f;
    int n;
    int pool;

    /* set this so that no more buffering is allowed for write streams */
    _Sfexiting = 1001;

    sfsync(NULL);

    for (p = &_Sfpool; p; p = p->next) {
	for (n = 0; n < p->n_sf; ++n) {
	    if (!(f = p->sf[n]) || SFFROZEN(f))
		continue;

	    SFLOCK(f, 0);

	    /* let application know that we are leaving */
	    (void) SFRAISE(f, SF_ATEXIT, NULL);

	    if (f->flags & SF_STRING)
		continue;

	    /* from now on, write streams are unbuffered */
	    pool = f->mode & SF_POOL;
	    f->mode &= ~SF_POOL;
	    if ((f->flags & SF_WRITE) && !(f->mode & SF_WRITE))
		(void) _sfmode(f, SF_WRITE, 1);
	    if (((f->bits & SF_MMAP) && f->data) ||
		((f->mode & SF_WRITE) && f->next == f->data))
		(void) SFSETBUF(f, NULL, 0);
	    f->mode |= pool;

	    SFOPEN(f, 0);
	}
    }
}

/* put into discrete pool */
int _sfsetpool(Sfio_t * f)
{
    Sfpool_t *p;
    Sfio_t **array;
    int n, rv;

    if (!_Sfcleanup) {
	_Sfcleanup = _sfcleanup;
	(void) atexit(_sfcleanup);
    }

    if (!(p = f->pool))
	p = f->pool = &_Sfpool;

    rv = -1;

    if (p->n_sf >= p->s_sf) {
	if (p->s_sf == 0) {	/* initialize pool array */
	    p->s_sf = sizeof(p->array) / sizeof(p->array[0]);
	    p->sf = p->array;
	} else {		/* allocate a larger array */
	    n = (p->sf != p->array ? p->s_sf : (p->s_sf / 4 + 1) * 4) + 4;
	    if (!(array = malloc(n * sizeof(Sfio_t *))))
		goto done;

	    /* move old array to new one */
	    memcpy(array, p->sf, p->n_sf * sizeof(Sfio_t *));
	    if (p->sf != p->array)
		free(p->sf);

	    p->sf = array;
	    p->s_sf = n;
	}
    }

    /* always add at end of array because if this was done during some sort
       of walk through all streams, we'll want the new stream to be seen.
     */
    p->sf[p->n_sf++] = f;
    rv = 0;

  done:
    return rv;
}

/* create an auxiliary buffer for sfputr */
Sfrsrv_t *_sfrsrv(Sfio_t * f, ssize_t size)
{
    Sfrsrv_t *rsrv, *rs;

    /* make buffer if nothing yet */
    size = ((size + SF_GRAIN - 1) / SF_GRAIN) * SF_GRAIN;
    if (!(rsrv = f->rsrv) || size > rsrv->size) {
	if (!(rs = malloc(size + sizeof(Sfrsrv_t))))
	    size = -1;
	else {
	    if (rsrv) {
		if (rsrv->slen > 0)
		    memcpy(rs, rsrv, sizeof(Sfrsrv_t) + rsrv->slen);
		free(rsrv);
	    }
	    f->rsrv = rsrv = rs;
	    rsrv->size = size;
	    rsrv->slen = 0;
	}
    }

    if (rsrv && size > 0)
	rsrv->slen = 0;

    return size >= 0 ? rsrv : NULL;
}

/**
 * @param f change r/w mode and sync file pointer for this stream
 * @param wanted desired mode
 * @param local a local call
 */
int _sfmode(Sfio_t * f, int wanted, int local)
{
    Sfoff_t addr;
    int rv = 0;

    if ((!local && SFFROZEN(f))
	|| (!(f->flags & SF_STRING) && f->file < 0)) {
	if (local || !f->disc || !f->disc->exceptf) {
	    local = 1;
	    goto err_notify;
	}

	for (;;) {
	    if ((rv = f->disc->exceptf(f, SF_LOCKED, 0, f->disc)) < 0)
		return rv;
	    if ((!local && SFFROZEN(f)) ||
		(!(f->flags & SF_STRING) && f->file < 0)) {
		if (rv == 0) {
		    local = 1;
		    goto err_notify;
		} else
		    continue;
	    } else
		break;
	}
    }

    if (f->mode & SF_GETR) {
	f->mode &= ~SF_GETR;
	if (f->getr) {
	    f->next[-1] = f->getr;
	    f->getr = 0;
	}
    }

    if (f->mode & SF_STDIO)	/* synchronizing with stdio pointers */
	(*_Sfstdsync) (f);

    if (f->disc == _Sfudisc && wanted == SF_WRITE &&
	sfclose((*_Sfstack) (f, NULL)) < 0) {
	local = 1;
	goto err_notify;
    }

    if (f->mode & SF_POOL) {	/* move to head of pool */
	if (f == f->pool->sf[0] || _Sfpmove(f, 0) < 0) {
	    local = 1;
	    goto err_notify;
	}
	f->mode &= ~SF_POOL;
    }

    SFLOCK(f, local);

    /* buffer initialization */
    wanted &= SF_RDWR;
    if (f->mode & SF_INIT) {
	if (!f->pool && _sfsetpool(f) < 0) {
	    rv = -1;
	    goto done;
	}

	if (wanted == 0)
	    goto done;

	if (wanted != (int) (f->mode & SF_RDWR) && !(f->flags & wanted))
	    goto err_notify;

	if ((f->flags & SF_STRING) && f->size >= 0 && f->data) {
	    f->mode &= ~SF_INIT;
	    f->extent = ((f->flags & SF_READ) || (f->bits & SF_BOTH)) ?
		f->size : 0;
	    f->here = 0;
	    f->endb = f->data + f->size;
	    f->next = f->endr = f->endw = f->data;
	    if (f->mode & SF_READ)
		f->endr = f->endb;
	    else
		f->endw = f->endb;
	} else {
	    unsigned short n = f->flags;
	    (void) SFSETBUF(f, f->data, f->size);
	    f->flags |= (n & SF_MALLOC);
	}
    }

    if (wanted == (int) SFMODE(f, 1))
	goto done;

    switch (SFMODE(f, 1)) {
    case SF_WRITE:		/* switching to SF_READ */
	if (wanted == 0 || wanted == SF_WRITE)
	    break;
	if (!(f->flags & SF_READ))
	    goto err_notify;
	else if (f->flags & SF_STRING) {
	    SFSTRSIZE(f);
	    f->endb = f->data + f->extent;
	    f->mode = SF_READ;
	    break;
	}

	/* reset buffer */
	if (f->next > f->data && SFFLSBUF(f, -1) < 0)
	    goto err_notify;

	if (f->size == 0) {	/* unbuffered */
	    f->data = f->tiny;
	    f->size = sizeof(f->tiny);
	}
	f->next = f->endr = f->endw = f->endb = f->data;
	f->mode = SF_READ | SF_LOCK;

	break;

    case (SF_READ | SF_SYNCED):	/* a previously sync-ed read stream */
	if (wanted != SF_WRITE) {	/* just reset the pointers */
	    f->mode = SF_READ | SF_LOCK;

	    /* see if must go with new physical location */
	    if ((f->flags & (SF_SHARE | SF_PUBLIC)) ==
		(SF_SHARE | SF_PUBLIC)
		&& (addr = SFSK(f, 0, SEEK_CUR, f->disc)) != f->here) {
		f->endb = f->endr = f->endw = f->next = f->data;
		f->here = addr;
	    } else {
		addr = f->here + (f->endb - f->next);
		if (SFSK(f, addr, SEEK_SET, f->disc) < 0)
		    goto err_notify;
		f->here = addr;
	    }

	    break;
	}
	/* fall through */

    case SF_READ:		/* switching to SF_WRITE */
	if (wanted != SF_WRITE)
	    break;
	else if (!(f->flags & SF_WRITE))
	    goto err_notify;
	else if (f->flags & SF_STRING) {
	    f->endb = f->data + f->size;
	    f->mode = SF_WRITE | SF_LOCK;
	    break;
	}

	/* reset buffer and seek pointer */
	if (!(f->mode & SF_SYNCED)) {
	    intptr_t n = (intptr_t)(f->endb - f->next);
	    if (f->extent >= 0 && (n > 0 || (f->data && (f->bits & SF_MMAP)))) {	/* reset file pointer */
		addr = f->here - n;
		if (SFSK(f, addr, SEEK_SET, f->disc) < 0)
		    goto err_notify;
		f->here = addr;
	    }
	}

	f->mode = SF_WRITE | SF_LOCK;
	if (f->data == f->tiny) {
	    f->endb = f->data = f->next = NULL;
	    f->size = 0;
	} else
	    f->endb = (f->next = f->data) + f->size;

	break;

    default:			/* unknown case */
      err_notify:
	if ((wanted &= SF_RDWR) == 0
	    && (wanted = f->flags & SF_RDWR) == SF_RDWR)
	    wanted = SF_READ;

	/* set errno for operations that access wrong stream type */
	if (wanted != (f->mode & SF_RDWR) && f->file >= 0)
	    errno = EBADF;

	if (_Sfnotify)		/* notify application of the error */
	    _Sfnotify(f, wanted, f->file);

	rv = -1;
	break;
    }

  done:
    SFOPEN(f, local);
    return rv;
}
