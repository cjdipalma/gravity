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

struct g_ann_program_inst *newinst(struct g_ann_program *prog) {
	if (MAX_PROGRAM_CAPACITY <= prog->size) {
		G_DEBUG(G_ERR_SOFTWARE);
		assert( 0 );
		exit(-1);
		return 0;
	}
	return &prog->inst[prog->size++];
}

static size_t memory_unit(int whole, int fraction, int precision) {
	G_UNUSED(whole);
	G_UNUSED(fraction);
	switch (precision) {
	case G_IR_PRECISION_FLOAT : return 4;
	case G_IR_PRECISION_DOUBLE: return 8;
	case G_IR_PRECISION_FIXED : break;/* FIX : not implemented */
	default /*-------------*/ : break;
	}
	G_DEBUG(G_ERR_SOFTWARE);
	assert( 0 );
	exit(-1);
	return 0;
}

static int emit_memory(struct g_ann *ann, const struct g_ir *ir) {
	struct g_ann_memory *mem;
	uint64_t n, m;
	int l;

	/* activate */

	mem = &ann->memory[G_ANN_MEMORY_ACTIVATE];
	mem->whole = ir->memory[G_IR_MEMORY_ACTIVATE].whole;
	mem->fraction = ir->memory[G_IR_MEMORY_ACTIVATE].fraction;
	mem->precision = ir->memory[G_IR_MEMORY_ACTIVATE].precision;
	mem->unit = memory_unit(mem->whole, mem->fraction, mem->precision);
	for (l=0; l<ir->layers; ++l) {
		n = (uint64_t)ir->nodes[l].size;
		/* a_ */
		mem->a_[l] = mem->size;
		mem->size += n * 1;
		if (l) {
			m = (uint64_t)ir->nodes[l - 1].size;
			/* w */
			mem->w[l] = mem->size;
			mem->size += n * m;
			/* b */
			mem->b[l] = mem->size;
			mem->size += n * 1;
		}
	}

	/* train */

	mem = &ann->memory[G_ANN_MEMORY_TRAIN];
	mem->whole = ir->memory[G_IR_MEMORY_TRAIN].whole;
	mem->fraction = ir->memory[G_IR_MEMORY_TRAIN].fraction;
	mem->precision = ir->memory[G_IR_MEMORY_TRAIN].precision;
	mem->unit = memory_unit(mem->whole, mem->fraction, mem->precision);
	for (l=0; l<ir->layers; ++l) {
		n = (uint64_t)ir->nodes[l].size;
		/* a_ */
		mem->a_[l] = mem->size;
		mem->size += n * 1;
		/* d_ */
		mem->d_[l] = mem->size;
		mem->size += n * 1;
		if (l) {
			m = (uint64_t)ir->nodes[l - 1].size;
			/* w */
			mem->w[l] = mem->size;
			mem->size += n * m;
			/* b */
			mem->b[l] = mem->size;
			mem->size += n * 1;
			/* w_ */
			mem->w_[l] = mem->size;
			mem->size += n * m;
			/* b_ */
			mem->b_[l] = mem->size;
			mem->size += n * 1;
		}
	}
	return 0;
}

