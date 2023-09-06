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

/*	Function to handle io exceptions.
**	Written by Kiem-Phong Vo
*/

/**
 * @param f stream where the exception happened
 * @param type type that was performed
 * @param io return value that indicated exception
 * @param disc discipline in use
 */
int _sfexcept(Sfio_t * f, int type, ssize_t io, Sfdisc_t * disc)
{
    int ev, local, lock;
    ssize_t size;
    uchar *data;

    SFMTXSTART(f, -1);

    GETLOCAL(f, local);
    lock = f->mode & SF_LOCK;

    if (local && io <= 0)
	f->flags |= io < 0 ? 0 : SF_EOF;

    if (disc && disc->exceptf) {	/* let the stream be generally accessible for this duration */
	if (local && lock)
	    SFOPEN(f, 0);

	/* so that exception handler knows what we are asking for */
	_Sfi = f->val = io;
	ev = disc->exceptf(f, type, &io, disc);

	/* relock if necessary */
	if (local && lock)
	    SFLOCK(f, 0);

	if (io > 0 && !(f->flags & SF_STRING))
	    SFMTXRETURN(f, ev);
	if (ev < 0)
	    SFMTXRETURN(f, SF_EDONE);
	if (ev > 0)
	    SFMTXRETURN(f, SF_EDISC);
    }

    if (f->flags & SF_STRING) {
	if (type == SF_READ)
	    goto chk_stack;
	else if (type != SF_WRITE && type != SF_SEEK)
	    SFMTXRETURN(f, SF_EDONE);
	if (local && io >= 0) {
	    if (f->size >= 0 && !(f->flags & SF_MALLOC))
		goto chk_stack;
	    /* extend buffer */
	    if ((size = f->size) < 0)
		size = 0;
	    if ((io -= size) <= 0)
		io = SF_GRAIN;
	    size = ((size + io + SF_GRAIN - 1) / SF_GRAIN) * SF_GRAIN;
	    if (f->size > 0)
		data = realloc(f->data, size);
	    else
		data = malloc(size);
	    if (!data)
		goto chk_stack;
	    f->endb = data + size;
	    f->next = data + (f->next - f->data);
	    f->endr = f->endw = f->data = data;
	    f->size = size;
	}
	SFMTXRETURN(f, SF_EDISC);
    }

    if (errno == EINTR) {
	if (_Sfexiting || (f->bits & SF_ENDING))	/* stop being a hero */
	    SFMTXRETURN(f, SF_EDONE);

	/* a normal interrupt, we can continue */
	errno = 0;
	f->flags &= (unsigned short)~SF_EOF;
	SFMTXRETURN(f, SF_ECONT);
    }

  chk_stack:
    if (local && f->push && ((type == SF_READ && f->next >= f->endb) || (type == SF_WRITE && f->next <= f->data))) {	/* pop the stack */
	Sfio_t *pf;

	if (lock)
	    SFOPEN(f, 0);

	/* pop and close */
	pf = _Sfstack(f, NULL);
	if ((ev = sfclose(pf)) < 0)	/* can't close, restack */
	    _Sfstack(f, pf);

	if (lock)
	    SFLOCK(f, 0);

	ev = ev < 0 ? SF_EDONE : SF_ESTACK;
    } else
	ev = SF_EDONE;

    SFMTXRETURN(f, ev);
}
