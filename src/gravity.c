/**
 * gravity.c
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

#include "g_emitc.h"

int main(int argc, char *argv[]) {
	const struct g_ir *ir;
	const char *pathname;
	struct g_ann *ann;
	int i;

	pathname = 0;
	for (i=1; i<argc; ++i) {
		if (!strcmp("--version", argv[i])) {
			printf("The Gravity Compiler %d.%d\n",
			       G_VERSION / 10,
			       G_VERSION % 10);
		}
		else if (!strcmp("--debug", argv[i])) {
			g_debug_enabled = 1;
		}
		else {
			if (pathname) {
				pathname = 0;
				break;
			}
			pathname = argv[i];
		}
	}
	if (!pathname) {
		printf("usage: gravity [--verion][--debug] input\n");
		return -1;
	}
	ir = g_ir_parse(pathname);
	if (!ir) {
		G_DEBUG(0);
		return -1;
	}
	ann = g_ann_open(ir);
	g_ir_destroy();
	if (!ann || g_emitc(ann)) {
		g_ann_close(ann);
		fprintf(stderr, "gravity compiler error (run with --debug)\n");
		G_DEBUG(0);
		return -1;
	}
	g_ann_close(ann);
	return 0;
}
