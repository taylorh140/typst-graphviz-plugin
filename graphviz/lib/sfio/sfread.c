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

/*	Read n bytes from a stream into a buffer
**
**	Written by Kiem-Phong Vo.
*/

/**
 * @param f read from this stream
 * @param buf buffer to read into
 * @param n number of bytes to be read
 */
ssize_t sfread(Sfio_t * f, void * buf, size_t n)
{
    uchar *s, *begs;
    ssize_t r;
    int local, justseek;

    SFMTXSTART(f, (ssize_t) (-1));

    GETLOCAL(f, local);
    justseek = f->bits & SF_JUSTSEEK;
    f->bits &= (unsigned short)~SF_JUSTSEEK;

    if (!buf)
	SFMTXRETURN(f, (ssize_t) (-1));

    /* release peek lock */
    if (f->mode & SF_PEEK) {
	if (!(f->mode & SF_READ))
	    SFMTXRETURN(f, (ssize_t) (-1));

	if (f->mode & SF_GETR) {
	    if (((uchar *) buf + f->val) != f->next &&
		(!f->rsrv || f->rsrv->data != (uchar *) buf))
		SFMTXRETURN(f, (ssize_t) (-1));
	    f->mode &= ~SF_PEEK;
	    SFMTXRETURN(f, 0);
	} else {
	    if (buf != f->next)
		SFMTXRETURN(f, (ssize_t) (-1));
	    f->mode &= ~SF_PEEK;
	    if (f->mode & SF_PKRD) {	/* actually read the data now */
		f->mode &= ~SF_PKRD;
		if (n > 0)
		    n = (r = read(f->file, f->data, n)) < 0 ? 0 : r;
		f->endb = f->data + n;
		f->here += n;
	    }
	    f->next += n;
	    f->endr = f->endb;
	    SFMTXRETURN(f, n);
	}
    }

    s = begs = (uchar *) buf;
    for (;; f->mode &= ~SF_LOCK) {	/* check stream mode */
	if (SFMODE(f, local) != SF_READ && _sfmode(f, SF_READ, local) < 0) {
	    n = s > begs ? s - begs : (size_t) (-1);
	    SFMTXRETURN(f, (ssize_t) n);
	}

	SFLOCK(f, local);

	if ((r = f->endb - f->next) > 0) {	/* has buffered data */
	    if (r > (ssize_t) n)
		r = (ssize_t) n;
	    if (s != f->next)
		memcpy(s, f->next, r);
	    f->next += r;
	    s += r;
	    n -= r;
	}

	if (n <= 0)		/* all done */
	    break;

	if (!(f->flags & SF_STRING) && !(f->bits & SF_MMAP)) {
	    f->next = f->endb = f->data;

	    /* exact IO is desirable for these cases */
	    if (SFDIRECT(f, n) || ((f->flags & SF_SHARE) && f->extent < 0))
		r = (ssize_t) n;
	    else if (justseek && n <= f->iosz && f->iosz <= f->size)
		r = f->iosz;	/* limit buffering */
	    else
		r = f->size;	/* full buffering */

	    /* if read almost full size, then just do it direct */
	    if (r > (ssize_t) n && (r - r / 8) <= (ssize_t) n)
		r = (ssize_t) n;

	    /* read directly to user's buffer */
	    if (r == (ssize_t) n && (r = SFRD(f, s, r, f->disc)) >= 0) {
		s += r;
		n -= r;
		if (r == 0 || n == 0)	/* eof or eob */
		    break;
	    } else
		goto do_filbuf;
	} else {
	  do_filbuf:
	    if (justseek)
		f->bits |= SF_JUSTSEEK;
	    if (SFFILBUF(f) <= 0)
		break;
	}
    }

    SFOPEN(f, local);
    r = s - begs;
    SFMTXRETURN(f, r);
}
