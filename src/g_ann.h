/**
 * g_ann.h
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

#ifndef _G_ANN_H_
#define _G_ANN_H_

#include "g_ir.h"

#define G__ANN_PRECISION_NONE   G__IR_PRECISION_NONE
#define G__ANN_PRECISION_FLOAT  G__IR_PRECISION_FLOAT
#define G__ANN_PRECISION_DOUBLE G__IR_PRECISION_DOUBLE
#define G__ANN_PRECISION_FIXED  G__IR_PRECISION_FIXED

#define G__ANN_MEMORY_ACTIVATE G__IR_MEMORY_ACTIVATE
#define G__ANN_MEMORY_TRAIN    G__IR_MEMORY_TRAIN
#define G__ANN_MEMORY_END      G__IR_MEMORY_END

#define G__ANN_PROGRAM_ACTIVATEX 0
#define G__ANN_PROGRAM_ACTIVATE  1
#define G__ANN_PROGRAM_BACKPROP  2
#define G__ANN_PROGRAM_TRAIN     3
#define G__ANN_PROGRAM_END       4

#define G__ANN_PROGRAM_INST_RET         1
#define G__ANN_PROGRAM_INST_RETARG      2
#define G__ANN_PROGRAM_INST_BATCHLOOP   3
#define G__ANN_PROGRAM_INST_CLEAR      11
#define G__ANN_PROGRAM_INST_COPYX      12
#define G__ANN_PROGRAM_INST_MUL1       13
#define G__ANN_PROGRAM_INST_MUL2       14
#define G__ANN_PROGRAM_INST_MUL3       15
#define G__ANN_PROGRAM_INST_MULS       16
#define G__ANN_PROGRAM_INST_ADD        17
#define G__ANN_PROGRAM_INST_SUBY       18
#define G__ANN_PROGRAM_INST_RELU      101
#define G__ANN_PROGRAM_INST_LINEAR    102
#define G__ANN_PROGRAM_INST_SOFTMAX   103
#define G__ANN_PROGRAM_INST_SIGMOID   104
#define G__ANN_PROGRAM_INST_RELUD    1001
#define G__ANN_PROGRAM_INST_LINEARD  1002
#define G__ANN_PROGRAM_INST_SOFTMAXD 1003
#define G__ANN_PROGRAM_INST_SIGMOIDD 1004

struct g__ann {
	int layers;
	const char *module;
	const char *prefix;
	struct g__ann_memory {
		int whole;
		int fraction;
		int precision;
		uint64_t unit;
		uint64_t size;
		uint64_t *w;
		uint64_t *b;
		uint64_t *a_;
		uint64_t *d_;
		uint64_t *w_;
		uint64_t *b_;
	} memory[G__ANN_MEMORY_END];
	struct g__ann_program {
		int size;
		struct g__ann_program_inst {
			int opc;
			int whole;
			int fraction;
			int precision;
			union {
				uint64_t i;
				double r;
			} arg[5];
		} *inst;
	} program[G__ANN_PROGRAM_END];
};

struct g__ann *g__ann_open(const struct g__ir *ir);

void g__ann_close(struct g__ann *an);

#endif /* _G__ANN_H_ */
