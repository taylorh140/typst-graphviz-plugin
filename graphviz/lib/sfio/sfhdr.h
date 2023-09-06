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

#if !defined(_BLD_sfio)
#define _BLD_sfio	1
#endif

#include <inttypes.h>

/*	Internal definitions for sfio.
**	Written by Kiem-Phong Vo
*/

#include	<sfio/sfio_t.h>
#include	"config.h"

#if defined(__mips) && __mips == 2 && !defined(_NO_LARGEFILE64_SOURCE)
#define _NO_LARGEFILE64_SOURCE  1
#endif
#if !defined(_NO_LARGEFILE64_SOURCE) && \
	defined(HAVE_LSEEK64) && defined(HAVE_STAT64) && defined(HAVE_OFF64_T) && \
	defined(HAVE_STRUCT_STAT64)
#	if !defined(_LARGEFILE64_SOURCE)
#	define _LARGEFILE64_SOURCE     1
#	endif
#else
#	undef  _LARGEFILE64_SOURCE
#endif

#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdint.h>
#include	<stddef.h>

#ifdef HAVE_SYS_STAT_H
#	include	<sys/stat.h>
#endif /*HAVE_SYS_STAT_H*/

#include	<fcntl.h>

#include	<unistd.h>

#include	<errno.h>
#include	<ctype.h>

#define SFMTXSTART(f,v)		{ if(!f) return(v); }
#define SFMTXRETURN(f,v)	{ return(v); }

/* 64-bit vs 32-bit file stuff */
#ifdef HAVE_SYS_STAT_H
#ifdef _LARGEFILE64_SOURCE
    typedef struct stat64 Stat_t;
#define	lseek		lseek64
#define stat		stat64
#define fstat		fstat64
#define off_t		off64_t
#else
    typedef struct stat Stat_t;
#endif
#endif

/* Private flags in the "bits" field */
#define SF_MMAP		00000001	/* in memory mapping mode               */
#define SF_BOTH		00000002	/* both read/write                      */
#define SF_HOLE		00000004	/* a hole of zero's was created         */
#define SF_NULL		00000010	/* stream is /dev/null                  */
#define SF_SEQUENTIAL	00000020	/* sequential access                    */
#define SF_JUSTSEEK	00000040	/* just did a sfseek                    */

/* on closing, don't be a hero about reread/rewrite on interrupts */
#define SF_ENDING	00000400

#define SF_DCDOWN	00001000	/* recurse down the discipline stack    */

/* bits for the mode field, SF_INIT defined in sfio_t.h */
#define SF_RC		00000010u	/* peeking for a record                 */
#define SF_RV		00000020u	/* reserve without read or most write   */
#define SF_LOCK		00000040u	/* stream is locked for io op           */
#define SF_PUSH		00000100u	/* stream has been pushed               */
#define SF_POOL		00000200u	/* stream is in a pool but not current  */
#define SF_PEEK		00000400u	/* there is a pending peek              */
#define SF_PKRD		00001000u	/* did a peek read                      */
#define SF_GETR		00002000u	/* did a getr on this stream            */
#define SF_SYNCED	00004000u	/* stream was synced                    */
#define SF_STDIO	00010000u	/* given up the buffer to stdio         */
#define SF_AVAIL	00020000u	/* was closed, available for reuse      */
#define SF_LOCAL	00100000u	/* sentinel for a local call            */

/* short-hands */
#ifndef uchar
#define uchar		unsigned char
#endif
#ifndef ulong
#define ulong		uint64_t
#endif
#ifndef uint
#define uint		unsigned int
#endif
#ifndef ushort
#define ushort		unsigned short
#endif

/* macros do determine stream types from Stat_t data */
#ifndef S_IFMT
#define S_IFMT	0
#endif
#ifndef S_IFDIR
#define S_IFDIR	0
#endif
#ifndef S_IFREG
#define S_IFREG	0
#endif
#ifndef S_IFCHR
#define S_IFCHR	0
#endif

