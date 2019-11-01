/**
 * g_ann.c
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

#define MAX_PROGRAM_CAPACITY 1000

struct g__ann_program_inst *newinst(struct g__ann_program *program) {
	if (MAX_PROGRAM_CAPACITY <= program->size) {
		G__DEBUG(G__ERR_SOFTWARE);
		assert( 0 );
		exit(-1);
		return 0;
	}
	return &program->inst[program->size++];
}

static size_t unit(const struct g__ann_memory *memory) {
	switch (memory->precision) {
	case G__IR_PRECISION_FLOAT : return 4;
	case G__IR_PRECISION_DOUBLE: return 8;
	case G__IR_PRECISION_FIXED : break;/* FIX : not implemented */
	default /*--------------*/ : break;
	}
	G__DEBUG(G__ERR_SOFTWARE);
	assert( 0 );
	exit(-1);
	return 0;
}

static int emit_memory(struct g__ann *ann, const struct g__ir *ir) {
	struct g__ann_memory *memory;
	uint64_t n, m;
	int l;

	memory = &ann->memory;
	memory->whole = ir->memory.whole;
	memory->fraction = ir->memory.fraction;
	memory->precision = ir->memory.precision;
	for (l=1; l<ir->layers; ++l) {
		n = (uint64_t)ir->nodes[l].size;
		m = (uint64_t)ir->nodes[l - 1].size;
		memory->w[l] = memory->size;
		memory->size += unit(memory) * n * m;
		memory->b[l] = memory->size;
		memory->size += unit(memory) * n * 1;
	}
	memory->hard = memory->size;
	for (l=1; l<ir->layers; ++l) {
		n = (uint64_t)ir->nodes[l].size;
		m = (uint64_t)ir->nodes[l - 1].size;
		memory->w_[l] = memory->size;
		memory->size += unit(memory) * n * m;
		memory->b_[l] = memory->size;
		memory->size += unit(memory) * n * 1;
	}
	for (l=0; l<ir->layers; ++l) {
		n = (uint64_t)ir->nodes[l].size;
		memory->a_[l] = memory->size;
		memory->size += unit(memory) * n * 1;
		if (l) {
			memory->d_[l] = memory->size;
			memory->size += unit(memory) * n * 1;
		}
	}
	return 0;
}

static int emit_program_initialize(struct g__ann *ann,
				   const struct g__ir *ir,
				   int program_) {
	struct g__ann_memory *memory;
	struct g__ann_program *program;
	struct g__ann_program_inst *inst;
	uint64_t n, m;
	int l;

	/* setup */

	memory = &ann->memory;
	program = &ann->program[program_];

	/* return */

	inst = newinst(program);
	inst->opc = G__ANN_PROGRAM_INST_RET;
	inst->whole = memory->whole;
	inst->fraction = memory->fraction;
	inst->precision = memory->precision;

	/*
	 * w[*]
	 *   w[*] = random [-6.0 / (n + m) -- +6.0 / (n + m)]
	 *   b[*] = 0
	 */

	for (l=1; l<ann->layers; ++l) {
		n = (uint64_t)ir->nodes[l].size;
		m = (uint64_t)ir->nodes[l - 1].size;
		/*--*/
		inst = newinst(program);
		inst->opc = G__ANN_PROGRAM_INST_RANDOM;
		inst->arg[0].i = memory->w[l];
		inst->arg[1].r = (-6.0 / (n + m)) * 1.0;
		inst->arg[2].r = (+6.0 / (n + m)) * 2.0;
		inst->arg[3].i = n * m;
		inst->whole = memory->whole;
		inst->fraction = memory->fraction;
		inst->precision = memory->precision;
		/*--*/
		inst = newinst(program);
		inst->opc = G__ANN_PROGRAM_INST_CLEAR;
		inst->arg[0].i = memory->b[l];
		inst->arg[1].i = n * 1;
		inst->whole = memory->whole;
		inst->fraction = memory->fraction;
		inst->precision = memory->precision;
	}
	return 0;
}

