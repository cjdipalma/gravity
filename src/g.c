/**
 * g.c
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

#include "g_ann.h"
#include "g.h"

struct g {
	int x;
};

int g_version(void) {
	return G__VERSION;
}

g_t g_open(const char *program) {
	if (!g__strlen(program)) {
		G__DEBUG(G__ERR_ARGUMENT);
		return 0;
	}
	return 0;
}

void g_close(g_t g) {
	if (g) {
		memset(g, 0, sizeof (struct g));
	}
	G__FREE(g);
}

void *g_activate(g_t g, const void *x) {
	if (!g || !x) {
		G__DEBUG(G__ERR_ARGUMENT);
		return 0;
	}
	return 0;
}

int g_train(g_t g, const void *x, const void *y) {
	if (!g || !x || !y) {
		G__DEBUG(G__ERR_ARGUMENT);
		return -1;
	}
	return 0;
}
