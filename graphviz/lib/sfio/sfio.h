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

/*	Public header file for the sfio library
**
**	Written by Kiem-Phong Vo
*/

#include "config.h"

#ifdef HAVE_SYS_TYPES_H
#   include <sys/types.h>
#endif // HAVE_SYS_TYPES_H

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

/* Sfoff_t should be large enough for largest file address */


#define Sfoff_t		long long
#define Sflong_t	long long
#define Sfulong_t	unsigned long long
#define Sfdouble_t	long double

	typedef struct _sfio_s Sfio_t;

    typedef struct _sfdisc_s Sfdisc_t;
    typedef ssize_t(*Sfread_f)
	(Sfio_t *, void *, size_t, Sfdisc_t *);
    typedef ssize_t(*Sfwrite_f)
	(Sfio_t *, const void *, size_t, Sfdisc_t *);
    typedef Sfoff_t(*Sfseek_f) (Sfio_t *, Sfoff_t, int, Sfdisc_t *);
    typedef int (*Sfexcept_f) (Sfio_t *, int, void *, Sfdisc_t *);

/* discipline structure */
    struct _sfdisc_s {
	Sfread_f readf;		/* read function                */
	Sfwrite_f writef;	/* write function               */
	Sfseek_f seekf;		/* seek function                */
	Sfexcept_f exceptf;	/* to handle exceptions         */
	Sfdisc_t *disc;		/* the continuing discipline    */
    };

/* formatting environment */
    typedef struct _sffmt_s Sffmt_t;
    typedef int (*Sffmtext_f)(void *, Sffmt_t *);
    typedef int (*Sffmtevent_f)(Sfio_t *, int, void *, Sffmt_t *);
    struct _sffmt_s {
	Sffmtext_f extf;	/* function to process arguments        */
	Sffmtevent_f eventf;	/* process events                       */

	char *form;		/* format string to stack               */
	va_list args;		/* corresponding arg list               */

	int fmt;		/* format character                     */
	ssize_t size;		/* object size                          */
	int flags;		/* formatting flags                     */
	int width;		/* width of field                       */
	int precis;		/* precision required                   */
	int base;		/* conversion base                      */

	char *t_str;		/* type string                          */
	ssize_t n_str;		/* length of t_str                      */
    };

#define SFFMT_SSHORT	00000010	/* 'hh' flag, char                     */
#define SFFMT_TFLAG	00000020	/* 't' flag, ptrdiff_t                 */
#define SFFMT_ZFLAG	00000040	/* 'z' flag, size_t                    */

#define SFFMT_LEFT	00000100	/* left-justification                  */
#define SFFMT_SIGN	00000200	/* must have a sign                    */
#define SFFMT_BLANK	00000400	/* if not signed, prepend a blank      */
#define SFFMT_ZERO	00001000	/* zero-padding on the left            */
#define SFFMT_ALTER	00002000	/* alternate formatting                */
#define SFFMT_THOUSAND	00004000	/* thousand grouping                   */
#define SFFMT_SKIP	00010000	/* skip assignment in scanf()          */
#define SFFMT_SHORT	00020000	/* 'h' flag                            */
#define SFFMT_LONG	00040000	/* 'l' flag                            */
#define SFFMT_LLONG	00100000	/* 'll' flag                           */
#define SFFMT_LDOUBLE	00200000	/* 'L' flag                            */
#define SFFMT_VALUE	00400000	/* value is returned                   */
#define SFFMT_ARGPOS	01000000	/* getting arg for $ patterns          */
#define SFFMT_IFLAG	02000000	/* 'I' flag                            */
#define SFFMT_JFLAG	04000000	/* 'j' flag, intmax_t                  */
#define SFFMT_SET	07777770	/* flags settable on calling extf      */

/* bits for various types of files */
#define	SF_READ		0000001	/* open for reading                     */
#define SF_WRITE	0000002	/* open for writing                     */
#define SF_STRING	0000004	/* a string stream                      */

#define SF_APPENDWR	0000010	/* file is in append mode only.         */

#define SF_MALLOC	0000020	/* buffer is malloc-ed                  */
#define SF_LINE		0000040	/* line buffering                       */
#define SF_SHARE	0000100	/* stream with shared file descriptor   */
#define SF_EOF		0000200	/* eof was detected                     */
#define SF_STATIC	0001000	/* a stream that cannot be freed        */
#define SF_IOCHECK	0002000	/* call exceptf before doing IO         */
#define SF_PUBLIC	0004000	/* SF_SHARE and follow physical seek    */
#define SF_WHOLE	0020000	/* preserve wholeness of sfwrite/sfputr */

