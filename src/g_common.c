/**
 * g_common.c
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

#define _GNU_SOURCE

#include <unistd.h>
#include "g_common.h"

int g_debug_enabled;

void g_sprintf(char *str, size_t size, const char *format, ...) {
	va_list ap;

	assert( str && size && format );

	va_start(ap, format);
	if (size <= (size_t)vsnprintf(str, size, format, ap)) {
		G_DEBUG(G_ERR_SOFTWARE);
		assert( 0 );
		exit(-1);
	}
	va_end(ap);
}

void *g_malloc(size_t n) {
	void *p;

	p = malloc(n);
	if (!p) {
		G_DEBUG(G_ERR_MEMORY);
		return 0;
	}
	return p;
}

const char *g_strdup(const char *s_) {
	char *s;

	s = g_malloc(g_strlen(s_) + 1);
	if (!s) {
		G_DEBUG(0);
		return 0;
	}
	memcpy(s, s_, g_strlen(s_));
	s[g_strlen(s_)] = 0;
	return s;
}

const char *g_error(int e) {
	switch (e) {
	case 0 /*-------*/ : return "^";
	case G_ERR_MEMORY  : return "G_ERR_MEMORY";
	case G_ERR_SYSTEM  : return "G_ERR_SYSTEM";
	case G_ERR_SOFTWARE: return "G_ERR_SOFTWARE";
	case G_ERR_SYNTAX  : return "G_ERR_SYNTAX";
	case G_ERR_FILE    : return "G_ERR_FILE";
	case G_ERR_JITC    : return "G_ERR_JITC";
	}
	return "G_ERR_UNKNOWN";
}

const char *g_pathname_open(const char *tail) {
	const char *tmpdir;
	char *pathname;
	size_t n;

	tmpdir = g_strlen(getenv("TMPDIR")) ? getenv("TMPDIR") : ".";
	n = g_strlen(tmpdir) + g_strlen(tail) + 32;
	pathname = g_malloc(n);
	if (!pathname) {
		G_DEBUG(0);
		return 0;
	}
	g_sprintf(pathname, n, "%s/%x%x%s", tmpdir, rand(), rand(), tail);
	return pathname;
}

void g_pathname_close(const char *pathname) {
	if (g_strlen(pathname)) {
		unlink(pathname);
		G_FREE(pathname);
	}
}
