/**
 * g_ir.c
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

#include "g_ir.h"

#define MAX_BATCH   1000
#define MAX_SIZE 1000000

#define MARK_MODULE    0
#define MARK_PREFIX    1
#define MARK_PRECISION 2
#define MARK_COSTFNC   3
#define MARK_BATCH     4
#define MARK_ETA       5
#define MARK_INPUT     6
#define MARK_OUTPUT    7
#define MARK_HIDDEN    8
#define MARK_END       9

#define NODE_TYPE_INPUT  0
#define NODE_TYPE_OUTPUT 1
#define NODE_TYPE_HIDDEN 2

static struct {
	void *_mem_;
	struct g_ir *ir;
	int mark[MARK_END];
	struct node {
		int type;
		int size;
		int activation;
		struct node *link;
	} *root;
} state;

static int validc(const char *s) {
	if (!g_strlen(s)) {
		return -1;
	}
	if (('_' != *s) && !isalpha(*s)) {
		return -1;
	}
	while (*s) {
		if (('_' != *s) && !isalnum(*s)) {
			return -1;
		}
		++s;
	}
	return 0;
}

const struct g_ir *g_ir_parse(const char *pathname) {
	FILE *file;

	assert( !state._mem_ && g_strlen(pathname) );

	file = fopen(pathname, "r");
	if (!file) {
		yyerror("unable to open '%s' for reading", pathname);
		g_ir_destroy();
		G_DEBUG(G_ERR_FILE);
		return 0;
	}
	state.ir = g_ir_malloc(sizeof (struct g_ir));
	if (!state.ir) {
		yyerror("out of memory");
		fclose(file);
		g_ir_destroy();
		G_DEBUG(0);
		return 0;
	}
	yyrestart(file);
	if (yyparse()) {
		fclose(file);
		g_ir_destroy();
		G_DEBUG(0);
		return 0;
	}
	fclose(file);
	return state.ir;
}

void g_ir_destroy(void) {
	void **link;

	link = state._mem_;
	while (state._mem_) {
		link = state._mem_;
		state._mem_ = (*link);
		G_FREE(link);
	}
	yylex_destroy();
	memset(&state, 0, sizeof (state));
}

/*-----------------------------------------------------------------------------
 * Lexer/Parser Backend
 *---------------------------------------------------------------------------*/