#define SF_FLAGS	0077177	/* PUBLIC FLAGS PASSABLE TO SFNEW()     */
#define SF_SETS		0027163	/* flags passable to sfset()            */

/* exception events: SF_NEW(0), SF_READ(1), SF_WRITE(2) and the below 	*/
#define SF_SEEK		3	/* seek error                           */

#define SF_CLOSING	4	/* stream is about to be closed.        */

#define SF_DPUSH	5	/* when discipline is being pushed      */
#define SF_DPOP		6	/* when discipline is being popped      */
#define SF_DPOLL	7	/* see if stream is ready for I/O       */
#define SF_DBUFFER	8	/* buffer not empty during push or pop  */
#define SF_SYNC		9	/* announcing start/end synchronization */
#define SF_FINAL	11	/* closing is done except stream free   */
#define SF_READY	12	/* a polled stream is ready             */
#define SF_LOCKED	13	/* stream is in a locked state          */
#define SF_ATEXIT	14	/* process is exiting                   */
#define SF_EVENT	100	/* start of user-defined events         */

/* for stack and disciplines */
#define SF_POPSTACK	((Sfio_t*)0)	/* pop the stream stack         */
#define SF_POPDISC	((Sfdisc_t*)0)	/* pop the discipline stack     */

/* for the notify function and discipline exception */
#define SF_NEW		0	/* new stream                           */
#define SF_SETFD	(-1)	/* about to set the file descriptor     */

#define SF_UNBOUND SIZE_MAX // unbounded buffer size

     extern ssize_t _Sfi;

#if defined(_BLD_sfio) && defined(GVDLL)
#define SFIO_API	__declspec(dllexport)
#else
#define SFIO_API /* nothing */
#endif
/* standard in/out/err streams */
    SFIO_API extern Sfio_t *sfstdin;
    SFIO_API extern Sfio_t *sfstdout;
    SFIO_API extern Sfio_t *sfstderr;
    SFIO_API extern Sfio_t _Sfstdin;
    SFIO_API extern Sfio_t _Sfstdout;
    SFIO_API extern Sfio_t _Sfstderr;
#undef SFIO_API


    extern Sfio_t *sfnew(Sfio_t *, void *, size_t, int, int);
    extern Sfio_t *sfopen(const char *, const char *);
    extern Sfio_t *sfstack(Sfio_t *, Sfio_t *);
    extern Sfio_t *sfswap(Sfio_t *, Sfio_t *);
    extern int sfsync(Sfio_t *);
    extern void *sfsetbuf(Sfio_t *, void *, size_t);
    extern Sfdisc_t *sfdisc(Sfio_t *, Sfdisc_t *);
    extern int sfraise(Sfio_t *, int, void *);
    extern int sfsetfd(Sfio_t *, int);
    extern ssize_t sfread(Sfio_t *, void *, size_t);
    extern ssize_t sfwrite(Sfio_t *, const void *, size_t);
    extern int sfclose(Sfio_t *);
    extern ssize_t sfputr(Sfio_t*, const char*);
    extern ssize_t sfnputc(Sfio_t *, int, size_t);
    extern int sfungetc(Sfio_t *, int);
    extern int sfprint(Sfio_t*, Sffmt_t *format);
    extern int sfscanf(Sfio_t *, const char *, ...);
    extern int sfsscanf(const char *, const char *, ...);
    extern int sfvsscanf(const char *, const char *, va_list);
    extern int sfvscanf(Sfio_t *, const char *, va_list);

/* io functions with discipline continuation */
    extern ssize_t sfrd(Sfio_t *, void *, size_t, Sfdisc_t *);
    extern ssize_t sfwr(Sfio_t *, const void *, size_t, Sfdisc_t *);
    extern Sfoff_t sfsk(Sfio_t *, Sfoff_t, int, Sfdisc_t *);
    extern ssize_t sfpkrd(int, void *, size_t, int, int);

    extern int sfputc(Sfio_t *, int);

    extern int sfgetc(Sfio_t *);

    extern int _sfflsbuf(Sfio_t *, int);

    extern int _sffilbuf(Sfio_t *, int);

/* miscellaneous function analogues of fast in-line functions */
    extern int sffileno(Sfio_t *);
    extern ssize_t sfslen(void);

#undef extern

#ifdef __cplusplus
}
#endif