static int emit_program_activate(struct g__ann *ann,
				 const struct g__ir *ir,
				 int program_) {
	struct g__ann_memory *memory;
	struct g__ann_program *program;
	struct g__ann_program_inst *inst;
	uint64_t n, m;
	int l;

	/* setup */

	memory = &ann->memory;
	program = &ann->program[program_];

	/*
	 * return:
	 *   y := a_[L]
	 */

	l = ann->layers;
	/*--*/
	inst = newinst(program);
	inst->opc = G__ANN_PROGRAM_INST_RETARG;
	inst->arg[0].i = memory->a_[l - 1];
	inst->whole = memory->whole;
	inst->fraction = memory->fraction;
	inst->precision = memory->precision;

	/*
	 * a_[*]:
	 *    a_[0] := x
	 */

	n = (uint64_t)ir->nodes[0].size;
	/*--*/
	inst = newinst(program);
	inst->opc = G__ANN_PROGRAM_INST_COPYX;
	inst->arg[0].i = memory->a_[0];
	inst->arg[1].i = n;
	inst->whole = memory->whole;
	inst->fraction = memory->fraction;
	inst->precision = memory->precision;

	/*
	 * a_[*]:
	 *    a_[l] := activation( w[l] * a_[l - 1] + b[l] )
	 *
	 * activation:
	 *    RELU
	 *    LINEAR
	 *    SOFTMAX
	 *    SIGMOID
	 */

	for (l=1; l<ann->layers; ++l) {
		n = (uint64_t)ir->nodes[l].size;
		m = (uint64_t)ir->nodes[l - 1].size;
		/*--*/
		inst = newinst(program);
		inst->opc = G__ANN_PROGRAM_INST_MAC1;
		inst->arg[0].i = memory->a_[l];
		inst->arg[1].i = memory->w[l];
		inst->arg[2].i = memory->a_[l - 1];
		inst->arg[3].i = n;
		inst->arg[4].i = m;
		inst->whole = memory->whole;
		inst->fraction = memory->fraction;
		inst->precision = memory->precision;
		/*--*/
		inst = newinst(program);
		inst->opc = G__ANN_PROGRAM_INST_ADD;
		inst->arg[0].i = memory->a_[l];
		inst->arg[1].i = memory->b[l];
		inst->arg[2].i = n;
		inst->whole = memory->whole;
		inst->fraction = memory->fraction;
		inst->precision = memory->precision;
		/*--*/
		inst = newinst(program);
		inst->opc = 100 + ir->nodes[l].activation;
		inst->arg[0].i = memory->a_[l];
		inst->arg[1].i = n;
		inst->whole = memory->whole;
		inst->fraction = memory->fraction;
		inst->precision = memory->precision;
	}
	return 0;
}