#ifndef S_ISDIR
#define S_ISDIR(m)	(((m)&S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(m)	(((m)&S_IFMT) == S_IFREG)
#endif
#ifndef S_ISCHR
#define S_ISCHR(m)	(((m)&S_IFMT) == S_IFCHR)
#endif

#if defined(S_IRUSR) && defined(S_IWUSR) && defined(S_IRGRP) && defined(S_IWGRP) && defined(S_IROTH) && defined(S_IWOTH)
#define SF_CREATMODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#else
#define SF_CREATMODE	0666
#endif

/* a couple of error number that we use, default values are like Linux */
#ifndef EINTR
#define EINTR	4
#endif
#ifndef EBADF
#define EBADF	9
#endif
#ifndef EAGAIN
#define EAGAIN	11
#endif
#ifndef EINVAL
#define EINVAL	22
#endif
/* function to get the decimal point for local environment */
#include	<locale.h>
#define SFSETLOCALE(decimal,thousand) \
	{ struct lconv*	lv_; \
	  if((decimal) == 0) \
	  { (decimal) = '.'; \
	    if((lv_ = localeconv())) \
	    { if(lv_->decimal_point && lv_->decimal_point[0]) \
	    	(decimal) = lv_->decimal_point[0]; \
	      if(lv_->thousands_sep && lv_->thousands_sep[0]) \
	    	(thousand) = lv_->thousands_sep[0]; \
	    } \
	  } \
	}
/* stream pool structure. */
    typedef struct _sfpool_s Sfpool_t;
    struct _sfpool_s {
	Sfpool_t *next;
	int mode;		/* type of pool                 */
	int s_sf;		/* size of pool array           */
	int n_sf;		/* number currently in pool     */
	Sfio_t **sf;		/* array of streams             */
	Sfio_t *array[3];	/* start with 3                 */
    };

/* reserve buffer structure */
    typedef struct _sfrsrv_s Sfrsrv_t;
    struct _sfrsrv_s {
	ssize_t slen;		/* last string length           */
	ssize_t size;		/* buffer size                  */
	uchar data[1];		/* data buffer                  */
    };

/* extensions to sfvprintf/sfvscanf */
#define FP_SET(fp,fn)	(fp < 0 ? (fn += 1) : (fn = fp) )
#define FP_WIDTH	0
#define FP_PRECIS	1
#define FP_BASE		2
#define FP_STR		3
#define FP_SIZE		4
#define FP_INDEX	5	/* index size   */

    typedef struct _fmt_s Fmt_t;
    typedef struct _fmtpos_s Fmtpos_t;
    typedef union {
	int i, *ip;
	long l, *lp;
	short h, *hp;
	uint ui;
	ulong ul;
	ushort uh;
	Sflong_t ll, *llp;
	Sfulong_t lu;
	Sfdouble_t ld;
	double d;
	float f;
	char c, *s, **sp;
	void *vp;
	Sffmt_t *ft;
    } Argv_t;

    struct _fmt_s {
	char *form;		/* format string                */
	va_list args;		/* corresponding arglist        */

	char *oform;		/* original format string       */
	va_list oargs;		/* original arg list            */
	int argn;		/* number of args already used  */
	Fmtpos_t *fp;		/* position list                */

	Sffmt_t *ft;		/* formatting environment       */
	Sffmtevent_f eventf;	/* event function               */
	Fmt_t *next;		/* stack frame pointer          */
    };

    struct _fmtpos_s {
	Sffmt_t ft;		/* environment                  */
	Argv_t argv;		/* argument value               */
	int fmt;		/* original format              */
	int need[FP_INDEX];	/* positions depending on       */
    };

#define LEFTP		'('
#define RIGHTP		')'
#define QUOTE		'\''

