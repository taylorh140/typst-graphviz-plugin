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
#ifdef HAVE_SYS_IOCTL_H
#include	<sys/ioctl.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif

/*	Read/Peek a record from an unseekable device
**
**	Written by Kiem-Phong Vo.
*/

#define STREAM_PEEK	001
#define SOCKET_PEEK	002

/**
 * @param fd file descriptor
 * @param argbuf buffer to read data
 * @param n buffer size
 * @param rc record character
 * @param action >0: peeking, if rc>=0, get action records,
 *               <0: no peeking, if rc>=0, get -action records,
 *               =0: no peeking, if rc>=0, must get a single record
 */
ssize_t sfpkrd(int fd, void * argbuf, size_t n, int rc, int action) {
    ssize_t r;
    int ntry, t;
    char *buf = argbuf, *endbuf;

    if (rc < 0 && action <= 0)
	return read(fd, buf, n);

    t = (action > 0 || rc >= 0) ? (STREAM_PEEK | SOCKET_PEEK) : 0;
#ifndef I_PEEK
    t &= ~STREAM_PEEK;
#endif
#ifndef MSG_PEEK
    t &= ~SOCKET_PEEK;
#endif

    for (ntry = 0; ntry < 2; ++ntry) {
	r = -1;
#ifdef I_PEEK
	if (t & STREAM_PEEK) {
	    struct strpeek pbuf;
	    pbuf.flags = 0;
	    pbuf.ctlbuf.maxlen = -1;
	    pbuf.ctlbuf.len = 0;
	    pbuf.ctlbuf.buf = NULL;
	    pbuf.databuf.maxlen = n;
	    pbuf.databuf.buf = buf;
	    pbuf.databuf.len = 0;

	    if ((r = ioctl(fd, I_PEEK, &pbuf)) < 0) {
		if (errno == EINTR)
		    return -1;
		t &= ~STREAM_PEEK;
	    } else {
		t &= ~SOCKET_PEEK;
		if (r > 0 && (r = pbuf.databuf.len) <= 0) {
		    if (action <= 0)	/* read past eof */
			r = read(fd, buf, 1);
		    return r;
		}
		if (r == 0)
		    r = -1;
		else if (r > 0)
		    break;
	    }
	}
#endif

	if (ntry == 1)
	    break;

	/* poll or select to see if data is present.  */
	while (action > 0 ||
	       /* block until there is data before peeking again */
	       ((t & STREAM_PEEK) && rc >= 0) ||
	       /* let select be interrupted instead of recv which autoresumes */
	       (t & SOCKET_PEEK)) {
	    r = -2;
#ifdef HAVE_SELECT
	    if (r == -2) {
		fd_set rd;
		FD_ZERO(&rd);
		FD_SET(fd, &rd);
		r = select(fd + 1, &rd, NULL, NULL, NULL);
		if (r < 0) {
		    if (errno == EINTR)
			return -1;
		    else if (errno == EAGAIN) {
			errno = 0;
			continue;
		    } else
			r = -2;
		} else
		    r = FD_ISSET(fd, &rd) ? 1 : -1;
	    }
#endif /*HAVE_SELECT*/

	    if (r > 0) {	/* there is data now */
		if (action <= 0 && rc < 0)
		    return read(fd, buf, n);
		else
		    r = -1;
	    } else
		r = -1;
	    break;
	}

#ifdef MSG_PEEK
	if (t & SOCKET_PEEK) {
	    while ((r = recv(fd, buf, n, MSG_PEEK)) < 0) {
		if (errno == EINTR)
		    return -1;
		else if (errno == EAGAIN) {
		    errno = 0;
		    continue;
		}
		t &= ~SOCKET_PEEK;
		break;
	    }
	    if (r >= 0) {
		t &= ~STREAM_PEEK;
		if (r > 0)
		    break;
		else {		/* read past eof */
		    if (action <= 0)
			r = read(fd, buf, 1);
		    return r;
		}
	    }
	}
#endif
    }

    if (r < 0) {
	if (action > 0)
	    return -1;
	else { // get here means: action <= 0 && rc >= 0
	    /* number of records read at a time */
	    if ((action = action ? -action : 1) > (int) n)
		action = n;
	    r = 0;
	    ssize_t r_chunk;
	    while ((r_chunk = read(fd, buf, action)) > 0) {
		r += r_chunk;
		for (endbuf = buf + t; buf < endbuf;)
		    if (*buf++ == rc)
			action -= 1;
		if (action == 0 || (int) (n - r) < action)
		    break;
	    }
	    return r == 0 ? r_chunk : r;
	}
    }

    /* successful peek, find the record end */
    if (rc >= 0) {
	char *sp;

	t = action == 0 ? 1 : action < 0 ? -action : action;
	for (endbuf = (sp = buf) + r; sp < endbuf;)
	    if (*sp++ == rc)
		if ((t -= 1) == 0)
		    break;
	r = sp - buf;
    }

    /* advance */
    if (action <= 0)
	r = read(fd, buf, r);

    return r;
}