static int emit_program_backprop(struct g__ann *ann,
				 const struct g__ir *ir,
				 int program_) {
	struct g__ann_memory *memory;
	struct g__ann_program *program;
	struct g__ann_program_inst *inst;
	uint64_t n, m;
	int l;

	/* setup */

	memory = &ann->memory;
	program = &ann->program[program_];

	/* return */

	inst = newinst(program);
	inst->opc = G__ANN_PROGRAM_INST_RET;
	inst->whole = memory->whole;
	inst->fraction = memory->fraction;
	inst->precision = memory->precision;

	/*
	 * d_[*]:
	 *    d_[L] := a_[L] − y
	 */

	l = ann->layers - 1;
	n = (uint64_t)ir->nodes[l].size;
	if ((G__IR_COSTFNC_CROSS_ENTROPY != ir->costfnc) ||
	    (G__IR_ACTIVATION_SOFTMAX != ir->nodes[l].activation)) {

		/*
		 * Currently only cross_entropy cost function and
		 * softmax output activation is supported.
		 */

		G__DEBUG(G__ERR_SOFTWARE);
		return -1;
	}
	/*--*/
	inst = newinst(program);
	inst->opc = G__ANN_PROGRAM_INST_SUBY;
	inst->arg[0].i = memory->d_[l];
	inst->arg[1].i = memory->a_[l];
	inst->arg[2].i = n;
	inst->whole = memory->whole;
	inst->fraction = memory->fraction;
	inst->precision = memory->precision;

	/*
	 * d_[*]:
	 *    d_[l] := (w[l+1]' * d_[l+1]) ⊙ σ′(a_[l])
	 *    d_[1] := (w[l+1]' * d_[l+1])
	 */

	while (1 < l) {
		n = (uint64_t)ir->nodes[l].size;
		m = (uint64_t)ir->nodes[l - 1].size;
		if (G__IR_ACTIVATION_SOFTMAX == ir->nodes[l - 1].activation) {

			/*
			 * Currently not supporting softmax activation
			 * for hidden layers.
			 */

			G__DEBUG(G__ERR_SOFTWARE);
			return -1;
		}
		/*--*/
		inst = newinst(program);
		inst->opc = G__ANN_PROGRAM_INST_MAC2;
		inst->arg[0].i = memory->d_[l - 1];
		inst->arg[1].i = memory->w[l];
		inst->arg[2].i = memory->d_[l];
		inst->arg[3].i = n;
		inst->arg[4].i = m;
		inst->whole = memory->whole;
		inst->fraction = memory->fraction;
		inst->precision = memory->precision;
		/*--*/
		inst = newinst(program);
		inst->opc = 1000 + ir->nodes[l - 1].activation;
		inst->arg[0].i = memory->d_[l - 1];
		inst->arg[1].i = memory->a_[l - 1];
		inst->arg[2].i = m;
		inst->whole = memory->whole;
		inst->fraction = memory->fraction;
		inst->precision = memory->precision;
		--l;
	}

	/*
	 * b_[*]:
	 *    b_[l] := b_[l] + d_[l]
	 *
	 * w_[*]:
	 *    w_[l] := w_[l] + d_[l] * a_[l - 1]
	 */

	for (l=1; l<ann->layers; ++l) {
		n = (uint64_t)ir->nodes[l].size;
		m = (uint64_t)ir->nodes[l - 1].size;
		/*--*/
		inst = newinst(program);
		inst->opc = G__ANN_PROGRAM_INST_ADD;
		inst->arg[0].i = memory->b_[l];
		inst->arg[1].i = memory->d_[l];
		inst->arg[2].i = n;
		inst->whole = memory->whole;
		inst->fraction = memory->fraction;
		inst->precision = memory->precision;
		/*--*/
		inst = newinst(program);
		inst->opc = G__ANN_PROGRAM_INST_MAC3;
		inst->arg[0].i = memory->w_[l];
		inst->arg[1].i = memory->d_[l];
		inst->arg[2].i = memory->a_[l - 1];
		inst->arg[3].i = n;
		inst->arg[4].i = m;
		inst->whole = memory->whole;
		inst->fraction = memory->fraction;
		inst->precision = memory->precision;
	}
	return 0;
}

static int emit_program_train(struct g__ann *ann,
			      const struct g__ir *ir,
			      int program_) {
	struct g__ann_memory *memory;
	struct g__ann_program *program;
	struct g__ann_program_inst *inst;
	uint64_t n, m, size;
	int l;

	/* setup */

	memory = &ann->memory;
	program = &ann->program[program_];

	/* return */

	inst = newinst(program);
	inst->opc = G__ANN_PROGRAM_INST_RET;
	inst->whole = memory->whole;
	inst->fraction = memory->fraction;
	inst->precision = memory->precision;

	/*
	 * adjustments:
	 *   w_[*] := 0
	 *   b_[*] := 0
	 */

	size = 0;
	for (l=1; l<ann->layers; ++l) {
		n = (uint64_t)ir->nodes[l].size;
		m = (uint64_t)ir->nodes[l - 1].size;
		size += n * m;
		size += n * 1;
	}
	/*--*/
	inst = newinst(program);
	inst->opc = G__ANN_PROGRAM_INST_CLEAR;
	inst->arg[0].i = memory->w_[1];
	inst->arg[1].i = size;
	inst->whole = memory->whole;
	inst->fraction = memory->fraction;
	inst->precision = memory->precision;

	/*
	 * for each (x -> y) pair:
	 *   activate()
	 *   backprop()
	 */

	l = ann->layers;
	n = (uint64_t)ir->nodes[0].size;
	m = (uint64_t)ir->nodes[l - 1].size;
	/*--*/
	inst = newinst(program);
	inst->opc = G__ANN_PROGRAM_INST_BATCHLOOP;
	inst->arg[0].i = ir->batch;
	inst->arg[1].i = n;
	inst->arg[2].i = m;

	/*
	 * w[*]:
	 *    w_[l] := (η / k) * w_[l]
	 *    w[l] := w[l] - w_[l]
	 *
	 * b[*]:
	 *    b_[l] := (η / k) * b_[l]
	 *    b[l] := b[l] - b_[l]
	 */

	for (l=1; l<ann->layers; ++l) {
		n = (uint64_t)ir->nodes[l].size;
		m = (uint64_t)ir->nodes[l - 1].size;
		/*--*/
		inst = newinst(program);
		inst->opc = G__ANN_PROGRAM_INST_MAC4;
		inst->arg[0].i = memory->w[l];
		inst->arg[1].i = memory->w_[l];
		inst->arg[2].r = -((double)ir->eta / (double)ir->batch);
		inst->arg[3].i = n * m;
		inst->whole = memory->whole;
		inst->fraction = memory->fraction;
		inst->precision = memory->precision;
		/*--*/
		inst = newinst(program);
		inst->opc = G__ANN_PROGRAM_INST_MAC4;
		inst->arg[0].i = memory->b[l];
		inst->arg[1].i = memory->b_[l];
		inst->arg[2].r = -((double)ir->eta / (double)ir->batch);
		inst->arg[3].i = n * 1;
		inst->whole = memory->whole;
		inst->fraction = memory->fraction;
		inst->precision = memory->precision;
	}
	return 0;
}