#define FMTSET(ft, frm,ags, fv, sz, flgs, wid,pr,bs, ts,ns) \
	((ft->form = (char*)frm), va_copy(ft->args,ags), \
	 (ft->fmt = fv), (ft->size = sz), \
	 (ft->flags = (flgs&SFFMT_SET)), \
	 (ft->width = wid), (ft->precis = pr), (ft->base = bs), \
	 (ft->t_str = ts), (ft->n_str = ns) )
#define FMTGET(ft, frm,ags, fv, sz, flgs, wid,pr,bs) \
	((frm = ft->form), va_copy(ags,ft->args), (fv = ft->fmt), (sz = ft->size), \
	 (flgs = (flgs&~(SFFMT_SET))|(ft->flags&SFFMT_SET)), \
	 (wid = ft->width), (pr = ft->precis), (bs = ft->base) )
#define FMTCMP(sz, type, maxtype) \
	(sz == sizeof(type) || (sz == 0 && sizeof(type) == sizeof(maxtype)) || \
	 (sz == 64 && sz == sizeof(type)*CHAR_BIT) )

/* format flags&types, must coexist with those in sfio.h */
#define SFFMT_EFORMAT	01000000000	/* sfcvt converting %e  */
#define SFFMT_MINUS	02000000000	/* minus sign           */

#define SFFMT_TYPES	(SFFMT_SHORT|SFFMT_SSHORT | SFFMT_LONG|SFFMT_LLONG|\
			 SFFMT_LDOUBLE | SFFMT_IFLAG|SFFMT_JFLAG| \
			 SFFMT_TFLAG | SFFMT_ZFLAG )

/* type of elements to be converted */
#define SFFMT_INT	001	/* %d,%i                */
#define SFFMT_UINT	002	/* %u,o,x etc.          */
#define SFFMT_FLOAT	004	/* %f,e,g etc.          */
#define SFFMT_BYTE	010	/* %c                   */
#define SFFMT_POINTER	020	/* %p, %n               */
#define SFFMT_CLASS	040	/* %[                   */

/* local variables used across sf-functions */
#define _Sfpage		(_Sfextern.sf_page)
#define _Sfpool		(_Sfextern.sf_pool)
#define _Sfpmove	(_Sfextern.sf_pmove)
#define _Sfstack	(_Sfextern.sf_stack)
#define _Sfnotify	(_Sfextern.sf_notify)
#define _Sfstdsync	(_Sfextern.sf_stdsync)
#define _Sfudisc	(&(_Sfextern.sf_udisc))
#define _Sfcleanup	(_Sfextern.sf_cleanup)
#define _Sfexiting	(_Sfextern.sf_exiting)
#define _Sfdone		(_Sfextern.sf_done)
    typedef struct _sfextern_s {
	ssize_t sf_page;
	struct _sfpool_s sf_pool;
	int (*sf_pmove) (Sfio_t *, int);
	Sfio_t *(*sf_stack) (Sfio_t *, Sfio_t *);
	void (*sf_notify) (Sfio_t *, int, int);
	int (*sf_stdsync) (Sfio_t *);
	struct _sfdisc_s sf_udisc;
	void (*sf_cleanup) (void);
	int sf_exiting;
	int sf_done;
    } Sfextern_t;

/* grain size for buffer increment */
#define SF_GRAIN	1024

/* when the buffer is empty, certain io requests may be better done directly
   on the given application buffers. The below condition determines when.
*/
#define SFDIRECT(f,n)	(((ssize_t)(n) >= (f)->size) || \
			 ((n) >= SF_GRAIN && (ssize_t)(n) >= (f)->size/16 ) )

/* the bottomless bit bucket */
#define DEVNULL		"/dev/null"
#define SFSETNULL(f)	((f)->extent = (Sfoff_t)(-1), (f)->bits |= SF_NULL)
#define SFISNULL(f)	((f)->extent < 0 && ((f)->bits&SF_NULL) )

