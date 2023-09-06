/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include	<limits.h>
#include	<sfio/sfhdr.h>
#include	<stddef.h>

/*	Dealing with $ argument addressing stuffs.
**
**	Written by Kiem-Phong Vo.
*/

static char *sffmtint(const char *str, int *v)
{
    for (*v = 0; isdigit((int)*str); ++str)
	*v = *v * 10 + (*str - '0');
    *v -= 1;
    return (char *) str;
}

/* function to initialize conversion tables */
static int sfcvinit(void)
{
    for (int d = 0; d <= UCHAR_MAX; ++d) {
	_Sfcv36[d] = SF_RADIX;
	_Sfcv64[d] = SF_RADIX;
    }

    /* [0-9] */
    unsigned char d;
    for (d = 0; d < 10; ++d) {
	_Sfcv36[(uchar) _Sfdigits[d]] = d;
	_Sfcv64[(uchar) _Sfdigits[d]] = d;
    }

    /* [a-z] */
    for (; d < 36; ++d) {
	_Sfcv36[(uchar) _Sfdigits[d]] = d;
	_Sfcv64[(uchar) _Sfdigits[d]] = d;
    }

    /* [A-Z] */
    for (unsigned char l = 10; d < 62; ++l, ++d) {
	_Sfcv36[(uchar) _Sfdigits[d]] = l;
	_Sfcv64[(uchar) _Sfdigits[d]] = d;
    }

    /* remaining digits */
    for (; d < SF_RADIX; ++d) {
	_Sfcv36[(uchar) _Sfdigits[d]] = d;
	_Sfcv64[(uchar) _Sfdigits[d]] = d;
    }

    _Sftype['d'] = _Sftype['i'] = SFFMT_INT;
    _Sftype['u'] = _Sftype['o'] = _Sftype['x'] = _Sftype['X'] = SFFMT_UINT;
    _Sftype['e'] = _Sftype['E'] =
	_Sftype['g'] = _Sftype['G'] = _Sftype['f'] = SFFMT_FLOAT;
    _Sftype['s'] = _Sftype['n'] = _Sftype['p'] = _Sftype['!'] =
	SFFMT_POINTER;
    _Sftype['c'] = SFFMT_BYTE;
    _Sftype['['] = SFFMT_CLASS;

    return 1;
}

/* table for floating point and integer conversions */
Sftab_t _Sftable = {
    .sf_pos10 = {1e1, 1e2, 1e4, 1e8, 1e16, 1e32},
    .sf_neg10 = {1e-1, 1e-2, 1e-4, 1e-8, 1e-16, 1e-32},

    .sf_dec =
    {'0', '0', '0', '1', '0', '2', '0', '3', '0', '4',
     '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
     '1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
     '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
     '2', '0', '2', '1', '2', '2', '2', '3', '2', '4',
     '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
     '3', '0', '3', '1', '3', '2', '3', '3', '3', '4',
     '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
     '4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
     '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
     '5', '0', '5', '1', '5', '2', '5', '3', '5', '4',
     '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
     '6', '0', '6', '1', '6', '2', '6', '3', '6', '4',
     '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
     '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
     '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
     '8', '0', '8', '1', '8', '2', '8', '3', '8', '4',
     '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
     '9', '0', '9', '1', '9', '2', '9', '3', '9', '4',
     '9', '5', '9', '6', '9', '7', '9', '8', '9', '9',
     },

    .sf_digits = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@_",

    .sf_cvinitf = sfcvinit,
    .sf_fmtintf = sffmtint
};