static int emit_program_activate(struct g_ann *ann,
				 const struct g_ir *ir,
				 int mem_,
				 int prog_) {
	struct g_ann_memory *mem;
	struct g_ann_program *prog;
	struct g_ann_program_inst *inst;
	uint64_t n, m;
	int l;

	/* setup */

	mem = &ann->memory[mem_];
	prog = &ann->program[prog_];

	/*
         * return y:
         *   a[L]
         */

	l = ann->layers;
	/*--*/
	inst = newinst(prog);
	inst->opc = G_ANN_PROGRAM_INST_RETARG;
	inst->arg[0].i = mem->a_[l - 1];
	inst->whole = mem->whole;
	inst->fraction = mem->fraction;
	inst->precision = mem->precision;

        /*
         * a[*]:
         *    a[0] := x
         */

	n = ir->nodes[0].size;
	/*--*/
	inst = newinst(prog);
	inst->opc = G_ANN_PROGRAM_INST_COPYX;
	inst->arg[0].i = mem->a_[0];
	inst->arg[1].i = mem->unit * n;

	/*
         * a[*]:
         *    a[l] := RELU( w[l] * a[l - 1] + b[l] )
         *    a[L] := fnc( w[l] * a[l - 1] + b[l] )
         *
         * fnc:
         *    RELU
         *    LINEAR
         *    SOFTMAX
         *    SIGMOID
         */

        for (l=1; l<ann->layers; ++l) {
                n = ir->nodes[l].size;
                m = ir->nodes[l - 1].size;
		/*--*/
		inst = newinst(prog);
		inst->opc = G_ANN_PROGRAM_INST_MUL1;
		inst->arg[0].i = mem->a_[l];
		inst->arg[1].i = mem->w[l];
		inst->arg[2].i = mem->a_[l - 1];
		inst->arg[3].i = n;
		inst->arg[4].i = m;
		inst->whole = mem->whole;
		inst->fraction = mem->fraction;
		inst->precision = mem->precision;
		/*--*/
		inst = newinst(prog);
		inst->opc = G_ANN_PROGRAM_INST_ADD;
		inst->arg[0].i = mem->a_[l];
		inst->arg[1].i = mem->b[l];
		inst->arg[2].i = n;
		inst->whole = mem->whole;
		inst->fraction = mem->fraction;
		inst->precision = mem->precision;
		/*--*/
		inst = newinst(prog);
		inst->opc = 100 + ir->nodes[l].activation;
		inst->arg[0].i = mem->a_[l];
		inst->arg[1].i = n;
		inst->whole = mem->whole;
		inst->fraction = mem->fraction;
		inst->precision = mem->precision;
        }
	return 0;
}