#define SFKILL(f)	((f)->mode = (SF_AVAIL|SF_LOCK) )
#define SFKILLED(f)	(((f)->mode&(SF_AVAIL|SF_LOCK)) == (SF_AVAIL|SF_LOCK) )

/* exception types */
#define SF_EDONE	0	/* stop this operation and return       */
#define SF_EDISC	1	/* discipline says it's ok              */
#define SF_ESTACK	2	/* stack was popped                     */
#define SF_ECONT	3	/* can continue normally                */

#define SETLOCAL(f)	((f)->mode |= SF_LOCAL)
#define GETLOCAL(f,v)	((v) = ((f)->mode&SF_LOCAL), (f)->mode &= ~SF_LOCAL, (void)(v))
#define SFWRALL(f)	((f)->mode |= SF_RV)
#define SFISALL(f,v)	((((v) = (f)->mode&SF_RV) ? ((f)->mode &= ~SF_RV) : 0), \
			 ((v) || (f)->extent < 0 || \
			  ((f)->flags&(SF_SHARE|SF_APPENDWR|SF_WHOLE)) ) )
#define SFSK(f,a,o,d)	(SETLOCAL(f),sfsk(f,(Sfoff_t)a,o,d))
#define SFRD(f,b,n,d)	(SETLOCAL(f),sfrd(f,(void*)b,n,d))
#define SFWR(f,b,n,d)	(SETLOCAL(f),sfwr(f,(void*)b,n,d))
#define SFSYNC(f)	(SETLOCAL(f),sfsync(f))
#define SFCLOSE(f)	(SETLOCAL(f),sfclose(f))
#define SFFLSBUF(f,n)	(SETLOCAL(f),_sfflsbuf(f,n))
#define SFFILBUF(f)	(SETLOCAL(f), _sffilbuf(f ,-1))
#define SFSETBUF(f,s,n)	(SETLOCAL(f),sfsetbuf(f,s,n))
#define SFWRITE(f,s,n)	(SETLOCAL(f),sfwrite(f,s,n))
#define SFREAD(f,s,n)	(SETLOCAL(f),sfread(f,s,n))
#define SFNPUTC(f,c,n)	(SETLOCAL(f),sfnputc(f,c,n))
#define SFRAISE(f,e,d)	(SETLOCAL(f),sfraise(f,e,d))

/* lock/open a stream */
#define SFMODE(f,l)	((f)->mode & ~(SF_RV|SF_RC|((l) ? SF_LOCK : 0)) )
#define SFLOCK(f,l)	(void)((f)->mode |= SF_LOCK, (f)->endr = (f)->endw = (f)->data)
#define _SFOPENRD(f)	((f)->endr = (f)->endb)
#define _SFOPENWR(f)	((f)->endw = ((f)->flags&SF_LINE) ? (f)->data : (f)->endb)
#define _SFOPEN(f)	((f)->mode == SF_READ  ? _SFOPENRD(f) : \
			 (f)->mode == SF_WRITE ? _SFOPENWR(f) : \
			 ((f)->endw = (f)->endr = (f)->data) )
#define SFOPEN(f,l)	(void)((l) ? 0 : \
				((f)->mode &= ~(SF_LOCK|SF_RC|SF_RV), _SFOPEN(f), 0) )

/* check to see if the stream can be accessed */
#define SFFROZEN(f)	((f)->mode&(SF_PUSH|SF_LOCK|SF_PEEK) ? 1 : \
			 ((f)->mode&SF_STDIO) ? (*_Sfstdsync)(f) : 0)


/* set discipline code */
#define SFDISC(f,dc,iof) \
	{	Sfdisc_t* d; \
		if(!(dc)) \
			d = (dc) = (f)->disc; \
		else 	d = (f->bits&SF_DCDOWN) ? ((dc) = (dc)->disc) : (dc); \
		while(d && !(d->iof))	d = d->disc; \
		if(d)	(dc) = d; \
	}
