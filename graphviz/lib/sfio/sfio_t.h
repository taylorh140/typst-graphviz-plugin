/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*	This header file is for library writers who need to know certain
**	internal info concerning the full Sfio_t structure. Including this
**	file means that you agree to track closely with sfio development
**	in case its internal architecture is changed.
**
**	Written by Kiem-Phong Vo
*/

/* the parts of Sfio_t private to sfio functions */
#define _SFIO_PRIVATE \
	Sfoff_t			extent;	/* current file	size		*/ \
	Sfoff_t			here;	/* current physical location	*/ \
	unsigned char		getr;	/* the last sfgetr separator 	*/ \
	unsigned char		tiny[1];/* for unbuffered read stream	*/ \
	unsigned short		bits;	/* private flags		*/ \
	unsigned int		mode;	/* current io mode		*/ \
	struct _sfdisc_s*	disc;	/* discipline			*/ \
	struct _sfpool_s*	pool;	/* the pool containing this	*/ \
	struct _sfrsrv_s*	rsrv;	/* reserved buffer		*/ \
	Sfoff_t			lpos;	/* last seek position		*/ \
	size_t			iosz;	/* prefer size for I/O          */

#include	<sfio/sfio.h>

/* a file structure */
    struct _sfio_s {
	unsigned char *next;	/* next position to read/write from     */
	unsigned char *endw;	/* end of write buffer                  */
	unsigned char *endr;	/* end of read buffer                   */
	unsigned char *endb;	/* end of buffer                        */
	Sfio_t *push;		/* the stream that was pushed on        */
	unsigned short flags;	/* type of stream                       */
	short file;		/* file descriptor                      */
	unsigned char *data;	/* base of data buffer                  */
	ssize_t size;		/* buffer size                          */
	ssize_t val;		/* values or string lengths             */
#ifdef _SFIO_PRIVATE
	 _SFIO_PRIVATE
#endif
    };

/* mode bit to indicate that the structure hasn't been initialized */
#define SF_INIT		0000004u

/* short-hand for common stream types */
#define SF_RDWR		(SF_READ|SF_WRITE)
#define SF_RDSTR	(SF_READ|SF_STRING)
#define SF_WRSTR	(SF_WRITE|SF_STRING)
#define SF_RDWRSTR	(SF_RDWR|SF_STRING)

/* for static initialization of an Sfio_t structure */
#define SFNEW(data,size,file,type,disc,mutex)	\
	{ (unsigned char*)(data),			/* next		*/ \
	  (unsigned char*)(data),			/* endw		*/ \
	  (unsigned char*)(data),			/* endr		*/ \
	  (unsigned char*)(data),			/* endb		*/ \
	  (Sfio_t*)0,					/* push		*/ \
	  (unsigned short)((type)&SF_FLAGS),		/* flags	*/ \
	  (short)(file),				/* file		*/ \
	  (unsigned char*)(data),			/* data		*/ \
	  (ssize_t)(size),				/* size		*/ \
	  (ssize_t)(-1),				/* val		*/ \
	  (Sfoff_t)0,					/* extent	*/ \
	  (Sfoff_t)0,					/* here		*/ \
	  0,						/* getr		*/ \
	  {0},						/* tiny		*/ \
	  0,						/* bits		*/ \
	  (unsigned int)(((type)&(SF_RDWR))|SF_INIT),	/* mode		*/ \
	  (struct _sfdisc_s*)(disc),			/* disc		*/ \
	  (struct _sfpool_s*)0,				/* pool		*/ \
	  (struct _sfrsrv_s*)0,				/* rsrv		*/ \
	  (Sfoff_t)0,					/* lpos		*/ \
	  (size_t)0					/* iosz		*/ \
	}

/* function to clear an Sfio_t structure */
#define SFCLEAR(f) \
	( (f)->next = (unsigned char*)0,		/* next		*/ \
	  (f)->endw = (unsigned char*)0,		/* endw		*/ \
	  (f)->endr = (unsigned char*)0,		/* endr		*/ \
	  (f)->endb = (unsigned char*)0,		/* endb		*/ \
	  (f)->push = (Sfio_t*)0,			/* push		*/ \
	  (f)->flags = (unsigned short)0,		/* flags	*/ \
	  (f)->file = -1,				/* file		*/ \
	  (f)->data = (unsigned char*)0,		/* data		*/ \
	  (f)->size = (ssize_t)(-1),			/* size		*/ \
	  (f)->val = (ssize_t)(-1),			/* val		*/ \
	  (f)->extent = (Sfoff_t)(-1),			/* extent	*/ \
	  (f)->here = (Sfoff_t)0,			/* here		*/ \
	  (f)->getr = 0,				/* getr		*/ \
	  (f)->tiny[0] = 0,				/* tiny		*/ \
	  (f)->bits = 0,				/* bits		*/ \
	  (f)->mode = 0,				/* mode		*/ \
	  (f)->disc = (struct _sfdisc_s*)0,		/* disc		*/ \
	  (f)->pool = (struct _sfpool_s*)0,		/* pool		*/ \
	  (f)->rsrv = (struct _sfrsrv_s*)0,		/* rsrv		*/ \
	  (f)->lpos = (Sfoff_t)0,			/* lpos		*/ \
	  (f)->iosz = (size_t)0				/* iosz		*/ \
	)

#ifdef __cplusplus
}
#endif
