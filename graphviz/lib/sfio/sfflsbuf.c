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

/*	Write a buffer out to a file descriptor or
**	extending a buffer for a SF_STRING stream.
**
**	Written by Kiem-Phong Vo
*/

/**
 * @param f write out the buffered content of this stream
 * @param c if c>=0, c is also written out
 */
int _sfflsbuf(Sfio_t * f, int c)
{
    ssize_t n, w;
    uchar *data;
    uchar outc;
    int local, isall;
    int inpc = c;

    SFMTXSTART(f, -1);

    GETLOCAL(f, local);

    for (;; f->mode &= ~SF_LOCK) {	/* check stream mode */
	if (SFMODE(f, local) != SF_WRITE
	    && _sfmode(f, SF_WRITE, local) < 0)
	    SFMTXRETURN(f, -1);
	SFLOCK(f, local);

	/* current data extent */
	n = f->next - (data = f->data);

	if (n == (f->endb - data) && (f->flags & SF_STRING)) {	/* extend string stream buffer */
	    (void) SFWR(f, data, 1, f->disc);

	    /* !(f->flags&SF_STRING) is required because exception
	       handlers may turn a string stream to a file stream */
	    if (f->next < f->endb || !(f->flags & SF_STRING))
		n = f->next - (data = f->data);
	    else {
		SFOPEN(f, local);
		SFMTXRETURN(f, -1);
	    }
	}

	if (c >= 0) {		/* write into buffer */
	    if (n < (f->endb - (data = f->data))) {
		*f->next++ = c;
		if (c == '\n' &&
		    (f->flags & SF_LINE) && !(f->flags & SF_STRING)) {
		    c = -1;
		    n += 1;
		} else
		    break;
	    } else if (n == 0) {	/* unbuffered io */
		outc = (uchar) c;
		data = &outc;
		c = -1;
		n = 1;
	    }
	}

	if (n == 0 || (f->flags & SF_STRING))
	    break;

	isall = SFISALL(f, isall);
	if ((w = SFWR(f, data, n, f->disc)) > 0) {
	    if ((n -= w) > 0)	/* save unwritten data, then resume */
		memcpy(f->data, data + w, n);
	    f->next = f->data + n;
	    if (c < 0 && (!isall || n == 0))
		break;
	} else if (w == 0) {
	    SFOPEN(f, local);
	    SFMTXRETURN(f, -1);
	} else if (c < 0)
	    break;
    }

    SFOPEN(f, local);

    if (inpc < 0)
	inpc = f->endb - f->next;

    SFMTXRETURN(f, inpc);
}