static int emit_program_backprop(struct g_ann *ann,
				 const struct g_ir *ir,
				 int mem_,
				 int prog_) {
	struct g_ann_memory *mem;
	struct g_ann_program *prog;
	struct g_ann_program_inst *inst;
	uint64_t n, m;
	int l;

	/* setup */

	mem = &ann->memory[mem_];
	prog = &ann->program[prog_];

	/* return */

	inst = newinst(prog);
	inst->opc = G_ANN_PROGRAM_INST_RET;
	inst->whole = mem->whole;
	inst->fraction = mem->fraction;
	inst->precision = mem->precision;

	/*
	 * d[*]:
	 *    d[L] := a[L] − y
	 */

	l = ann->layers - 1;
	n = ir->nodes[l].size;
	if ((G_IR_COSTFNC_CROSS_ENTROPY != ir->costfnc) ||
	    (G_IR_ACTIVATION_SOFTMAX != ir->nodes[l].activation)) {

		/*
		 * Currently only cross_entropy cost function and
		 * softmax output activation is supported.
		 */

		G_DEBUG(G_ERR_SOFTWARE);
		return -1;
	}
	/*--*/
	inst = newinst(prog);
	inst->opc = G_ANN_PROGRAM_INST_SUBY;
	inst->arg[0].i = mem->d_[l];
	inst->arg[1].i = mem->a_[l];
	inst->arg[2].i = n;
	inst->whole = mem->whole;
	inst->fraction = mem->fraction;
	inst->precision = mem->precision;

	/*
         * d[*]:
         *    d[l] := (w[l+1]' * d[l+1]) ⊙ σ′(z[l])
         */

	while (1 < l) {
		n = ir->nodes[l].size;
		m = ir->nodes[l - 1].size;
		if (G_IR_ACTIVATION_SOFTMAX == ir->nodes[l - 1].activation) {

			/*
			 * Currently not supporting softmax activation
			 * for hidden layers.
			 */

			G_DEBUG(G_ERR_SOFTWARE);
			return -1;
		}
		/*--*/
		inst = newinst(prog);
		inst->opc = G_ANN_PROGRAM_INST_MUL2;
		inst->arg[0].i = mem->d_[l - 1];
		inst->arg[1].i = mem->w[l];
		inst->arg[2].i = mem->d_[l];
		inst->arg[3].i = n;
		inst->arg[4].i = m;
		inst->whole = mem->whole;
		inst->fraction = mem->fraction;
		inst->precision = mem->precision;
		/*--*/
		inst = newinst(prog);
		inst->opc = 1000 + ir->nodes[l - 1].activation;
		inst->arg[0].i = mem->d_[l - 1];
		inst->arg[1].i = mem->a_[l - 1];
		inst->arg[2].i = m;
		inst->whole = mem->whole;
		inst->fraction = mem->fraction;
		inst->precision = mem->precision;
		--l;
	}

	/*
         * w[*]:
         *    w[l] := w[l] + d[l] * a[l - 1]'
         *
         * b[*]:
         *    b[l] := b[l] + d[l]
         */

	for (l=1; l<ann->layers; ++l) {
		n = ir->nodes[l].size;
		m = ir->nodes[l - 1].size;
		/*--*/
		inst = newinst(prog);
		inst->opc = G_ANN_PROGRAM_INST_ADD;
		inst->arg[0].i = mem->b_[l];
		inst->arg[1].i = mem->d_[l];
		inst->arg[2].i = n;
		inst->whole = mem->whole;
		inst->fraction = mem->fraction;
		inst->precision = mem->precision;
		/*--*/
		inst = newinst(prog);
		inst->opc = G_ANN_PROGRAM_INST_MUL3;
		inst->arg[0].i = mem->w_[l];
		inst->arg[1].i = mem->d_[l];
		inst->arg[2].i = mem->a_[l - 1];
		inst->arg[3].i = n;
		inst->arg[4].i = m;
		inst->whole = mem->whole;
		inst->fraction = mem->fraction;
		inst->precision = mem->precision;
	}


	/*
	 * w[*]:
	 *    w[l] := (η / k) * w[l]
	 *    w[l] := w[l] - w[l]
	 *
	 * b[*]:
	 *    b[l] := (η / k) * b[l]
	 *    b[l] := b[l] - b[l]
	 */

        for (l=1; l<ann->layers; ++l) {
		n = ir->nodes[l].size;
		m = ir->nodes[l - 1].size;
		/*--*/
		inst = newinst(prog);
		inst->opc = G_ANN_PROGRAM_INST_MULS;
		inst->arg[0].i = mem->w_[l];
		inst->arg[1].i = n * m;
		inst->arg[2].r = -((double)ir->eta / (double)ir->batch);
		inst->whole = mem->whole;
		inst->fraction = mem->fraction;
		inst->precision = mem->precision;
		/*--*/
		inst = newinst(prog);
		inst->opc = G_ANN_PROGRAM_INST_ADD;
		inst->arg[0].i = mem->w[l];
		inst->arg[1].i = mem->w_[l];
		inst->arg[2].i = n * m;
		inst->whole = mem->whole;
		inst->fraction = mem->fraction;
		inst->precision = mem->precision;
		/*--*/
		inst = newinst(prog);
		inst->opc = G_ANN_PROGRAM_INST_MULS;
		inst->arg[0].i = mem->b_[l];
		inst->arg[1].i = n * 1;
		inst->arg[2].r = -((double)ir->eta / (double)ir->batch);
		inst->whole = mem->whole;
		inst->fraction = mem->fraction;
		inst->precision = mem->precision;
		/*--*/
		inst = newinst(prog);
		inst->opc = G_ANN_PROGRAM_INST_ADD;
		inst->arg[0].i = mem->b[l];
		inst->arg[1].i = mem->b_[l];
		inst->arg[2].i = n * 1;
		inst->whole = mem->whole;
		inst->fraction = mem->fraction;
		inst->precision = mem->precision;
        }
	return 0;
}

static int emit_program_train(struct g_ann *ann,
			      const struct g_ir *ir,
			      int mem_,
			      int prog_) {
	struct g_ann_memory *mem;
	struct g_ann_program *prog;
	struct g_ann_program_inst *inst;
	uint64_t n, m;
	int l;

	/* setup */

	mem = &ann->memory[mem_];
	prog = &ann->program[prog_];

	/* return */

	inst = newinst(prog);
	inst->opc = G_ANN_PROGRAM_INST_RET;
	inst->whole = mem->whole;
	inst->fraction = mem->fraction;
	inst->precision = mem->precision;

	/*
         * adjustments:
         *   w[*] := 0
         *   b[*] := 0
         */

	for (l=1; l<ann->layers; ++l) {
                n = ir->nodes[l].size;
                m = ir->nodes[l - 1].size;
		/*--*/
		inst = newinst(prog);
		inst->opc = G_ANN_PROGRAM_INST_CLEAR;
		inst->arg[0].i = mem->w_[l];
		inst->arg[1].i = mem->unit * n * m;
		/*--*/
		inst = newinst(prog);
		inst->opc = G_ANN_PROGRAM_INST_CLEAR;
		inst->arg[0].i = mem->b_[l];
		inst->arg[1].i = mem->unit * n * 1;
        }

	/* batch training loop (x -> y) */

	l = ann->layers;
	n = ir->nodes[0].size;
	m = ir->nodes[l - 1].size;
	/*--*/
	inst = newinst(prog);
	inst->opc = G_ANN_PROGRAM_INST_BATCHLOOP;
	inst->arg[0].i = ir->batch;
	inst->arg[1].i = n;
	inst->arg[2].i = m;
	return 0;
}

