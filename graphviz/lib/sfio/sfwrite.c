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

/*	Write data out to the file system
**
**	Written by Kiem-Phong Vo.
*/

/**
 * @param f write to this stream
 * @param buf buffer to be written
 * @param n number of bytes
 */
ssize_t sfwrite(Sfio_t * f, const void * buf, size_t n)
{
    uchar *s, *begs, *next;
    ssize_t w;
    int local;

    SFMTXSTART(f, (ssize_t) (-1));

    GETLOCAL(f, local);

    if (!buf)
	SFMTXRETURN(f, (ssize_t) (-1));

    /* release peek lock */
    if (f->mode & SF_PEEK) {
	if (!(f->mode & SF_WRITE) && (f->flags & SF_RDWR) != SF_RDWR)
	    SFMTXRETURN(f, (ssize_t) (-1));

	if (buf != f->next && (!f->rsrv || f->rsrv->data != buf))
	    SFMTXRETURN(f, (ssize_t) (-1));

	f->mode &= ~SF_PEEK;

	if (f->mode & SF_PKRD) {	/* read past peeked data */
	    char buffer[16];
	    ssize_t r;

	    for (w = n; w > 0;) {
		if ((r = w) > sizeof(buffer))
		    r = sizeof(buffer);
		if ((r = read(f->file, buffer, (size_t)r)) <= 0) {
		    n -= w;
		    break;
		} else
		    w -= r;
	    }

	    f->mode &= ~SF_PKRD;
	    f->endb = f->data + n;
	    f->here += n;
	}
    }

    s = begs = (uchar *) buf;
    for (;; f->mode &= ~SF_LOCK) {	/* check stream mode */
	if (SFMODE(f, local) != SF_WRITE
	    && _sfmode(f, SF_WRITE, local) < 0) {
	    w = s > begs ? s - begs : -1;
	    SFMTXRETURN(f, w);
	}

	SFLOCK(f, local);

	w = f->endb - f->next;

	if (s == f->next) {	/* after sfreserve */
	    if (w > (ssize_t) n)
		w = (ssize_t) n;
	    f->next = (s += w);
	    n -= w;
	    break;
	}

	/* attempt to create space in buffer */
	if (w == 0 || ((f->flags & SF_WHOLE) && w < (ssize_t) n)) {
	    if (f->flags & SF_STRING) {	/* extend buffer */
		(void) SFWR(f, s, n - w, f->disc);
		if ((w = f->endb - f->next) < (ssize_t) n)
		    break;
	    } else if (f->next > f->data) {
		(void) SFFLSBUF(f, -1);
		if ((w = f->endb - f->next) < (ssize_t) n &&
		    (f->flags & SF_WHOLE) && f->next > f->data)
		    break;
	    }
	}

	if (!(f->flags & SF_STRING) && f->next == f->data && SFDIRECT(f, n)) {	/* bypass buffering */
	    if ((w = SFWR(f, s, n, f->disc)) <= 0)
		break;
	} else {
	    if (w > (ssize_t) n)
		w = (ssize_t) n;
	    if (w <= 0)		/* no forward progress possible */
		break;
	    memcpy(f->next, s, w);
	    f->next += w;
	}

	s += w;
	if ((n -= w) <= 0)
	    break;
    }

    /* always flush buffer for share streams */
    if (f->extent < 0 && (f->flags & SF_SHARE) && !(f->flags & SF_PUBLIC))
	(void) SFFLSBUF(f, -1);

    /* check to see if buffer should be flushed */
    else if (n == 0 && (f->flags & SF_LINE) && !(f->flags & SF_STRING)) {
	if ((ssize_t) (n = f->next - f->data) > (w = s - begs))
	    n = w;
	if (n > 0 && n < HIFORLINE) {
	    for (next = f->next - 1; n > 0; --n, --next) {
		if (*next == '\n') {
		    n = HIFORLINE;
		    break;
		}
	    }
	}
	if (n >= HIFORLINE)
	    (void) SFFLSBUF(f, -1);
    }

    SFOPEN(f, local);

    w = s - begs;
    SFMTXRETURN(f, w);
}