#define SFDCRD(f,buf,n,dc,rv) \
	{	int		dcdown = f->bits&SF_DCDOWN; f->bits |= SF_DCDOWN; \
		rv = (*dc->readf)(f,buf,n,dc); \
		if(!dcdown)	f->bits &= (unsigned short)~SF_DCDOWN; \
	}
#define SFDCWR(f,buf,n,dc,rv) \
	{	int		dcdown = f->bits&SF_DCDOWN; f->bits |= SF_DCDOWN; \
		rv = (*dc->writef)(f,buf,n,dc); \
		if(!dcdown)	f->bits &= (unsigned short)~SF_DCDOWN; \
	}
#define SFDCSK(f,addr,type,dc,rv) \
	{	int		dcdown = f->bits&SF_DCDOWN; f->bits |= SF_DCDOWN; \
		rv = (*dc->seekf)(f,addr,type,dc); \
		if(!dcdown)	f->bits &= (unsigned short)~SF_DCDOWN; \
	}

/* fast peek of a stream */
#define _SFAVAIL(f,s,n)	((n) = (f)->endb - ((s) = (f)->next) )
#define SFRPEEK(f,s,n)	(_SFAVAIL(f,s,n) > 0 ? (n) : \
				((n) = SFFILBUF(f), (s) = (f)->next, (n)) )
#define SFWPEEK(f,s,n)	(_SFAVAIL(f,s,n) > 0 ? (n) : \
				((n) = SFFLSBUF(f,-1), (s) = (f)->next, (n)) )

/* more than this for a line buffer, we might as well flush */
#define HIFORLINE	128

/* safe closing function */
#define CLOSE(f)	{ while(close(f) < 0 && errno == EINTR) errno = 0; }

/* string stream extent */
#define SFSTRSIZE(f)	{ Sfoff_t s_ = (f)->next - (f)->data; \
			  if(s_ > (f)->here) \
			    { (f)->here = s_; if(s_ > (f)->extent) (f)->extent = s_; } \
			}

/* control flags for open() */
#ifdef O_CREAT
#define _has_oflags	1
#else				/* for example, research UNIX */
#define _has_oflags	0
#define O_CREAT		004
#define O_TRUNC		010
#define O_APPEND	020
#define O_EXCL		040

#ifndef O_RDONLY
#define	O_RDONLY	000
#endif
#ifndef O_WRONLY
#define O_WRONLY	001
#endif
#ifndef O_RDWR
#define O_RDWR		002
#endif
#endif				/*O_CREAT */

#ifndef O_BINARY
#define O_BINARY	000
#endif
#ifndef O_TEXT
#define O_TEXT		000
#endif

#define	SF_RADIX	64	/* maximum integer conversion base */

/* floating point to ascii conversion */
#define SF_MAXEXP10	6
#define SF_FDIGITS	256	/* max allowed fractional digits */
#define SF_IDIGITS	1024	/* max number of digits in int part */
#define SF_MAXDIGITS	(((SF_FDIGITS+SF_IDIGITS)/sizeof(int) + 1)*sizeof(int))

/* tables for numerical translation */
#define _Sfpos10	(_Sftable.sf_pos10)
#define _Sfneg10	(_Sftable.sf_neg10)
#define _Sfdec		(_Sftable.sf_dec)
#define _Sfdigits	(_Sftable.sf_digits)
#define _Sfcvinitf	(_Sftable.sf_cvinitf)
#define _Sfcvinit	(_Sftable.sf_cvinit)
#define _Sffmtintf	(_Sftable.sf_fmtintf)
#define _Sfcv36		(_Sftable.sf_cv36)
#define _Sfcv64		(_Sftable.sf_cv64)
#define _Sftype		(_Sftable.sf_type)
    typedef struct _sftab_ {
	Sfdouble_t sf_pos10[SF_MAXEXP10];	/* positive powers of 10        */
	Sfdouble_t sf_neg10[SF_MAXEXP10];	/* negative powers of 10        */
	uchar sf_dec[200];	/* ascii reps of values < 100   */
	char *sf_digits;	/* digits for general bases     */
	int (*sf_cvinitf) (void);	/* initialization function      */
	int sf_cvinit;		/* initialization state         */
	char *(*sf_fmtintf) (const char *, int *);
	uchar sf_cv36[UCHAR_MAX + 1];	/* conversion for base [2-36]   */
	uchar sf_cv64[UCHAR_MAX + 1];	/* conversion for base [37-64]  */
	uchar sf_type[UCHAR_MAX + 1];	/* conversion formats&types     */
    } Sftab_t;