static int emit_program(struct g_ann *ann, const struct g_ir *ir) {
	if (emit_program_activate(ann,
				  ir,
				  G_ANN_MEMORY_ACTIVATE,
				  G_ANN_PROGRAM_ACTIVATEX) ||
	    emit_program_activate(ann,
				  ir,
				  G_ANN_MEMORY_TRAIN,
				  G_ANN_PROGRAM_ACTIVATE) ||
	    emit_program_backprop(ann,
				  ir,
				  G_ANN_MEMORY_TRAIN,
				  G_ANN_PROGRAM_BACKPROP) ||
	    emit_program_train(ann,
			       ir,
			       G_ANN_MEMORY_TRAIN,
			       G_ANN_PROGRAM_TRAIN)) {
		G_DEBUG(0);
		return -1;
	}
	return 0;
}

struct g_ann *g_ann_open(const struct g_ir *ir) {
	struct g_ann *ann;
	size_t n;
	int i;

	assert( ir );

	/* initialize */

	ann = g_malloc(sizeof (struct g_ann));
	if (!ann) {
		G_DEBUG(0);
		return 0;
	}
	memset(ann, 0, sizeof (struct g_ann));
	ann->layers = ir->layers;
	ann->module = g_strdup(ir->module);
	ann->prefix = g_strdup(ir->prefix);
	if (!ann->module || !ann->prefix) {
		g_ann_close(ann);
		G_DEBUG(0);
		return 0;
	}

	/* memories */

	for (i=0; i<G_ANN_MEMORY_END; ++i) {
		n = ir->layers * sizeof (uint64_t);
		ann->memory[i].w = g_malloc(n);
		ann->memory[i].b = g_malloc(n);
		ann->memory[i].a_ = g_malloc(n);
		ann->memory[i].d_ = g_malloc(n);
		ann->memory[i].w_ = g_malloc(n);
		ann->memory[i].b_ = g_malloc(n);
		if (!ann->memory[i].w ||
		    !ann->memory[i].b ||
		    !ann->memory[i].a_ ||
		    !ann->memory[i].d_ ||
		    !ann->memory[i].w_ ||
		    !ann->memory[i].b_) {
			g_ann_close(ann);
			G_DEBUG(0);
			return 0;
		}
		memset(ann->memory[i].w, 0, n);
		memset(ann->memory[i].b, 0, n);
		memset(ann->memory[i].a_, 0, n);
		memset(ann->memory[i].d_, 0, n);
		memset(ann->memory[i].w_, 0, n);
		memset(ann->memory[i].b_, 0, n);
	}

	/* programs */

	for (i=0; i<G_ANN_PROGRAM_END; ++i) {
		n = MAX_PROGRAM_CAPACITY * sizeof (struct g_ann_program_inst);
		ann->program[i].inst = g_malloc(n);
		if (!ann->program[i].inst) {
			g_ann_close(ann);
			G_DEBUG(0);
			return 0;
		}
		memset(ann->program[i].inst, 0, n);
	}
	if (emit_memory(ann, ir) || emit_program(ann, ir)) {
		g_ann_close(ann);
		G_DEBUG(0);
		return 0;
	}
	return ann;
}

void g_ann_close(struct g_ann *ann) {
	int i;

	if (ann) {
		for (i=0; i<G_ANN_MEMORY_END; ++i) {
			G_FREE(ann->memory[i].w);
			G_FREE(ann->memory[i].b);
			G_FREE(ann->memory[i].a_);
			G_FREE(ann->memory[i].d_);
			G_FREE(ann->memory[i].w_);
			G_FREE(ann->memory[i].b_);
		}
		for (i=0; i<G_ANN_PROGRAM_END; ++i) {
			G_FREE(ann->program[i].inst);
		}
		G_FREE(ann->module);
		G_FREE(ann->prefix);
		memset(ann, 0, sizeof (struct g_ann));
	}
	G_FREE(ann);
}
