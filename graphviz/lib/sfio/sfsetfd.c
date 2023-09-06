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

/*	Change the file descriptor
**
**	Written by Kiem-Phong Vo.
*/

static int _sfdup(int fd, int newfd)
{
    int dupfd;

#ifdef F_DUPFD			/* the simple case */
    while ((dupfd = fcntl(fd, F_DUPFD, newfd)) < 0 && errno == EINTR)
	errno = 0;
    return dupfd;

#else				/* do it the hard way */
    if ((dupfd = dup(fd)) < 0 || dupfd >= newfd)
	return dupfd;

    /* dup() succeeded but didn't get the right number, recurse */
    newfd = _sfdup(fd, newfd);

    /* close the one that didn't match */
    CLOSE(dupfd);

    return newfd;
#endif
}

int sfsetfd(Sfio_t * f, int newfd)
{
    int oldfd;

    SFMTXSTART(f, -1);

    if (f->flags & SF_STRING)
	SFMTXRETURN(f, -1);

    if ((f->mode & SF_INIT) && f->file < 0) {	/* restoring file descriptor after a previous freeze */
	if (newfd < 0)
	    SFMTXRETURN(f, -1);
    } else {			/* change file descriptor */
	if ((f->mode & SF_RDWR) != f->mode && _sfmode(f, 0, 0) < 0)
	    SFMTXRETURN(f, -1);
	SFLOCK(f, 0);

	oldfd = f->file;
	if (oldfd >= 0) {
	    if (newfd >= 0) {
		if ((newfd = _sfdup(oldfd, newfd)) < 0) {
		    SFOPEN(f, 0);
		    SFMTXRETURN(f, -1);
		}
		CLOSE(oldfd);
	    } else {		/* sync stream if necessary */
		if (((f->mode & SF_WRITE) && f->next > f->data) ||
		    (f->mode & SF_READ) || f->disc == _Sfudisc) {
		    if (SFSYNC(f) < 0) {
			SFOPEN(f, 0);
			SFMTXRETURN(f, -1);
		    }
		}

		if (((f->mode & SF_WRITE) && f->next > f->data) ||
		    ((f->mode & SF_READ) && f->extent < 0 &&
		     f->next < f->endb)) {
		    SFOPEN(f, 0);
		    SFMTXRETURN(f, -1);
		}

		/* make stream appears uninitialized */
		f->endb = f->endr = f->endw = f->data;
		f->extent = f->here = 0;
		f->mode = (f->mode & SF_RDWR) | SF_INIT;
		f->bits &= (unsigned short)~SF_NULL;	/* off /dev/null handling */
	    }
	}

	SFOPEN(f, 0);
    }

    /* notify changes */
    if (_Sfnotify)
	_Sfnotify(f, SF_SETFD, newfd);

    f->file = newfd;

    SFMTXRETURN(f, newfd);
}
