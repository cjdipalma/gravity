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

#include "g_emitc.h"
#include "g_vcm.h"
#include "g.h"

#define SIG 1298343576
#define MAX_LAYERS  10

typedef int    (*version_fnc_t)        (void);
typedef size_t (*activate_memory_fnc_t)(void);
typedef size_t (*train_memory_fnc_t)   (void);
typedef void   (*initialize_fnc_t)     (void *);
typedef void  *(*activate_fnc_t)       (void *, const void *);
typedef void   (*train_fnc_t)          (void *, const void *, const void *);

struct g {
	unsigned sig;
	g__vcm_t vcm;
	void *activate_m;
	void *train_m;
	version_fnc_t version;
	activate_memory_fnc_t activate_memory;
	train_memory_fnc_t train_memory;
	initialize_fnc_t initialize;
	activate_fnc_t activate;
	train_fnc_t train;
};

static int populate(const char *pathname,
		    const char *module,
		    const char *prefix,
		    const char *precision,
		    const char *costfnc,
		    const char *batch,
		    const char *eta,
		    const char *input,
		    const char *output,
		    const char *layers[]) {
	FILE *file;
	int i;

	file = fopen(pathname, "w");
	if (!file) {
		G__DEBUG(G__ERR_FILE);
		return -1;
	}
	if (0 > fprintf(file,
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n"
			"%s\n",
			module,
			prefix,
			precision,
			costfnc,
			batch,
			eta,
			input,
			output)) {
		fclose(file);
		G__DEBUG(G__ERR_FILE);
		return -1;
	}
	for (i=0; layers[i]; ++i) {
		if (0 > fprintf(file, "%s\n", layers[i])) {
			fclose(file);
			G__DEBUG(G__ERR_FILE);
			return -1;
		}
	}
	fclose(file);
	return 0;
}

int g_version(void) {
	return G__VERSION;
}

void g_debug(int enabled) {
	if (enabled) {
		g__debug_enabled = 1;
		yyerroron = 1;
	}
}

g_t g_open(const char *precision,
	   const char *costfnc,
	   const char *batch,
	   const char *eta,
	   const char *input,
	   const char *output,
	   /* hidden */ ...) {
	const char *tmp, pathname[64], module[64], *layers[1+MAX_LAYERS];
	const struct g__ir *ir;
	struct g__ann *ann;
	struct g *g;
	va_list va;
	int tag, i;

	/* check arguments */

	i = 0;
	tag = rand();
	memset(layers, 0, sizeof (layers));
	if (!g__strlen(precision) ||
	    !g__strlen(costfnc) ||
	    !g__strlen(batch) ||
	    !g__strlen(eta) ||
	    !g__strlen(input) ||
	    !g__strlen(output)) {
		G__DEBUG(G__ERR_ARGUMENT);
		return 0;
	}
	va_start(va, output);
	do {
		layers[i] = va_arg(va, const char *);
		if (MAX_LAYERS <= i) {
			va_end(va);
			G__DEBUG(G__ERR_ARGUMENT);
			return 0;
		}
	}
	while (layers[i++]);
	va_end(va);
	if (!layers[0]) {
		G__DEBUG(G__ERR_ARGUMENT);
		return 0;
	}

	/* initialize */

	g = g__malloc(sizeof (struct g));
	if (!g) {
		G__DEBUG(0);
		return 0;
	}
	memset(g, 0, sizeof (struct g));
	g->sig = SIG;

	/* populate */

	tmp = getenv("TMPDIR");
	tmp = tmp ? tmp : getenv("TMP");
	tmp = tmp ? tmp : getenv("TEMP");
	tmp = tmp ? tmp : ".";
	g__sprintf((char *)pathname, sizeof (pathname), "%s/_%x_.g", tmp, tag);
	g__sprintf((char *)module, sizeof (module), ".module \"_%x_\"", tag);
	if (populate(pathname,
		     module,
		     ".prefix \"\"",
		     precision,
		     costfnc,
		     batch,
		     eta,
		     input,
		     output,
		     layers)) {
		g_close(g);
		G__DEBUG(0);
		return 0;
	}

	/* g compile */

	ir = g__ir_parse(pathname);
	g__unlink(pathname);
	if (!ir) {
		g_close(g);
		G__DEBUG(0);
		return 0;
	}
	ann = g__ann_open(ir);
	g__ir_destroy();
	if (!ann || g__emitc(ann, tmp)) {
		g__ann_close(ann);
		g_close(g);
		G__DEBUG(0);
		return 0;
	}
	g__ann_close(ann);

	/* c compile */

	g__sprintf((char *)pathname, sizeof (pathname), "%s/_%x_.c", tmp, tag);
	g->vcm = g__vcm_open(pathname);
	g__unlink(pathname);
	g__sprintf((char *)pathname, sizeof (pathname), "%s/_%x_.h", tmp, tag);
	g__unlink(pathname);
	if (!g->vcm) {
		g_close(g);
		G__DEBUG(0);
		return 0;
	}

	/* jit connect */

	g->version = (version_fnc_t)(long)
		g__vcm_lookup(g->vcm, "_version");
	g->activate_memory = (activate_memory_fnc_t)(long)
		g__vcm_lookup(g->vcm, "_activate_memory");
	g->train_memory = (train_memory_fnc_t)(long)
		g__vcm_lookup(g->vcm, "_train_memory");
	g->initialize = (initialize_fnc_t)(long)
		g__vcm_lookup(g->vcm, "_initialize");
	g->activate = (activate_fnc_t)(long)
		g__vcm_lookup(g->vcm, "_activate");
	g->train = (train_fnc_t)(long)
		g__vcm_lookup(g->vcm, "_train");
	assert( g->version &&
		g->activate_memory &&
		g->train_memory &&
		g->initialize &&
		g->activate &&
		g->train );
	assert( G__VERSION == g->version() );

	/* allocate ANN memory */

	g->activate_m = g__malloc(g->activate_memory());
	g->train_m = g__malloc(g->train_memory());
	if (!g->activate_m || !g->train_m) {
		g_close(g);
		G__DEBUG(0);
		return 0;
	}
	memset(g->activate_m, 0, g->activate_memory());
	memset(g->train_m, 0, g->train_memory());
	g->initialize(g->train_m);
	g->activate_m = g->train_m;
	return g;
}

void g_close(g_t g) {
	if (g && (SIG == g->sig)) {
		g__vcm_close(g->vcm);
		/*G__FREE(g->activate_m);*/
		G__FREE(g->train_m);
		memset(g, 0, sizeof (struct g));
		G__FREE(g);
	}
}

size_t g_activate_memory(g_t g) {
	if (!g || (SIG != g->sig)) {
		G__DEBUG(G__ERR_ARGUMENT);
		return 0;
	}
	return g->activate_memory();
}

size_t g_train_memory(g_t g) {
	if (!g || (SIG != g->sig)) {
		G__DEBUG(G__ERR_ARGUMENT);
		return 0;
	}
	return g->train_memory();
}

void *g_activate(g_t g, const void *x) {
	if (!g || (SIG != g->sig) || !x) {
		G__DEBUG(G__ERR_ARGUMENT);
		return 0;
	}
	return g->activate(g->activate_m, x);
}

int g_train(g_t g, const void *x, const void *y) {
	if (!g || (SIG != g->sig) | !x || !y) {
		G__DEBUG(G__ERR_ARGUMENT);
		return -1;
	}
	g->train(g->train_m, x, y);
	return 0;
}
