/**
 * g_common.h
 * Copyright (C) Tony Givargis, 2019
 *
 * This file is part of The Gravity Compiler.
 *
 * The Gravity Compiler is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. The Gravity Compiler is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with Foobar.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _G_COMMON_H_
#define _G_COMMON_H_

#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#define G_VERSION 10

#define G_ERR_MEMORY   -100
#define G_ERR_SYSTEM   -101
#define G_ERR_SOFTWARE -102
#define G_ERR_SYNTAX   -103
#define G_ERR_FILE     -104
#define G_ERR_JITC     -105

#define G_MIN(a,b) ((a) < (b) ? (a) : (b))
#define G_MAX(a,b) ((a) > (b) ? (a) : (b))
#define G_DUP(a,b) (0 == ((a) % (b)) ? (a) / (b) : (a) / (b) + 1)

#define G_INLINE __attribute__((__unused__)) static

#define G_UNUSED(x) do {			\
		(void)(x);			\
	} while (0)

#define G_DEBUG(e) do {				\
		if (g_debug_enabled) {		\
			fprintf(stderr,		\
				"error:"	\
				" %s:%d: %s\n",	\
				__FILE__,	\
				__LINE__,	\
				g_error(e));	\
		}				\
	} while (0)

#define G_FREE(m) do {				\
		if (m) {			\
			free((void *)(m));	\
			(m) = 0;		\
		}				\
	} while (0)

extern int g_debug_enabled;

void g_free(void *p);

void g_sprintf(char *str, size_t size, const char *format, ...);

void *g_malloc(size_t n);

const char *g_strdup(const char *s);

const char *g_error(int e);

const char *g_pathname_open(const char *tail);

void g_pathname_close(const char *pathname);

G_INLINE size_t g_strlen(const char *s) {
	return s ? strlen(s) : 0;
}

#endif /* _G_COMMON_H_ */
