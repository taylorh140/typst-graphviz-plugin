/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <stddef.h>
#include	<sfio/sfhdr.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#include <sysinfoapi.h>
#endif

static ssize_t get_page_size(void) {
#ifdef _WIN32
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return (ssize_t)info.dwPageSize;
#else
  return (ssize_t)sysconf(_SC_PAGESIZE);
#endif
}

/*	Set a (new) buffer for a stream.
**	If size < 0, it is assigned a suitable value depending on the
**	kind of stream. The actual buffer size allocated is dependent
**	on how much memory is available.
**
**	Written by Kiem-Phong Vo.
*/
#ifndef HAVE_SYS_STAT_H
    struct stat {
    int st_mode;
    int st_size;
};
#define fstat(fd,st)	(-1)
#endif /*HAVE_SYS_STAT_H*/

/**
 * @param f stream to be buffered
 * @param buf new buffer
 * @param size buffer size, -1 for default size
 */
void *sfsetbuf(Sfio_t * f, void * buf, size_t size)
{
    int sf_malloc;
    uchar *obuf;
    Sfdisc_t *disc;
    ssize_t osize, blksize;
    int oflags, init, local;
    Stat_t st;

    SFMTXSTART(f, NULL);

    GETLOCAL(f, local);

    if (size == 0 && buf) {	/* special case to get buffer info */
	_Sfi = f->val =
	    (f->bits & SF_MMAP) ? (f->endb - f->data) : f->size;
	SFMTXRETURN(f, f->data);
    }

    /* cleanup actions already done, don't allow write buffering any more */
    if (_Sfexiting && !(f->flags & SF_STRING) && (f->mode & SF_WRITE)) {
	buf = NULL;
	size = 0;
    }

    if ((init = f->mode & SF_INIT)) {
	if (!f->pool && _sfsetpool(f) < 0)
	    SFMTXRETURN(f, NULL);
    } else if ((f->mode & SF_RDWR) != SFMODE(f, local)
	       && _sfmode(f, 0, local) < 0)
	SFMTXRETURN(f, NULL);

    if (init)
	f->mode = (f->mode & SF_RDWR) | SF_LOCK;
    else {
	int rv;

	/* synchronize first */
	SFLOCK(f, local);
	rv = SFSYNC(f);
	SFOPEN(f, local);
	if (rv < 0)
	    SFMTXRETURN(f, NULL);

	/* turn off the SF_SYNCED bit because buffer is changing */
	f->mode &= ~SF_SYNCED;
    }

    SFLOCK(f, local);

    blksize = 0;
    oflags = f->flags;

    if (f->data == f->tiny) {
	f->data = NULL;
	f->size = 0;
    }
    obuf = f->data;
    osize = f->size;

    f->flags &= (unsigned short)~SF_MALLOC;
    f->bits &= (unsigned short)~SF_MMAP;

    /* pure read/string streams must have a valid string */
    if ((f->flags & (SF_RDWR | SF_STRING)) == SF_RDSTR &&
	(size == SF_UNBOUND || !buf))
	size = 0;

    /* set disc to the first discipline with a seekf */
    for (disc = f->disc; disc; disc = disc->disc)
	if (disc->seekf)
	    break;

    if ((init || local) && !(f->flags & SF_STRING)) {	/* ASSERT(f->file >= 0) */
	st.st_mode = 0;

	/* if has discipline, set size by discipline if possible */
#ifdef HAVE_SYS_STAT_H
  enum { have_sys_stat_h = 1 };
#else
  enum { have_sys_stat_h = 0 };
#endif
	if (!have_sys_stat_h || disc) {
	    if ((f->here = SFSK(f, 0, SEEK_CUR, disc)) < 0)
		goto unseekable;
	    else {
		Sfoff_t e;
		if ((e = SFSK(f, 0, SEEK_END, disc)) >= 0)
		    f->extent = e > f->here ? e : f->here;
		(void) SFSK(f, f->here, SEEK_SET, disc);
		goto setbuf;
	    }
	}

	/* get file descriptor status */
	if (fstat((int) f->file, &st) < 0)
	    f->here = -1;
	else {
	    if (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))
		f->here = SFSK(f, 0, SEEK_CUR, f->disc);
	    else
		f->here = -1;
	}

	if (f->here >= 0) {
	    f->extent = (Sfoff_t) st.st_size;

	    /* seekable std-devices are share-public by default */
	    if (f == sfstdin || f == sfstdout || f == sfstderr)
		f->flags |= SF_SHARE | SF_PUBLIC;
	} else {
	  unseekable:
	    f->extent = -1;
	    f->here = 0;

	    if (init) {
		if (S_ISCHR(st.st_mode)) {
		    int oerrno = errno;

		    blksize = SF_GRAIN;

		    /* set line mode for terminals */
		    if (!(f->flags & SF_LINE) && isatty(f->file))
			f->flags |= SF_LINE;
#ifdef HAVE_SYS_STAT_H
		    else {	/* special case /dev/null */
			int dev, ino;
			dev = (int) st.st_dev;
			ino = (int) st.st_ino;
			if (stat(DEVNULL, &st) >= 0 &&
			    dev == (int) st.st_dev &&
			    ino == (int) st.st_ino)
			    SFSETNULL(f);
		    }
#endif
		    errno = oerrno;
		}
	    }
	}

	/* set page size, this is also the desired default buffer size */
	if (_Sfpage <= 0) {
		_Sfpage = get_page_size();
	}
    }

    /* get buffer space */
  setbuf:
    if (size == SF_UNBOUND) {	/* define a default size suitable for block transfer */
	if (init && osize > 0)
	    size = osize;
	else if (f == sfstderr && (f->mode & SF_WRITE))
	    size = 0;
	else if (f->flags & SF_STRING)
	    size = SF_GRAIN;
	else if ((f->flags & SF_READ) && !(f->bits & SF_BOTH) &&
		 f->extent > 0 && f->extent < (Sfoff_t) _Sfpage)
	    size =
		(((size_t) f->extent + SF_GRAIN -
		  1) / SF_GRAIN) * SF_GRAIN;
	else if ((ssize_t) (size = _Sfpage) < blksize)
	    size = blksize;

	buf = NULL;
    }

    sf_malloc = 0;
    if (size > 0 && !buf && !(f->bits & SF_MMAP)) {	/* try to allocate a buffer */
	if (obuf && size == (size_t) osize && init) {
	    buf = obuf;
	    obuf = NULL;
	    sf_malloc = (oflags & SF_MALLOC);
	}
	if (!buf) {		/* do allocation */
	    while (!buf && size > 0) {
		if ((buf = malloc(size)))
		    break;
		else
		    size /= 2;
	    }
	    if (size > 0)
		sf_malloc = SF_MALLOC;
	}
    }

    if (size == 0 && !(f->flags & SF_STRING) && !(f->bits & SF_MMAP) && (f->mode & SF_READ)) {	/* use the internal buffer */
	size = sizeof(f->tiny);
	buf = f->tiny;
    }

    /* set up new buffer */
    f->size = size;
    f->next = f->data = f->endr = f->endw = (uchar *) buf;
    f->endb = (f->mode & SF_READ) ? f->data : f->data + size;
    if (f->flags & SF_STRING) {	/* these fields are used to test actual size - see sfseek() */
	f->extent = (!sf_malloc &&
		     ((f->flags & SF_READ)
		      || (f->bits & SF_BOTH))) ? size : 0;
	f->here = 0;

	/* read+string stream should have all data available */
	if ((f->mode & SF_READ) && !sf_malloc)
	    f->endb = f->data + size;
    }

    f->flags = (unsigned short)((f->flags & ~SF_MALLOC) | sf_malloc);

    if (obuf && obuf != f->data && osize > 0 && (oflags & SF_MALLOC)) {
	free(obuf);
	obuf = NULL;
    }

    _Sfi = f->val = obuf ? osize : 0;

    SFOPEN(f, local);

    SFMTXRETURN(f, obuf);
}