static int emit_program(struct g__ann *ann, const struct g__ir *ir) {
	if (emit_program_initialize(ann,
				    ir,
				    G__ANN_PROGRAM_INITIALIZE) ||
	    emit_program_activate(ann,
				  ir,
				  G__ANN_PROGRAM_ACTIVATE) ||
	    emit_program_backprop(ann,
				  ir,
				  G__ANN_PROGRAM_BACKPROP) ||
	    emit_program_train(ann,
			       ir,
			       G__ANN_PROGRAM_TRAIN)) {
		G__DEBUG(0);
		return -1;
	}
	return 0;
}

struct g__ann *g__ann_open(const struct g__ir *ir) {
	struct g__ann_memory *memory;
	struct g__ann *ann;
	size_t n;
	int i;

	assert( ir );

	/* initialize */

	ann = g__malloc(sizeof (struct g__ann));
	if (!ann) {
		G__DEBUG(0);
		return 0;
	}
	memset(ann, 0, sizeof (struct g__ann));
	ann->layers = ir->layers;
	ann->module = g__strdup(ir->module);
	ann->prefix = g__strdup(ir->prefix);
	if (!ann->module || !ann->prefix) {
		g__ann_close(ann);
		G__DEBUG(0);
		return 0;
	}

	/* memories */

	n = ir->layers * sizeof (uint64_t);
	memory = &ann->memory;
	memory->w = g__malloc(n);
	memory->b = g__malloc(n);
	memory->a_ = g__malloc(n);
	memory->d_ = g__malloc(n);
	memory->w_ = g__malloc(n);
	memory->b_ = g__malloc(n);
	if (!memory->w ||
	    !memory->b ||
	    !memory->a_ ||
	    !memory->d_ ||
	    !memory->w_ ||
	    !memory->b_) {
		g__ann_close(ann);
		G__DEBUG(0);
		return 0;
	}
	memset(memory->w, 0, n);
	memset(memory->b, 0, n);
	memset(memory->a_, 0, n);
	memset(memory->d_, 0, n);
	memset(memory->w_, 0, n);
	memset(memory->b_, 0, n);

	/* programs */

	for (i=0; i<G__ANN_PROGRAM_END; ++i) {
		n = MAX_PROGRAM_CAPACITY * sizeof (struct g__ann_program_inst);
		ann->program[i].inst = g__malloc(n);
		if (!ann->program[i].inst) {
			g__ann_close(ann);
			G__DEBUG(0);
			return 0;
		}
		memset(ann->program[i].inst, 0, n);
	}
	if (emit_memory(ann, ir) || emit_program(ann, ir)) {
		g__ann_close(ann);
		G__DEBUG(0);
		return 0;
	}
	return ann;
}

void g__ann_close(struct g__ann *ann) {
	struct g__ann_memory *memory;
	int i;

	if (ann) {
		memory = &ann->memory;
		G__FREE(memory->w);
		G__FREE(memory->b);
		G__FREE(memory->a_);
		G__FREE(memory->d_);
		G__FREE(memory->w_);
		G__FREE(memory->b_);
		for (i=0; i<G__ANN_PROGRAM_END; ++i) {
			G__FREE(ann->program[i].inst);
		}
		G__FREE(ann->module);
		G__FREE(ann->prefix);
		memset(ann, 0, sizeof (struct g__ann));
	}
	G__FREE(ann);
}