/* thread-safe macro/function to initialize _Sfcv* conversion tables */
#define SFCVINIT()      (_Sfcvinit ? 1 : (_Sfcvinit = (*_Sfcvinitf)()) )

/* sfucvt() converts decimal integers to ASCII */
#define SFDIGIT(v,scale,digit) \
	{ if(v < 5*scale) \
		if(v < 2*scale) \
			if(v < 1*scale) \
				{ digit = '0'; } \
			else	{ digit = '1'; v -= 1*scale; } \
		else	if(v < 3*scale) \
				{ digit = '2'; v -= 2*scale; } \
			else if(v < 4*scale) \
				{ digit = '3'; v -= 3*scale; } \
			else	{ digit = '4'; v -= 4*scale; } \
	  else	if(v < 7*scale) \
			if(v < 6*scale) \
				{ digit = '5'; v -= 5*scale; } \
			else	{ digit = '6'; v -= 6*scale; } \
		else	if(v < 8*scale) \
				{ digit = '7'; v -= 7*scale; } \
			else if(v < 9*scale) \
				{ digit = '8'; v -= 8*scale; } \
			else	{ digit = '9'; v -= 9*scale; } \
	}
#define sfucvt(v,s,n,list,type,utype) \
	{ while((utype)v >= 10000) \
	  {	n = v; v = (type)(((utype)v)/10000); \
		n = (type)((utype)n - ((utype)v)*10000); \
	  	s -= 4; SFDIGIT(n,1000,s[0]); SFDIGIT(n,100,s[1]); \
			s[2] = *(list = (char*)_Sfdec + (n <<= 1)); s[3] = *(list+1); \
	  } \
	  if(v < 100) \
	  { if(v < 10) \
	    { 	s -= 1; s[0] = (char)('0'+v); \
	    } else \
	    { 	s -= 2; s[0] = *(list = (char*)_Sfdec + (v <<= 1)); s[1] = *(list+1); \
	    } \
	  } else \
	  { if(v < 1000) \
	    { 	s -= 3; SFDIGIT(v,100,s[0]); \
			s[1] = *(list = (char*)_Sfdec + (v <<= 1)); s[2] = *(list+1); \
	    } else \
	    {	s -= 4; SFDIGIT(v,1000,s[0]); SFDIGIT(v,100,s[1]); \
			s[2] = *(list = (char*)_Sfdec + (v <<= 1)); s[3] = *(list+1); \
	    } \
	  } \
	}

// handy function
#undef min
#define min(x,y)	((x) < (y) ? (x) : (y))

/* note that MEMCPY advances the associated pointers */
#define MEMCPY(to,fr,n) \
	do { memcpy((void*)to,(void*)fr,n); to += n; fr += n; } while (0)

    extern Sfextern_t _Sfextern;
    extern Sftab_t _Sftable;

    extern int _sfmode(Sfio_t *, int, int);
    extern int _sfexcept(Sfio_t *, int, ssize_t, Sfdisc_t *);
    extern Sfrsrv_t *_sfrsrv(Sfio_t *, ssize_t);
    extern int _sfsetpool(Sfio_t *);
    extern char *_sfcvt(void *, int, int *, int *, int);

#ifdef __cplusplus
}
#endif