int g_ir_top(void) {
	struct node *node;
	int i, n;

	if (!state.mark[MARK_MODULE]) {
		yyerror("missing .module sepcification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	if (!state.mark[MARK_PREFIX]) {
		state.ir->prefix = "g";
	}
	if (!state.mark[MARK_BATCH]) {
		yyerror("missing .batch specification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	if (!state.mark[MARK_ETA]) {
		yyerror("missing .eta specification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	if (!state.mark[MARK_INPUT]) {
		yyerror("missing .input specification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	if (!state.mark[MARK_OUTPUT]) {
		yyerror("missing .output specification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	n = 2 + state.mark[MARK_HIDDEN];
	state.ir->layers = n;
	state.ir->nodes = g_ir_malloc(n * sizeof (state.ir->nodes[0]));
	if (!state.ir->nodes) {
		yyerror("out of memory");
		G_DEBUG(0);
		return 0;
	}
	i = 0;
	node = state.root;
	while (node) {
		if (NODE_TYPE_INPUT == node->type) {
			state.ir->nodes[i].size = node->size;
			state.ir->nodes[i].activation = node->activation;
			++i;
		}
		node = node->link;
	}
	node = state.root;
	while (node) {
		if (NODE_TYPE_HIDDEN == node->type) {
			state.ir->nodes[i].size = node->size;
			state.ir->nodes[i].activation = node->activation;
			++i;
		}
		node = node->link;
	}
	node = state.root;
	while (node) {
		if (NODE_TYPE_OUTPUT == node->type) {
			state.ir->nodes[i].size = node->size;
			state.ir->nodes[i].activation = node->activation;
			++i;
		}
		node = node->link;
	}
	return 0;
}

int g_ir_module(const char *s) {
	if (state.mark[MARK_MODULE]) {
		yyerror("duplicate .module specification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	if (!g_strlen(s) || validc(s)) {
		yyerror("invalid .module specification '%s' ", s);
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	state.ir->module = s;
	state.mark[MARK_MODULE] += 1;
	return 0;
}

int g_ir_prefix(const char *s) {
	if (state.mark[MARK_PREFIX]) {
		yyerror("duplicate .prefix specification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	if (validc(s)) {
		yyerror("invalid .prefix specification '%s' ", s);
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	state.ir->prefix = s;
	state.mark[MARK_PREFIX] += 1;
	return 0;
}

int g_ir_precision(long	whole1,
		   long fraction1,
		   long precision1,
		   long	whole2,
		   long fraction2,
		   long precision2) {
	if (state.mark[MARK_PRECISION]) {
		yyerror("duplicate .precision specification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	if ((G_IR_PRECISION_FIXED == precision1) &&
	    ((1 > whole1) || (0 > fraction1) ||
	     (64 < (whole1 + fraction1)))) {
		yyerror("invalid .precision 'FIXED [%ld, %ld]' ",
			whole1,
			fraction1);
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	if ((G_IR_PRECISION_FIXED == precision2) &&
	    ((1 > whole2) || (0 > fraction2) ||
	     (64 < (whole2 + fraction2)))) {
		yyerror("invalid .precision 'FIXED [%ld, %ld]' ",
			whole2,
			fraction2);
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	state.ir->memory[G_IR_MEMORY_ACTIVATE].whole = (int)whole1;
	state.ir->memory[G_IR_MEMORY_ACTIVATE].fraction = (int)fraction1;
	state.ir->memory[G_IR_MEMORY_ACTIVATE].precision = (int)precision1;
	state.ir->memory[G_IR_MEMORY_TRAIN].whole = (int)whole2;
	state.ir->memory[G_IR_MEMORY_TRAIN].fraction = (int)fraction2;
	state.ir->memory[G_IR_MEMORY_TRAIN].precision = (int)precision2;
	state.mark[MARK_PRECISION] += 1;
	return 0;
}

int g_ir_costfnc(long costfnc) {
	if (state.mark[MARK_COSTFNC]) {
		yyerror("duplicate .costfnc specification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	state.ir->costfnc = (int)costfnc;
	state.mark[MARK_COSTFNC] += 1;
	return 0;
}

int g_ir_batch(long batch) {
	if (state.mark[MARK_BATCH]) {
		yyerror("duplicate .batch specification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	if (MAX_BATCH < batch) {
		yyerror("invalid .batch specification '%ld'", batch);
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	state.ir->batch = (int)batch;
	state.mark[MARK_BATCH] += 1;
	return 0;
}

int g_ir_eta(double eta) {
	if (state.mark[MARK_ETA]) {
		yyerror("duplicate .eta specification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	if ((0.0 >= eta) || (1.0 < eta)) {
		yyerror("invalid .eta specification '%f'", eta);
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	state.ir->eta = eta;
	state.mark[MARK_ETA] += 1;
	return 0;
}

int g_ir_input(long size) {
	struct node *node;

	if (state.mark[MARK_INPUT]) {
		yyerror("duplicate .input specification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	if (MAX_SIZE < size) {
		yyerror("invalid .input specification '%ld'", size);
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	node = g_ir_malloc(sizeof (struct node));
	if (!node) {
		yyerror("out of memory");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	node->type = NODE_TYPE_INPUT;
	node->size = (int)size;
	node->link = state.root;
	state.root = node;
	state.mark[MARK_INPUT] += 1;
	return 0;
}

int g_ir_output(long size, long activation) {
	struct node *node;

	if (state.mark[MARK_OUTPUT]) {
		yyerror("duplicate .output specification");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	if (MAX_SIZE < size) {
		yyerror("invalid .output specification '%ld'", size);
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	node = g_ir_malloc(sizeof (struct node));
	if (!node) {
		yyerror("out of memory");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	node->type = NODE_TYPE_OUTPUT;
	node->size = (int)size;
	node->activation = (int)activation;
	node->link = state.root;
	state.root = node;
	state.mark[MARK_OUTPUT] += 1;
	return 0;
}

int g_ir_hidden(long size, long activation) {
	struct node *node;

	if (MAX_SIZE < size) {
		yyerror("invalid .hidden specification '%ld'", size);
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	node = g_ir_malloc(sizeof (struct node));
	if (!node) {
		yyerror("out of memory");
		G_DEBUG(G_ERR_SYNTAX);
		return -1;
	}
	node->type = NODE_TYPE_HIDDEN;
	node->size = (int)size;
	node->activation = (int)activation;
	node->link = state.root;
	state.root = node;
	state.mark[MARK_HIDDEN] += 1;
	return 0;
}

void *g_ir_malloc(size_t n) {
	void **link;

	assert( n );

	n += sizeof (link);
	if (!(link = g_malloc(n))) {
		G_DEBUG(0);
		return 0;
	}
	memset(link, 0, n);
	(*link) = state._mem_;
	state._mem_ = link;
	return (link + 1);
}

char *g_ir_strdup(const char *s_) {
	char *s;

	assert( s_ );

	if (!(s = g_ir_malloc(g_strlen(s_) + 1))) {
		G_DEBUG(0);
		return 0;
	}
	strcpy(s, s_);
	return s;
}
